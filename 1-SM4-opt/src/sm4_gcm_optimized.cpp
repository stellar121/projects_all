#include "sm4_gcm_optimized.h"

// 不可约多项式：x^128 + x^7 + x^2 + x + 1（表示为0x87）
static const __m128i GF_POLY = _mm_set_epi64x(0x0000000000000087ULL, 0x0000000000000000ULL);

// 利用PCLMULQDQ指令实现GF(2^128)乘法（比软件实现快10倍以上）
__m128i sm4_gcm_opt::gf_mul_pclmul(const __m128i& a, const __m128i& b) {
    __m128i tmp1, tmp2, res;

    // 分块乘法：(a_high << 64) * (b_high << 64) 等
    tmp1 = _mm_clmulepi64_si128(a, b, 0x00);  // a_low * b_low
    tmp2 = _mm_clmulepi64_si128(a, b, 0x11);  // a_high * b_high

    // 中间项：(a_low + a_high) * (b_low + b_high)
    __m128i a_lo_hi = _mm_xor_si128(_mm_unpacklo_epi64(a, a), _mm_unpackhi_epi64(a, a));
    __m128i b_lo_hi = _mm_xor_si128(_mm_unpacklo_epi64(b, b), _mm_unpackhi_epi64(b, b));
    __m128i tmp3 = _mm_clmulepi64_si128(a_lo_hi, b_lo_hi, 0x00);
    tmp3 = _mm_xor_si128(tmp3, tmp1);
    tmp3 = _mm_xor_si128(tmp3, tmp2);

    // 合并结果（128位）
    res = _mm_xor_si128(tmp1, _mm_slli_si128(tmp3, 8));
    __m128i tmp4 = _mm_xor_si128(tmp2, _mm_srli_si128(tmp3, 8));
    res = _mm_xor_si128(res, tmp4);

    // 多项式约简（处理高64位溢出）
    for (int i = 0; i < 2; ++i) {
        __m128i mask = _mm_and_si128(_mm_srli_epi64(res, 63), GF_POLY);
        res = _mm_xor_si128(res, _mm_slli_epi64(mask, 1));
        res = _mm_xor_si128(res, _mm_slli_si128(mask, 8));
    }

    return res;
}

// 初始化上下文（使用GFNI优化的SM4生成认证密钥H）
void sm4_gcm_opt::init(Context& ctx, const uint8_t key[16], const uint8_t iv[], size_t iv_len) {
    // 生成SM4轮密钥
    sm4::key_expansion(key, ctx.rk);

    // 生成认证密钥H = SM4(0^128)，使用GFNI加速加密
    uint8_t zero_block[BLOCK_SIZE] = { 0 };
    uint8_t h_block[BLOCK_SIZE];
    sm4::encrypt_gfni(zero_block, h_block, ctx.rk);  // GFNI优化的加密
    ctx.auth_key = _mm_loadu_si128((__m128i*)h_block);

    // 初始化计数器（IV + 4字节0，补足128位）
    uint8_t nonce[BLOCK_SIZE] = { 0 };
    if (iv_len <= BLOCK_SIZE - 4) {
        memcpy(nonce, iv, iv_len);
        // 最后4字节设为0（GCM标准计数器初始值）
        nonce[BLOCK_SIZE - 1] = 0x00;
        nonce[BLOCK_SIZE - 2] = 0x00;
        nonce[BLOCK_SIZE - 3] = 0x00;
        nonce[BLOCK_SIZE - 4] = 0x01;  // 标准初始计数器从1开始
    }
    else {
        // IV过长时，用H加密IV（GCM扩展处理）
        __m128i iv_vec = _mm_loadu_si128((__m128i*)iv);
        __m128i iv_enc = gf_mul_pclmul(iv_vec, ctx.auth_key);
        _mm_storeu_si128((__m128i*)nonce, iv_enc);
    }
    ctx.nonce_counter = _mm_loadu_si128((__m128i*)nonce);

    // 初始化标签和长度计数
    ctx.tag = _mm_setzero_si128();
    ctx.ad_len = 0;
    ctx.data_len = 0;
}

// 附加数据更新（向量化处理，每次处理128位）
void sm4_gcm_opt::auth_update(Context& ctx, const uint8_t* adata, size_t len) {
    if (len == 0) return;
    ctx.ad_len += len;

    size_t pos = 0;
    __m128i h = ctx.auth_key;
    __m128i tag = ctx.tag;

    // 处理完整128位块
    while (pos + BLOCK_SIZE <= len) {
        __m128i ad_block = _mm_loadu_si128((__m128i*)(adata + pos));
        tag = gf_add(tag, gf_mul_pclmul(ad_block, h));  // PCLMUL加速乘法
        pos += BLOCK_SIZE;
    }

    // 处理剩余数据（不足128位）
    if (pos < len) {
        uint8_t last_block[BLOCK_SIZE] = { 0 };
        memcpy(last_block, adata + pos, len - pos);
        __m128i ad_block = _mm_loadu_si128((__m128i*)last_block);
        tag = gf_add(tag, gf_mul_pclmul(ad_block, h));
    }

    ctx.tag = tag;
}

// 加密更新（使用AESNI/GFNI加速SM4-CTR加密）
void sm4_gcm_opt::encrypt_update(Context& ctx, const uint8_t* plaintext, uint8_t* ciphertext, size_t len) {
    if (len == 0) return;
    ctx.data_len += len;

    size_t pos = 0;
    __m128i counter = ctx.nonce_counter;
    __m128i h = ctx.auth_key;
    __m128i tag = ctx.tag;
    uint32_t* rk = ctx.rk;

    // 批量处理（每次128位）
    while (pos < len) {
        // 1. 生成流密钥：SM4(counter)，使用GFNI加速
        uint8_t stream[BLOCK_SIZE];
        sm4::encrypt_gfni((uint8_t*)&counter, stream, rk);  // GFNI优化的加密
        __m128i stream_key = _mm_loadu_si128((__m128i*)stream);

        // 2. 加密当前块（异或流密钥）
        size_t block_len = (len - pos < BLOCK_SIZE) ? (len - pos) : BLOCK_SIZE;
        __m128i plain = _mm_loadu_si128((__m128i*)(plaintext + pos));
        __m128i cipher = _mm_xor_si128(plain, stream_key);
        _mm_storeu_si128((__m128i*)(ciphertext + pos), cipher);

        // 3. 更新认证标签（密文参与伽罗瓦乘法）
        if (block_len == BLOCK_SIZE) {
            tag = gf_add(tag, gf_mul_pclmul(cipher, h));
        }
        else {
            // 不足128位的块补0后参与计算
            uint8_t padded[BLOCK_SIZE] = { 0 };
            memcpy(padded, ciphertext + pos, block_len);
            __m128i padded_cipher = _mm_loadu_si128((__m128i*)padded);
            tag = gf_add(tag, gf_mul_pclmul(padded_cipher, h));
        }

        // 4. 计数器递增（大端序128位递增）
        uint64_t* cnt = (uint64_t*)&counter;
        cnt[0] = __builtin_bswap64(cnt[0]);  // 转为小端序递增
        cnt[0]++;
        if (cnt[0] == 0) {
            cnt[1] = __builtin_bswap64(cnt[1]);
            cnt[1]++;
            cnt[1] = __builtin_bswap64(cnt[1]);
        }
        cnt[0] = __builtin_bswap64(cnt[0]);  // 转回大端序

        pos += block_len;
    }

    ctx.nonce_counter = counter;
    ctx.tag = tag;
}

// 完成加密并生成标签（加入长度信息）
void sm4_gcm_opt::encrypt_final(Context& ctx, uint8_t tag[TAG_SIZE]) {
    // 1. 处理长度信息：附加数据长度 || 明文长度（各64位，大端序）
    uint8_t len_block[BLOCK_SIZE];
    uint64_t ad_len = __builtin_bswap64(ctx.ad_len * 8);   // 位长度
    uint64_t data_len = __builtin_bswap64(ctx.data_len * 8);
    memcpy(len_block, &ad_len, 8);
    memcpy(len_block + 8, &data_len, 8);
    __m128i len_vec = _mm_loadu_si128((__m128i*)len_block);

    // 2. 标签 = (当前标签 + len_vec * H) XOR SM4(初始计数器)
    __m128i tag_with_len = gf_add(ctx.tag, gf_mul_pclmul(len_vec, ctx.auth_key));
    uint8_t counter_enc[BLOCK_SIZE];
    sm4::encrypt_gfni((uint8_t*)&ctx.nonce_counter, counter_enc, ctx.rk);  // GFNI加速
    __m128i counter_vec = _mm_loadu_si128((__m128i*)counter_enc);
    __m128i final_tag = gf_add(tag_with_len, counter_vec);

    _mm_storeu_si128((__m128i*)tag, final_tag);
}

// 解密更新（复用加密逻辑，保证一致性）
void sm4_gcm_opt::decrypt_update(Context& ctx, const uint8_t* ciphertext, uint8_t* plaintext, size_t len) {
    if (len == 0) return;
    ctx.data_len += len;

    size_t pos = 0;
    __m128i counter = ctx.nonce_counter;
    __m128i h = ctx.auth_key;
    __m128i tag = ctx.tag;
    uint32_t* rk = ctx.rk;

    while (pos < len) {
        // 生成流密钥（与加密相同）
        uint8_t stream[BLOCK_SIZE];
        sm4::encrypt_gfni((uint8_t*)&counter, stream, rk);
        __m128i stream_key = _mm_loadu_si128((__m128i*)stream);

        // 解密密文（异或流密钥）
        size_t block_len = (len - pos < BLOCK_SIZE) ? (len - pos) : BLOCK_SIZE;
        __m128i cipher = _mm_loadu_si128((__m128i*)(ciphertext + pos));
        __m128i plain = _mm_xor_si128(cipher, stream_key);
        _mm_storeu_si128((__m128i*)(plaintext + pos), plain);

        // 更新标签（使用密文，与加密保持一致）
        if (block_len == BLOCK_SIZE) {
            tag = gf_add(tag, gf_mul_pclmul(cipher, h));
        }
        else {
            uint8_t padded[BLOCK_SIZE] = { 0 };
            memcpy(padded, ciphertext + pos, block_len);
            __m128i padded_cipher = _mm_loadu_si128((__m128i*)padded);
            tag = gf_add(tag, gf_mul_pclmul(padded_cipher, h));
        }

        // 计数器递增（同加密逻辑）
        uint64_t* cnt = (uint64_t*)&counter;
        cnt[0] = __builtin_bswap64(cnt[0]);
        cnt[0]++;
        if (cnt[0] == 0) {
            cnt[1] = __builtin_bswap64(cnt[1]);
            cnt[1]++;
            cnt[1] = __builtin_bswap64(cnt[1]);
        }
        cnt[0] = __builtin_bswap64(cnt[0]);

        pos += block_len;
    }

    ctx.nonce_counter = counter;
    ctx.tag = tag;
}

// 验证标签
bool sm4_gcm_opt::decrypt_final(Context& ctx, const uint8_t tag[TAG_SIZE]) {
    uint8_t calc_tag[TAG_SIZE];
    encrypt_final(ctx, calc_tag);  // 复用加密的最终标签计算逻辑
    return memcmp(calc_tag, tag, TAG_SIZE) == 0;
}

import cv2
import numpy as np
from scipy.fftpack import dct, idct

def embed_watermark(cover_path, watermark_path, output_path, alpha=10):
    # 读取原图和水印
    cover = cv2.imread(cover_path, cv2.IMREAD_GRAYSCALE)
    watermark = cv2.imread(watermark_path, cv2.IMREAD_GRAYSCALE)

    # 调整水印尺寸
    watermark = cv2.resize(watermark, (32, 32))
    watermark = (watermark > 128).astype(np.float32)  # 二值化

    # DCT
    dct_cover = dct(dct(cover.astype(float), axis=0, norm='ortho'), axis=1, norm='ortho')
    dct_cover[:32, :32] += alpha * watermark

    # 逆 DCT
    watermarked = idct(idct(dct_cover, axis=1, norm='ortho'), axis=0, norm='ortho')
    watermarked = np.clip(watermarked, 0, 255).astype(np.uint8)

    cv2.imwrite(output_path, watermarked)
    print(f"水印嵌入完成：{output_path}")
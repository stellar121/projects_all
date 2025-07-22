import cv2
from embed_watermark import embed_watermark
from extract_watermark import extract_watermark
from attacks import apply_flip, apply_translation, apply_crop, apply_contrast

# 文件路径
original_image = r"D:\pythonProject1\project2\lenna.png"
watermark_image = r"D:\pythonProject1\project2\watermark.png"
watermarked_output = "wm_lenna.png"

# Step 1: 嵌入水印
embed_watermark(original_image, watermark_image, watermarked_output)

# Step 2: 加载嵌入后的图像
wm_img = cv2.imread(watermarked_output, cv2.IMREAD_GRAYSCALE)

# Step 3: 模拟攻击
attacks = {
    "flip": apply_flip(wm_img),
    "translate": apply_translation(wm_img),
    "crop": apply_crop(wm_img),
    "contrast": apply_contrast(wm_img)
}

# Step 4: 对每种攻击提取水印并保存
for name, attacked_img in attacks.items():
    attacked_path = f"{name}.png"
    cv2.imwrite(attacked_path, attacked_img)

    extracted = extract_watermark(attacked_path, original_image)
    cv2.imwrite(f"extracted_{name}.png", extracted)
    print(f"{name} 攻击后的水印已提取并保存为 extracted_{name}.png")
# 数字图像水印系统（含鲁棒性攻击测试）

本项目基于 Python 实现了一个简洁的灰度图像水印嵌入与提取系统，使用 DCT（离散余弦变换）作为核心技术，并提供了多种图像攻击模拟与鲁棒性验证功能。


## 项目功能亮点

- 支持灰度图像水印的不可见嵌入
- 基于原图提取嵌入水印
- 提供常见图像攻击模拟：
  - 翻转（Flip）
  - 平移（Translation）
  - 裁剪（Crop）
  - 对比度调整（Contrast）
- 提供嵌入、攻击与提取后的可视化结果示例

## 项目目录结构

.

├── src/

│ ├── embed_watermark.py # 水印嵌入函数

│ ├── extract_watermark.py # 水印提取函数

│ └── attacks.py # 攻击模拟函数

├── images/

│ ├── lenna.png # 原始载体图像

│ ├── watermark.png # 二值水印图像

│ ├── wm_lenna.png # 嵌入水印后的图像

│ ├── flip.png # 各类攻击后的图像

│ ├── translate.png

│ ├── crop.png

│ ├── contrast.png

│ ├── extracted_flip.png # 提取出的水印结果

│ ├── extracted_translate.png

│ ├── extracted_crop.png

│ └── extracted_contrast.png

├── test/

│ └── test.py

└── README.md # 项目说明文件（本文件）




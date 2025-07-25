# 数字图像水印系统（含鲁棒性攻击测试）

## 项目目录结构

```text
.
├── src/
│   ├── embed_watermark.py         # 水印嵌入函数
│   ├── extract_watermark.py       # 水印提取函数
│   ├── attacks.py                 # 攻击模拟函数
├── images/
│   ├── lenna.png                  # 原始载体图像
│   ├── watermark.png              # 二值水印图像
│   ├── wm_lenna.png               # 嵌入水印后的图像
│   ├── flip.png                   # 攻击图像：翻转
│   ├── translate.png              # 攻击图像：平移
│   ├── crop.png                   # 攻击图像：裁剪
│   ├── contrast.png               # 攻击图像：对比度
│   ├── extracted_flip.png         # 提取结果：翻转
│   ├── extracted_translate.png    # 提取结果：平移
│   ├── extracted_crop.png         # 提取结果：裁剪
│   ├── extracted_contrast.png     # 提取结果：对比度
├── test/
│   └── test.py                    # 主测试脚本
├── README.md                      # 项目说明文件（本文件）
└── requirements.txt               # 依赖列表

## 具体的算法原理、代码分析、实验结果与分析见报告文档————project2.pdf

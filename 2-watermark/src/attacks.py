import cv2
import numpy as np

def apply_flip(image):
    return cv2.flip(image, 1)

def apply_translation(image, dx=10, dy=10):
    M = np.float32([[1, 0, dx], [0, 1, dy]])
    return cv2.warpAffine(image, M, (image.shape[1], image.shape[0]))

def apply_crop(image, crop_size=30):
    h, w = image.shape[:2]
    cropped = image[crop_size:h-crop_size, crop_size:w-crop_size]
    return cv2.resize(cropped, (w, h))  # 还原尺寸以便提取

def apply_contrast(image, alpha=1.5, beta=0):
    return cv2.convertScaleAbs(image, alpha=alpha, beta=beta)
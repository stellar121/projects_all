import cv2
import numpy as np
from scipy.fftpack import dct

def extract_watermark(watermarked_path, original_path, alpha=10):
    w_img = cv2.imread(watermarked_path, cv2.IMREAD_GRAYSCALE)
    o_img = cv2.imread(original_path, cv2.IMREAD_GRAYSCALE)

    dct_w = dct(dct(w_img.astype(float), axis=0, norm='ortho'), axis=1, norm='ortho')
    dct_o = dct(dct(o_img.astype(float), axis=0, norm='ortho'), axis=1, norm='ortho')

    wm = (dct_w[:32, :32] - dct_o[:32, :32]) / alpha
    wm = (wm > 0.5).astype(np.uint8) * 255
    return wm
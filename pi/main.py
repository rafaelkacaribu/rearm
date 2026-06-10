from picamera2 import Picamera2
import numpy as np
import math
import time
import cv2

def capture_area_image() -> np.ndarray:
    picam2 = Picamera2()

    config = picam2.create_still_configuration()
    picam2.configure(config)

    picam2.start()
    time.sleep(2)

    image = picam2.capture_array()

    picam2.stop()

    return image


def rotate(image: np.ndarray, angle_deg: float) -> np.ndarray:
    """
    Rotate a numpy image by angle_deg and crop away the black/empty borders.
    Positive angle = counter-clockwise.
    """

    h, w = image.shape[:2]
    center = (w / 2, h / 2)

    # Rotation matrix
    M = cv2.getRotationMatrix2D(center, angle_deg, 1.0)

    # New canvas size after rotation
    cos = abs(M[0, 0])
    sin = abs(M[0, 1])

    new_w = int((h * sin) + (w * cos))
    new_h = int((h * cos) + (w * sin))

    # Adjust translation
    M[0, 2] += (new_w / 2) - center[0]
    M[1, 2] += (new_h / 2) - center[1]

    rotated = cv2.warpAffine(
        image,
        M,
        (new_w, new_h),
        flags=cv2.INTER_LINEAR,
        borderMode=cv2.BORDER_CONSTANT,
        borderValue=(0, 0, 0)
    )

    # Crop largest rectangle without black borders
    angle = math.radians(abs(angle_deg))
    sin_a = abs(math.sin(angle))
    cos_a = abs(math.cos(angle))

    if h <= 0 or w <= 0:
        return rotated

    crop_w = int(w * cos_a - h * sin_a)
    crop_h = int(h * cos_a - w * sin_a)

    # Fallback for larger rotations
    if crop_w <= 0 or crop_h <= 0:
        return rotated

    cx, cy = new_w // 2, new_h // 2

    x1 = max(cx - crop_w // 2, 0)
    y1 = max(cy - crop_h // 2, 0)
    x2 = min(cx + crop_w // 2, new_w)
    y2 = min(cy + crop_h // 2, new_h)

    return rotated[y1:y2, x1:x2]

if __name__ == "__main__":
    img = capture_area_image()
    img = rotate(img, 1)[50:img.shape[0]-150, :]
    img = cv2.cvtColor(img, cv2.COLOR_RGB2BGR)
    cv2.imwrite("area.jpg", img)

    


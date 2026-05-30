# YOLOv8 object detection with oriented bounding box

import cv2
import numpy as np
from dataclasses import dataclass
from typing import Optional

_model = None
_model_path = "yolov8n.pt"  # standard COCO model, downloads automatically


@dataclass
class DetectionResult:
    x: float          # center X, pixels, relative to the original image
    y: float          # center Y, pixels, relative to the original image
    W: float          # box width in pixels
    L: float          # box height in pixels
    deg: float        # rotation angle [0, 180)
    confidence: float # how sure the model is [0.0 - 1.0]
    label: str        # what yolo thinks the object is

    def __repr__(self):
        return (
            f"DetectionResult(x={self.x:.1f}, y={self.y:.1f}, "
            f"W={self.W:.1f}, L={self.L:.1f}, deg={self.deg:.1f}°, "
            f"conf={self.confidence:.2f}, label='{self.label}')"
        )


def _load_model(model_path: str):
    # keep the model in memory so we don't reload it every frame
    global _model
    if _model is None:
        from ultralytics import YOLO
        _model = YOLO(model_path)
    return _model


def _rotated_box_from_mask(mask: np.ndarray) -> Optional[tuple]:
    # find contours in the binary mask and fit a rotated rectangle
    contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    if not contours:
        return None
    largest = max(contours, key=cv2.contourArea)
    if cv2.contourArea(largest) < 100:
        return None
    return cv2.minAreaRect(largest)  # ((cx, cy), (w, h), angle)


def _fallback_from_grayscale(roi: np.ndarray, threshold: int = 200) -> Optional[tuple]:
    # yolo found nothing — try plain grayscale threshold as last resort
    gray = cv2.cvtColor(roi, cv2.COLOR_BGR2GRAY) if len(roi.shape) == 3 else roi.copy()
    blurred = cv2.GaussianBlur(gray, (5, 5), 0)
    _, binary = cv2.threshold(blurred, threshold, 255, cv2.THRESH_BINARY_INV)
    kernel = cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (7, 7))
    cleaned = cv2.morphologyEx(binary, cv2.MORPH_CLOSE, kernel, iterations=2)
    cleaned = cv2.morphologyEx(cleaned, cv2.MORPH_OPEN, kernel, iterations=1)
    return _rotated_box_from_mask(cleaned)


def detect_object(
    image: np.ndarray,
    bounds: Optional[tuple[int, int, int, int]] = None,
    confidence_threshold: float = 0.1,
    model_path: str = _model_path,
) -> Optional[DetectionResult]:
    """
    Takes a numpy image and returns the bounding box of the detected object.

    bounds is (x, y, w, h) if you only want to look at part of the image.
    returned coordinates are always relative to the full original image.

    if yolo doesn't find anything it falls back to grayscale thresholding,
    which works well on a plain white background.
    """
    model = _load_model(model_path)

    h_img, w_img = image.shape[:2]

    # crop to the region we care about if bounds were given
    if bounds is not None:
        bx, by, bw, bh = bounds
        bx = max(0, min(bx, w_img))
        by = max(0, min(by, h_img))
        bw = max(1, min(bw, w_img - bx))
        bh = max(1, min(bh, h_img - by))
        roi = image[by:by + bh, bx:bx + bw]
        offset_x, offset_y = bx, by
    else:
        roi = image
        offset_x, offset_y = 0, 0

    results = model.predict(roi, verbose=False, conf=confidence_threshold)

    rect = None
    label = "unknown"
    confidence = 0.0

    if results and len(results[0].boxes) > 0:
        boxes = results[0].boxes

        # pick highest confidence detection
        confs = boxes.conf.cpu().numpy()
        best_idx = int(np.argmax(confs))
        confidence = float(confs[best_idx])

        class_id = int(boxes.cls.cpu().numpy()[best_idx])
        label = model.names.get(class_id, str(class_id))

        # get the bounding box as a binary mask, then fit a rotated rect on it
        # this gives us the angle even though standard yolo doesn't output it
        x1, y1, x2, y2 = boxes.xyxy.cpu().numpy()[best_idx].astype(int)
        x1, y1 = max(0, x1), max(0, y1)
        x2, y2 = min(roi.shape[1], x2), min(roi.shape[0], y2)

        # run grayscale threshold inside the yolo crop to get a tight rotated box
        crop = roi[y1:y2, x1:x2]
        inner_rect = _fallback_from_grayscale(crop)

        if inner_rect is not None:
            (cx_crop, cy_crop), (w, h), angle = inner_rect
            # shift from crop-local to roi-local coords
            rect = ((cx_crop + x1, cy_crop + y1), (w, h), angle)
        else:
            # just use the axis-aligned yolo box, angle = 0
            cx_box = (x1 + x2) / 2
            cy_box = (y1 + y2) / 2
            rect = ((cx_box, cy_box), (x2 - x1, y2 - y1), 0.0)

    # yolo found nothing, try grayscale fallback on the whole roi
    if rect is None:
        rect = _fallback_from_grayscale(roi)
        if rect is None:
            return None

    (cx_roi, cy_roi), (w, h), angle = rect

    angle_deg = float(angle) % 180
    if w < h:
        w, h = h, w
        angle_deg = (angle_deg + 90) % 180

    return DetectionResult(
        x=float(cx_roi) + offset_x,
        y=float(cy_roi) + offset_y,
        W=float(w),
        L=float(h),
        deg=angle_deg,
        confidence=confidence,
        label=label,
    )


def is_static(
    results: list[Optional[DetectionResult]],
    tolerance_px: float = 10.0,
    tolerance_deg: float = 2.0,
) -> bool:
    """
    Checks if the object stayed in the same spot across multiple frames.
    Pass in a list of DetectionResults from consecutive captures.
    Returns False if any frame had no detection.
    """
    if len(results) < 2:
        return False

    for a, b in zip(results, results[1:]):
        if a is None or b is None:
            return False
        if abs(a.x - b.x) > tolerance_px:
            return False
        if abs(a.y - b.y) > tolerance_px:
            return False
        # angles wrap around at 180 so we handle that case
        angle_diff = abs(a.deg - b.deg)
        angle_diff = min(angle_diff, 180 - angle_diff)
        if angle_diff > tolerance_deg:
            return False

    return True


if __name__ == "__main__":
    import sys

    path = sys.argv[1] if len(sys.argv) > 1 else "test.jpg"
    img = cv2.imread(path)

    if img is None:
        print(f"couldn't load {path}")
        sys.exit(1)

    result = detect_object(img)
    print(result if result else "nothing detected")

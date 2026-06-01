# YOLOv8 object detection with oriented bounding box

import cv2
import numpy as np
from dataclasses import dataclass
from typing import Optional

_model = None
_model_path = "yolov8n.pt"  # standard COCO model, downloads automatically


@dataclass
class DetectionResult:
    x: float           # center X, pixels, relative to the original image
    y: float           # center Y, pixels, relative to the original image
    W: float           # box width in pixels
    L: float           # box height in pixels
    deg: float         # rotation angle [0, 180)
    confidence: float  # how sure the model is [0.0 - 1.0]
    label: str         # what yolo thinks the object is
    mask: np.ndarray   # binary mask of the object in original image coordinates (0 or 255)

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


def _make_mask(roi: np.ndarray, bg_threshold: int = 200) -> np.ndarray:
    # filter out white background pixels — what's left is the object
    # works by thresholding in grayscale: bright = background, dark = object
    gray = cv2.cvtColor(roi, cv2.COLOR_BGR2GRAY) if len(roi.shape) == 3 else roi.copy()
    blurred = cv2.GaussianBlur(gray, (5, 5), 0)
    _, mask = cv2.threshold(blurred, bg_threshold, 255, cv2.THRESH_BINARY_INV)

    # clean up noise and fill small holes
    kernel = cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (7, 7))
    mask = cv2.morphologyEx(mask, cv2.MORPH_CLOSE, kernel, iterations=2)
    mask = cv2.morphologyEx(mask, cv2.MORPH_OPEN, kernel, iterations=1)
    return mask


def detect_object(
    image: np.ndarray,
    bounds: Optional[tuple[int, int, int, int]] = None,
    confidence_threshold: float = 0.1,
    bg_threshold: int = 200,
    model_path: str = _model_path,
) -> Optional[DetectionResult]:
    """
    Takes a numpy image and returns the bounding box + object mask.

    bounds is (x, y, w, h) if you only want to look at part of the image.
    returned coordinates and mask are always in the original image's space.

    bg_threshold controls background filtering — lower means stricter
    (only very dark pixels count as object). default 200 works well
    for a white sheet background.
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
    mask_roi = None  # will hold the binary mask in roi coordinates

    if results and len(results[0].boxes) > 0:
        boxes = results[0].boxes

        # pick highest confidence detection
        confs = boxes.conf.cpu().numpy()
        best_idx = int(np.argmax(confs))
        confidence = float(confs[best_idx])

        class_id = int(boxes.cls.cpu().numpy()[best_idx])
        label = model.names.get(class_id, str(class_id))

        x1, y1, x2, y2 = boxes.xyxy.cpu().numpy()[best_idx].astype(int)
        x1, y1 = max(0, x1), max(0, y1)
        x2, y2 = min(roi.shape[1], x2), min(roi.shape[0], y2)

        # mask the object inside the yolo crop, then place it back in roi space
        crop = roi[y1:y2, x1:x2]
        crop_mask = _make_mask(crop, bg_threshold)

        # place the crop mask back into a full-roi-sized mask
        mask_roi = np.zeros(roi.shape[:2], dtype=np.uint8)
        mask_roi[y1:y2, x1:x2] = crop_mask

        rect = _rotated_box_from_mask(crop_mask)
        if rect is not None:
            (cx_crop, cy_crop), (w, h), angle = rect
            rect = ((cx_crop + x1, cy_crop + y1), (w, h), angle)
        else:
            cx_box = (x1 + x2) / 2
            cy_box = (y1 + y2) / 2
            rect = ((cx_box, cy_box), (x2 - x1, y2 - y1), 0.0)

    # yolo found nothing, fall back to masking the whole roi
    if rect is None:
        mask_roi = _make_mask(roi, bg_threshold)
        rect = _rotated_box_from_mask(mask_roi)
        if rect is None:
            return None

    (cx_roi, cy_roi), (w, h), angle = rect

    angle_deg = float(angle) % 180
    if w < h:
        w, h = h, w
        angle_deg = (angle_deg + 90) % 180

    # place the mask back in full original image coordinates
    full_mask = np.zeros((h_img, w_img), dtype=np.uint8)
    full_mask[offset_y:offset_y + roi.shape[0], offset_x:offset_x + roi.shape[1]] = mask_roi

    return DetectionResult(
        x=float(cx_roi) + offset_x,
        y=float(cy_roi) + offset_y,
        W=float(w),
        L=float(h),
        deg=angle_deg,
        confidence=confidence,
        label=label,
        mask=full_mask,
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

    if result:
        print(result)

        # show the mask overlaid on the original image
        overlay = img.copy()
        overlay[result.mask > 0] = (0, 200, 0)  # green = detected object pixels
        preview = cv2.addWeighted(img, 0.6, overlay, 0.4, 0)

        # draw the rotated bounding box on top
        box_points = cv2.boxPoints(((result.x, result.y), (result.W, result.L), result.deg))
        box_points = np.int32(box_points)
        cv2.drawContours(preview, [box_points], 0, (0, 0, 255), 2)

        cv2.imshow("detection", preview)
        cv2.waitKey(0)
        cv2.destroyAllWindows()
    else:
        print("nothing detected")

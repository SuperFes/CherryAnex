#!/usr/bin/env python3
import json
from pathlib import Path

KB_DIR = Path(__file__).resolve().parents[1]  # .../anex
JSON_PATH = KB_DIR / "keyboard.json"
# New output: JSON with a "layout" array containing matrix/x/y/flags entries
OUT_JSON = KB_DIR / "layout.generated.json"

NO_LED = -1

def main():
    with open(JSON_PATH, "r", encoding="utf-8") as f:
        data = json.load(f)

    layout = data["layouts"]["LAYOUT"]["layout"]

    # Collect items as (row, col, x, y)
    items = []
    for it in layout:
        r, c = it["matrix"]
        x = float(it["x"])  # ensure numeric
        y = float(it["y"])  # ensure numeric
        items.append((r, c, x, y))

    # Determine bounds for normalization to 224x64 space
    xs = [x for _, _, x, _ in items]
    ys = [y for _, _, _, y in items]
    minx, maxx = min(xs), max(xs)
    miny, maxy = min(ys), max(ys)
    sx = 224.0 / (maxx - minx) if maxx > minx else 1.0
    sy =  64.0 / (maxy - miny) if maxy > miny else 1.0

    # Matrix dimensions (rows, cols)
    max_r = max(r for r, c, x, y in items)
    max_c = max(c for r, c, x, y in items)
    rows = max_r + 1
    cols = max_c + 1

    # Build matrix with NO_LED
    matrix = [[NO_LED for _ in range(cols)] for _ in range(rows)]
    positions = []  # (x, y) per LED index
    for idx, (r, c, x, y) in enumerate(items):
        matrix[r][c] = idx
        xn = int(round((x - minx) * sx))
        yn = int(round((y - miny) * sy))
        # clamp to bounds just in case
        xn = max(0, min(224, xn))
        yn = max(0, min(64, yn))
        positions.append((xn, yn))

    flags = [1] * len(items)  # LED_FLAG_KEYLIGHT

    # Build JSON layout with flags per entry (default 4 as requested)
    layout_out = []
    for (r, c, _x, _y), (xn, yn) in zip(items, positions):
        layout_out.append({
            "matrix": [r, c],
            "x": xn,
            "y": yn,
            "flags": 4,
        })

    # Emit JSON file
    OUT_JSON.parent.mkdir(parents=True, exist_ok=True)
    with open(OUT_JSON, "w", encoding="utf-8") as out:
        json.dump({"layout": layout_out}, out, ensure_ascii=False, indent=4)
        out.write("\n")

    print(f"Generated: {OUT_JSON} (entries: {len(layout_out)}, Matrix: {rows}x{cols})")

if __name__ == "__main__":
    main()

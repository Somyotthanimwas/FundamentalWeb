#!/bin/bash

set -u

SOURCE="/mnt/c/Program Files/FundamentalUpdater_rev4/Data/Fundamental/fundamental_v4.csv"
TARGET="$HOME/FundamentalWeb/data/fundamental_v4.csv"
REPO="$HOME/FundamentalWeb"

cd "$REPO" || exit 1

if [ ! -f "$SOURCE" ]; then
    echo "ERROR: V4 CSV not found"
    exit 1
fi

# ตรวจจำนวนหุ้นก่อน
ROWS=$(wc -l < "$SOURCE")

if [ "$ROWS" -ne 870 ]; then
    echo "ERROR: Invalid V4 CSV row count: $ROWS"
    exit 1
fi

# คัดลอก CSV ตัวจริงจาก V4
cp "$SOURCE" "$TARGET"

# ถ้าไม่มีการเปลี่ยนแปลง ไม่ต้องทำอะไร
if git diff --quiet -- data/fundamental_v4.csv; then
    exit 0
fi

echo "===== V4 CSV CHANGED ====="
date
echo "Rows: $ROWS"
grep '^AOT,' "$TARGET"

git add data/fundamental_v4.csv

git commit -m "Auto sync V4 fundamental CSV"

git push origin main

echo "===== SYNC COMPLETE ====="

#!/bin/bash

set -u

SOURCE="/mnt/c/Program Files/FundamentalUpdater_rev5/Data/Fundamental/fundamental_v4.csv"
TARGET="$HOME/FundamentalWeb/data/fundamental_v4.csv"
REPO="$HOME/FundamentalWeb"

cd "$REPO" || exit 1

if [ ! -f "$SOURCE" ]; then
    echo "ERROR: V4 CSV not found"
    exit 1
fi

echo "===== WAIT FOR V4 CSV STABLE ====="

# รอให้ V4 เขียนไฟล์เสร็จจริง
# ตรวจ size + hash ต้องเหมือนกัน 2 ครั้งติดกัน
STABLE_COUNT=0
PREV_SIZE=""
PREV_HASH=""

for i in {1..15}; do

    if [ ! -f "$SOURCE" ]; then
        echo "ERROR: V4 CSV disappeared"
        exit 1
    fi

    SIZE=$(stat -c %s "$SOURCE")
    HASH=$(sha256sum "$SOURCE" | awk '{print $1}')

    if [ "$SIZE" = "$PREV_SIZE" ] && [ "$HASH" = "$PREV_HASH" ]; then
        STABLE_COUNT=$((STABLE_COUNT + 1))
    else
        STABLE_COUNT=0
    fi

    PREV_SIZE="$SIZE"
    PREV_HASH="$HASH"

    if [ "$STABLE_COUNT" -ge 2 ]; then
        break
    fi

    sleep 1
done

if [ "$STABLE_COUNT" -lt 2 ]; then
    echo "ERROR: V4 CSV did not become stable"
    exit 1
fi

echo "CSV stable:"
echo "Size: $SIZE"
echo "SHA256: $HASH"

# ตรวจจำนวนหุ้นหลังไฟล์นิ่งแล้ว
ROWS=$(wc -l < "$SOURCE")

if [ "$ROWS" -lt 2 ]; then
    echo "ERROR: Invalid V4 CSV row count: $ROWS"
    exit 1
fi

# ตรวจว่ามี AOT
if ! grep -q '^AOT,' "$SOURCE"; then
    echo "ERROR: AOT not found in V4 CSV"
    exit 1
fi

echo "Rows: $ROWS"
echo "SOURCE AOT:"
grep '^AOT,' "$SOURCE"

# คัดลอกไป temp ก่อน แล้วค่อย replace แบบ atomic
TMP="$TARGET.tmp"

cp "$SOURCE" "$TMP"

# ตรวจไฟล์ที่ copy แล้ว
TMP_ROWS=$(wc -l < "$TMP")

if [ "$TMP_ROWS" -lt 2 ]; then
    echo "ERROR: Copied CSV row count invalid: $TMP_ROWS"
    rm -f "$TMP"
    exit 1
fi

mv -f "$TMP" "$TARGET"

echo "LOCAL AOT:"
grep '^AOT,' "$TARGET"

# ตรวจ GitHub ล่าสุดก่อนตัดสินใจ commit/push
echo "===== CHECK GITHUB ====="
git fetch origin main

# เพิ่ม CSV เข้า staging
git add data/fundamental_v4.csv

# ถ้า CSV ไม่มีการเปลี่ยนจาก HEAD จริง ๆ
if git diff --cached --quiet -- data/fundamental_v4.csv; then
    echo "CSV unchanged from local HEAD"

    # แต่ถ้า local HEAD ยังไม่ได้ push ให้ push ต่อ
    if git merge-base --is-ancestor origin/main HEAD; then
        if [ "$(git rev-parse origin/main)" != "$(git rev-parse HEAD)" ]; then
            echo "Local HEAD is ahead of GitHub. Pushing..."
            git push origin main
        else
            echo "GitHub already up to date"
        fi
    else
        echo "ERROR: Local branch and GitHub have diverged"
        echo "Refusing automatic push"
        exit 1
    fi

    exit 0
fi

echo "===== V4 CSV CHANGED ====="
date

git commit -m "Auto sync V4 fundamental CSV"

# ตรวจอีกครั้งก่อน push
git fetch origin main

if git merge-base --is-ancestor origin/main HEAD; then
    git push origin main
else
    echo "ERROR: Local branch and GitHub have diverged"
    echo "Refusing automatic push"
    exit 1
fi

echo "===== SYNC COMPLETE ====="

#!/bin/bash

set -u

SOURCE="/mnt/c/Program Files/FundamentalUpdater_rev5/Data/Fundamental/fundamental_v4.csv"
TARGET="$HOME/FundamentalWeb/data/fundamental_v4.csv"
REPO="$HOME/FundamentalWeb"

cd "$REPO" || exit 1

if [ ! -f "$SOURCE" ]; then
    echo "ERROR: V5 V4 CSV not found"
    exit 1
fi

echo "===== WAIT FOR V4 CSV STABLE ====="

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

ROWS=$(wc -l < "$SOURCE")

if [ "$ROWS" -lt 2 ]; then
    echo "ERROR: Invalid V4 CSV row count: $ROWS"
    exit 1
fi

if ! grep -q '^AOT,' "$SOURCE"; then
    echo "ERROR: AOT not found in V4 CSV"
    exit 1
fi

echo "Rows: $ROWS"
echo "SOURCE AOT:"
grep '^AOT,' "$SOURCE"

TMP="$TARGET.tmp"
cp "$SOURCE" "$TMP"

TMP_ROWS=$(wc -l < "$TMP")
if [ "$TMP_ROWS" -lt 2 ]; then
    echo "ERROR: Copied CSV row count invalid: $TMP_ROWS"
    rm -f "$TMP"
    exit 1
fi

mv -f "$TMP" "$TARGET"

echo "LOCAL AOT:"
grep '^AOT,' "$TARGET"

if git diff --quiet -- data/fundamental_v4.csv; then
    echo "No Git change"
    exit 0
fi

echo "===== V4 CSV CHANGED ====="
date

git add data/fundamental_v4.csv
git commit -m "Auto sync V4 fundamental CSV"
git push origin main

echo "===== SYNC COMPLETE ====="

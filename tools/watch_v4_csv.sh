#!/bin/bash

SOURCE="/mnt/c/Program Files/FundamentalUpdater_rev4/Data/Fundamental/fundamental_v4.csv"
SYNC="$HOME/FundamentalWeb/tools/sync_v4_csv.sh"

echo "===== V4 CSV AUTO WATCH ====="
echo "Watching:"
echo "$SOURCE"
echo

if [ ! -f "$SOURCE" ]; then
    echo "ERROR: V4 CSV not found"
    exit 1
fi

echo "Initial sync..."
"$SYNC"

echo
echo "Waiting for V4 CSV changes..."

inotifywait -m \
    -e close_write \
    -e moved_to \
    -e create \
    "$(dirname "$SOURCE")" |
while read -r directory event filename; do

    if [ "$filename" = "fundamental_v4.csv" ]; then
        echo
        echo "===== V4 CSV CHANGE DETECTED ====="
        date

        sleep 1

        "$SYNC"
    fi

done

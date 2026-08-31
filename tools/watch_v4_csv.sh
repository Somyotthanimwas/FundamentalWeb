#!/bin/bash

SOURCE="/mnt/c/Program Files/FundamentalUpdater_rev4/Data/Fundamental/fundamental_v4.csv"
SYNC="$HOME/FundamentalWeb/tools/sync_v4_csv.sh"
WATCH_DIR="$(dirname "$SOURCE")"

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
echo "Setting up watch..."

while true; do

    EVENT=$(inotifywait -q \
        -e close_write \
        -e moved_to \
        -e create \
        "$WATCH_DIR" 2>/dev/null)

    FILE=$(echo "$EVENT" | awk '{print $3}')

    if [ "$FILE" = "fundamental_v4.csv" ]; then

        echo
        echo "===== V4 CSV CHANGE DETECTED ====="
        date

        sleep 2

        "$SYNC"

    fi

done

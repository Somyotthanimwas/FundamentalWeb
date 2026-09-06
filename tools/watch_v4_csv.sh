#!/bin/bash

SOURCE="/mnt/c/Program Files/FundamentalUpdater_rev5/Data/Fundamental/fundamental_v4.csv"
SYNC="$HOME/FundamentalWeb/tools/sync_v4_csv.sh"

while true; do
    if [ ! -f "$SOURCE" ]; then
        echo "ERROR: V5 V4 CSV not found"
        sleep 5
        continue
    fi

    CURRENT_HASH=$(sha256sum "$SOURCE" | awk '{print $1}')

    if [ "$CURRENT_HASH" != "${LAST_HASH:-}" ]; then
        echo
        echo "===== V4 CSV CONTENT CHANGE DETECTED ====="
        date
        echo "SHA256: $CURRENT_HASH"

        if "$SYNC"; then
            LAST_HASH="$CURRENT_HASH"
        else
            echo "SYNC FAILED - will retry on next check"
        fi
    fi

    sleep 2
done

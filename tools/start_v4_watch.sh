#!/bin/bash

cd "$HOME/FundamentalWeb"

PIDFILE="tools/v4_watch.pid"
LOGFILE="tools/v4_watch.log"

if [ -f "$PIDFILE" ]; then
    PID=$(cat "$PIDFILE")

    if kill -0 "$PID" 2>/dev/null; then
        echo "V4 WATCH already running: PID $PID"
        exit 0
    fi

    rm -f "$PIDFILE"
fi

nohup ./tools/watch_v4_csv.sh > "$LOGFILE" 2>&1 &
PID=$!

echo "$PID" > "$PIDFILE"

echo "V4 WATCH STARTED"
echo "PID: $PID"
echo "LOG: $LOGFILE"

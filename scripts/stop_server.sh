#!/bin/bash
# The PID file is assumed to be in the parent directory of the scripts folder (project root)
PID_FILE="$(dirname "$0")/../server.pid"

if [ -f "$PID_FILE" ]; then
    kill "$(cat "$PID_FILE")" && rm "$PID_FILE"
    echo "Server stopped."
else
    echo "No server PID file found. Is the server running?"
fi

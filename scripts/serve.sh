#!/bin/bash
# Change directory to the output folder (assumes output is in the parent directory)
cd "$(dirname "$0")/../output" || { echo "Output folder not found."; exit 1; }

# Start the Python HTTP server on port 8000 in the background
python3 -m http.server 8000 &

# Save the process ID to a file in the parent directory (project root)
echo $! > ../server.pid

echo "Server started at http://localhost:8000"

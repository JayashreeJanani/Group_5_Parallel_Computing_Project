#!/bin/sh

set -eu

# Resolve project root relative to this script location.
SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
PROJECT_ROOT="$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)"

for subdir in data/output_serial data/output_openmp data/output_mpi; do
	dir="$PROJECT_ROOT/$subdir"
	if [ -d "$dir" ]; then
		find "$dir" -maxdepth 1 -type f -name '*.jpg' -delete
	fi
done

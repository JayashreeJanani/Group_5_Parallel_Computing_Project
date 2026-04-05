#!/bin/bash

set -euo pipefail

SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
PROJECT_ROOT="$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)"
LOG_FILE="$PROJECT_ROOT/mpi_outputs.txt"

if [ "$#" -ne 2 ]; then
	echo "Usage: $0 <num_processes> <small|medium|large>"
	exit 1
fi

PROCESSES="$1"
SIZE="$2"
case "$SIZE" in
	small|medium|large)
		INPUT_DIR="data/input/$SIZE"
		;;
	*)
		echo "Error: Invalid size '$SIZE'. Use one of: small, medium, large."
		exit 1
		;;
esac

cd "$PROJECT_ROOT"

# Create or overwrite log file for a fresh run.
cat > "$LOG_FILE" <<EOF
============================================================
MPI Benchmark Run
Started: $(date)
Working directory: $PROJECT_ROOT
Number of processes: $PROCESSES
Input dataset: $INPUT_DIR
============================================================

EOF

echo "[SETUP] Building mpi app with make mpi..." | tee -a "$LOG_FILE"
make mpi 2>&1 | tee -a "$LOG_FILE"
echo "[SETUP] Build complete." | tee -a "$LOG_FILE"
echo | tee -a "$LOG_FILE"

for iteration in 1 2 3 4 5; do
	{
		echo "------------------------------------------------------------"
		echo "Iteration $iteration/5"
		echo "Start time: $(date)"
		echo "Command: mpirun -np $PROCESSES ./mpi_app $INPUT_DIR"
		echo "------------------------------------------------------------"
	} | tee -a "$LOG_FILE"

	mpirun -np $PROCESSES ./mpi_app "$INPUT_DIR" 2>&1 | tee -a "$LOG_FILE"

	echo "[POST] Resetting outputs via scripts/reset_outputs.sh" | tee -a "$LOG_FILE"
	(cd scripts && ./reset_outputs.sh) 2>&1 | tee -a "$LOG_FILE"

	{
		echo "End time: $(date)"
		echo "Iteration $iteration complete."
		echo
	} | tee -a "$LOG_FILE"
done

{
	echo "============================================================"
	echo "All iterations completed successfully."
	echo "Finished: $(date)"
	echo "Log file: $LOG_FILE"
	echo "============================================================"
} | tee -a "$LOG_FILE"

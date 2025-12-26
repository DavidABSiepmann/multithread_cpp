#!/usr/bin/env bash
set -e
OUTDIR="${PWD}/outputs"
mkdir -p "$OUTDIR"
OUTCSV="$OUTDIR/results.csv"
PROFILE="$OUTDIR/profile_events.csv"

# Results header (overwrite file)
echo "threads,duration_s,work_us,run,processed,throughput_items_s" > "$OUTCSV"
# Create/clear profile events file with header
echo "thread,time,status" > "$PROFILE"

BIN="${PWD}/build/pipelines_cpp"
if [ ! -x "$BIN" ]; then
  echo "Build not found. Running build..."
  ./scripts/build.sh
fi

# Parameter grid (customize as needed)
THREADS_LIST=(1 2 4)
WORK_US_LIST=(0 10 100)
DURATION_LIST=(1 3)
REPEATS=3
WARMUP=1

for t in "${THREADS_LIST[@]}"; do
  for w in "${WORK_US_LIST[@]}"; do
    for d in "${DURATION_LIST[@]}"; do
      echo "Running bench: threads=$t work_us=$w duration=$d"
      # Program prints a couple header lines; filter them out and append data
      "$BIN" --threads "$t" --duration "$d" --work-us "$w" --warmup "$WARMUP" --repeats "$REPEATS" --seed 42 --out "$OUTCSV" --profile "$OUTDIR/profile_events.csv" >/dev/null
      # program writes results to $OUTCSV and profile events to $OUTDIR/profile_events.csv
    done
  done
done

echo "Bench finished, results -> $OUTCSV"

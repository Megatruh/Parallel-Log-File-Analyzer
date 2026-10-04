#!/usr/bin/env bash
# bench.sh - jalankan semua konfigurasi percobaan dan simpan hasilnya ke results/
#
# Pemakaian : ./script/bench.sh
# Parameter (bisa diubah lewat environment, contoh: ROUNDS=30 REPS=5 ./script/bench.sh)
#   ROUNDS   beban CPU per baris untuk versi final      (default 30)
#   SIZES    ukuran data (jumlah baris), dipisah spasi  (default 25%, 50%, 100% dari 660000)
#   WORKERS  variasi jumlah thread/process              (default "1 2 4 8")
#   REPS     pengulangan tiap konfigurasi, diambil yang tercepat (default 3)
set -euo pipefail

cd "$(dirname "$0")/.."

ROUNDS=${ROUNDS:-30}
SIZES=${SIZES:-"165000 330000 660000"}
WORKERS=${WORKERS:-"1 2 4 8"}
REPS=${REPS:-3}
SEED=247006111066
BUG_THREADS=${BUG_THREADS:-8}

mkdir -p results
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

echo ">> Build (ROUNDS=$ROUNDS)"
make clean >/dev/null
make all ROUNDS="$ROUNDS" >/dev/null

# --- catat spesifikasi mesin dan parameter (wajib ada di laporan) ---
{
  echo "Tanggal   : $(date '+%Y-%m-%d %H:%M:%S')"
  echo "nproc     : $(nproc)"
  lscpu | grep -E "Model name|^CPU\(s\)|Thread|Core|Socket|CPU max MHz|L3 cache" || true
  echo "ROUNDS    : $ROUNDS"
  echo "SIZES     : $SIZES"
  echo "WORKERS   : $WORKERS"
  echo "REPS      : $REPS (diambil waktu proses tercepat)"
  echo "Kernel    : $(uname -r)"
} > results/system.txt
cat results/system.txt

echo "data,mode,n,lines,t_read,t_proc,checksum,valid" > results/results.csv
echo "data,mode,n,lines,t_read,t_proc,checksum" > results/raw.csv

# best <file> <mode> <n> : jalankan REPS kali, cetak baris CSV tercepat (kolom ke-5 = t_proc)
best() {
  local runs
  runs=$(for _ in $(seq "$REPS"); do ./main_final "$1" "$2" "$3" -q; done)
  echo "$runs" | sed "s/^/$N,/" >> results/raw.csv
  echo "$runs" | sort -t, -k5 -n | head -1
}

for N in $SIZES; do
  F="$TMP/data_$N.log"
  ./gen_log "$F" "$N" "$SEED" >/dev/null
  echo ">> Data $N baris"

  seq_row=$(best "$F" seq 1)
  seq_sum=$(echo "$seq_row" | cut -d, -f6)
  echo "$N,$seq_row,1" >> results/results.csv

  for mode in thread proc; do
    for w in $WORKERS; do
      row=$(best "$F" "$mode" "$w")
      sum=$(echo "$row" | cut -d, -f6)
      valid=1; [ "$sum" = "$seq_sum" ] || valid=0
      [ "$valid" = 1 ] || echo "   PERINGATAN: $mode $w menghasilkan checksum berbeda dari sequential!"
      echo "$N,$row,$valid" >> results/results.csv
      printf "   %-6s %-2s  t_proc=%s s\n" "$mode" "$w" "$(echo "$row" | cut -d, -f5)"
    done
  done
  rm -f "$F"
done

# --- bukti bug race condition untuk laporan ---
BIGN=$(echo "$SIZES" | awk '{print $NF}')
./gen_log "$TMP/bug.log" "$BIGN" "$SEED" >/dev/null
./main_bug "$TMP/bug.log" "$BUG_THREADS" 5 > results/bug_run.txt

echo ">> Selesai. File: results/results.csv, results/raw.csv, results/system.txt, results/bug_run.txt"
echo ">> Lanjut: python3 script/plot.py"
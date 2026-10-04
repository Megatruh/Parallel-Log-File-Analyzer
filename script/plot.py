#!/usr/bin/env python3
"""plot.py - membuat 3 grafik (landscape) dan tabel hasil dari results/results.csv"""
import csv
import os
import sys

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
RES = os.path.join(ROOT, "results")
SRC = os.path.join(RES, "results.csv")
if not os.path.exists(SRC):
    sys.exit("results/results.csv belum ada. Jalankan ./script/bench.sh dulu.")

rows = []
with open(SRC) as f:
    for r in csv.DictReader(f):
        rows.append({"data": int(r["data"]), "mode": r["mode"], "n": int(r["n"]),
                     "t": float(r["t_proc"]), "valid": int(r["valid"])})

sizes = sorted({r["data"] for r in rows})
seq = {r["data"]: r["t"] for r in rows if r["mode"] == "seq"}


def series(mode, size):
    pts = sorted((r["n"], r["t"]) for r in rows if r["mode"] == mode and r["data"] == size)
    return [p[0] for p in pts], [p[1] for p in pts]


def waktu_vs(mode, label, fname):
    plt.figure(figsize=(10, 5.5))
    for size in sizes:
        x, y = series(mode, size)
        plt.plot(x, y, marker="o", label=f"{size:,} baris")
        plt.axhline(seq[size], ls=":", lw=1, alpha=.5)
    plt.xlabel(f"Jumlah {label}")
    plt.ylabel("Waktu proses (detik)")
    plt.title(f"Waktu vs Jumlah {label}  (garis putus-putus = sequential)")
    plt.xticks(sorted({r["n"] for r in rows if r["mode"] == mode}))
    plt.grid(alpha=.3)
    plt.legend()
    plt.tight_layout()
    plt.savefig(os.path.join(RES, fname), dpi=150)
    plt.close()


waktu_vs("thread", "Thread", "grafik_waktu_thread.png")
waktu_vs("proc", "Process", "grafik_waktu_process.png")

# --- Speedup vs Konfigurasi (batang berkelompok per ukuran data) ---
configs = [("thread", n) for n in sorted({r["n"] for r in rows if r["mode"] == "thread"})] + \
          [("proc", n) for n in sorted({r["n"] for r in rows if r["mode"] == "proc"})]
labels = [("T" if m == "thread" else "P") + str(n) for m, n in configs]
width = 0.8 / len(sizes)
plt.figure(figsize=(10, 5.5))
for i, size in enumerate(sizes):
    sp = []
    for m, n in configs:
        t = next(r["t"] for r in rows if r["data"] == size and r["mode"] == m and r["n"] == n)
        sp.append(seq[size] / t)
    xs = [k + i * width for k in range(len(configs))]
    plt.bar(xs, sp, width, label=f"{size:,} baris")
plt.axhline(1, color="gray", ls="--", lw=1)
plt.xticks([k + 0.4 - width / 2 for k in range(len(configs))], labels)
plt.xlabel("Konfigurasi (T = thread, P = process)")
plt.ylabel("Speedup terhadap sequential")
plt.title("Speedup vs Konfigurasi")
plt.grid(axis="y", alpha=.3)
plt.legend()
plt.tight_layout()
plt.savefig(os.path.join(RES, "grafik_speedup.png"), dpi=150)
plt.close()

# --- Tabel hasil (Markdown) untuk disalin ke laporan ---
lines = ["| Data (baris) | Mode | Worker | Waktu proses (s) | Speedup | Efisiensi (%) | Valid |",
         "|---:|---|---:|---:|---:|---:|:---:|"]
for size in sizes:
    for r in sorted((r for r in rows if r["data"] == size), key=lambda r: (r["mode"] != "seq", r["mode"], r["n"])):
        sp = seq[size] / r["t"]
        lines.append(f"| {size:,} | {r['mode']} | {r['n']} | {r['t']:.3f} | {sp:.2f}x | {100 * sp / r['n']:.1f} | "
                     f"{'ya' if r['valid'] else 'TIDAK'} |")
with open(os.path.join(RES, "tabel.md"), "w") as f:
    f.write("\n".join(lines) + "\n")

print("\n".join(lines))
print("\nTersimpan di results/: grafik_waktu_thread.png, grafik_waktu_process.png, grafik_speedup.png, tabel.md")
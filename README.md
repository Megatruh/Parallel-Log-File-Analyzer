# Parallel Log File Analyzer

Proyek UTS mata kuliah **Komputasi Paralel dan Terdistribusi** (Informatika, Universitas Siliwangi).

- **Nama**: [NAMA LENGKAP]
- **NPM**: 247006111066
- **Dosen pengampu**: Rohmat Gunawan, M.T.
- **Tema**: *Parallel computing in our lives*
- **Bahasa**: C (gcc, POSIX threads, `fork`)

## Deskripsi

Program ini menganalisis file log server berukuran besar (ratusan ribu baris) dan menghitung:

- jumlah log per level (`INFO`, `WARNING`, `ERROR`)
- jumlah per status code (200, 301, 403, 404, 500, 503)
- distribusi alamat IP (dihitung per oktet terakhir)
- checksum XOR dari fingerprint tiap baris, untuk membuktikan semua mode menghasilkan hasil yang sama

Masalah yang sama diselesaikan dengan **tiga pendekatan** lalu dibandingkan performanya:

| Pendekatan | Cara kerja |
|---|---|
| Sequential | Satu alur memproses seluruh file |
| Multithread | File dibagi ke N thread (`pthread`); tiap thread memegang hasil lokal, digabung di akhir |
| Multiprocessing | File dibagi ke N proses anak (`fork`); hasil dikirim ke proses induk lewat `pipe` |

## Format log

```
2026-10-02 10:15:32 [ERROR] 10.0.0.57 GET /login 404
```

Data dibuat oleh generator sintetis dengan seed dari NPM, sehingga hasilnya reproducible.

## Struktur proyek

```
.
├── app/
│   ├── gen_log.c        # generator data log
│   ├── main_bug.c       # versi berbug: race condition
│   └── main_final.c     # versi final: seq / thread / proc
├── include/             # header tiap modul
│   ├── config.h         # identitas, parameter, konstanta
│   ├── common.h         # struct Result dan Parsed
│   ├── timer.h
│   ├── loader.h
│   ├── parser.h
│   ├── chunker.h
│   ├── analyzer.h
│   ├── report.h
│   ├── seq_runner.h
│   ├── thread_runner.h
│   └── proc_runner.h
├── src/                 # implementasi tiap modul
├── script/
│   ├── bench.sh         # menjalankan semua konfigurasi percobaan
│   └── plot.py          # membuat grafik dari hasil percobaan
├── results/             # CSV dan grafik hasil percobaan
└── Makefile
```

### Peran modul

| Modul | Tanggung jawab |
|---|---|
| `timer` | Pengukuran waktu wall-clock (`CLOCK_MONOTONIC`) |
| `loader` | Membaca seluruh file log ke memori |
| `parser` | Mengurai satu baris log menjadi level, oktet IP, status code, dan fingerprint |
| `chunker` | Membagi buffer menjadi N chunk yang selalu dimulai di awal baris |
| `analyzer` | Menghitung statistik pada satu rentang, serta menggabungkan hasil |
| `report` | Mencetak banner Nama + NPM, hasil, waktu, speedup, dan efisiensi |
| `seq_runner` / `thread_runner` / `proc_runner` | Tiga strategi eksekusi yang memakai `analyzer` yang sama |

## Kebutuhan

- Linux (dikembangkan di Fedora 44 KDE)
- `gcc`, `make`
- `python3` dan `matplotlib` untuk grafik

```bash
sudo dnf install gcc make python3-matplotlib
```

## Cara pakai

> Bagian build dan runner masih dalam pengerjaan. Perintah di bawah adalah rencana antarmukanya.

```bash
# build semua target
make

# buat data uji (default: 660.000 baris, seed = NPM)
./gen_log data.log
./gen_log data.log 200000          # jumlah baris tertentu

# jalankan versi final
./main_final data.log seq 1        # sequential
./main_final data.log thread 4     # 4 thread
./main_final data.log proc 4       # 4 proses

# jalankan versi berbug (race condition)
./main_bug data.log 8

# percobaan dan grafik
./script/bench.sh
python3 script/plot.py
```

## Bug paralel yang dipelajari

**Race condition.** `main_bug` membuat banyak thread menaikkan penghitung pada satu `Result` global tanpa sinkronisasi. Operasi `counter++` bukan atomik (baca, tambah, tulis), sehingga sebagian pembaruan hilang dan hasil berbeda dari referensi sekuensial.

**Perbaikan di `main_final`.** Setiap worker memakai `Result` lokal. Hasilnya digabung sekali di akhir (reduction), jadi tidak ada data bersama di loop utama. Setiap `Result` juga di-align 64 byte agar tidak terjadi *false sharing* antar thread.

## Parameter percobaan

- Seed generator: NPM `247006111066`
- Ukuran data dasar: `66 × 10.000 = 660.000` baris (dua digit terakhir NPM); variasi 25%, 50%, 100%
- Variasi thread dan process: 1, 2, 4, 8
- Beban CPU per baris diatur lewat `ROUNDS` (default 100), bisa diubah dengan `-DROUNDS=...`

## Status pengerjaan

- [x] `config.h`, `common.h`
- [x] `timer`
- [x] `gen_log`
- [x] `loader`, `parser`
- [x] `chunker`, `analyzer`
- [ ] `report`, `seq_runner`
- [ ] `thread_runner`, `main_bug`, `main_final`
- [ ] `proc_runner`
- [ ] `Makefile`
- [ ] `bench.sh`, `plot.py`
- [ ] Laporan PDF (`UTS_NIM_Nama.pdf`)

## Lisensi

Proyek ini dibuat untuk keperluan tugas kuliah.
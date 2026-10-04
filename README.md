# Parallel Log File Analyzer

Proyek UTS mata kuliah **Komputasi Paralel dan Terdistribusi** (Informatika, Universitas Siliwangi).

- **Nama**: Farhan Esha Putra Kusuma Atmaja
- **NPM**: 247006111066
- **Dosen pengampu**: Rohmat Gunawan, M.T.
- **Tema**: *Parallel computing in our lives*
- **Bahasa**: C (gcc, POSIX threads, `fork`)

## Deskripsi

Program ini menganalisis file log server berukuran besar (ratusan ribu baris) dan menghitung:

- jumlah log per level (`INFO`, `WARNING`, `ERROR`)
- jumlah per status code (200, 301, 403, 404, 500, 503)
- distribusi alamat IP (dihitung per oktet terakhir) beserta 3 IP teratas
- checksum XOR dari fingerprint tiap baris, untuk membuktikan semua mode menghasilkan hasil yang sama

Masalah yang sama diselesaikan dengan **tiga pendekatan** lalu dibandingkan performanya:

| Pendekatan | Cara kerja |
|---|---|
| Sequential | Satu alur memproses seluruh file |
| Multithread | File dibagi ke N thread (`pthread`); tiap thread memegang hasil lokal, digabung di akhir |
| Multiprocessing | File dibagi ke N proses anak (`fork`); hasil dikirim ke proses induk lewat `pipe` |

Setiap baris diberi beban CPU tambahan (hash FNV-1a yang diulang `ROUNDS` kali) agar pekerjaan cukup berat dan keuntungan paralelisme bisa diamati.

## Format log

```
2026-10-02 10:15:32 [ERROR] 10.0.0.57 GET /login 404
```

Data dibuat oleh generator sintetis (`gen_log`) dengan seed dari NPM, sehingga hasilnya reproducible.

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
├── tests/
│   └── test_chunker.c   # uji: hasil dibagi N chunk == hasil satu kali jalan
├── script/
│   ├── bench.sh         # menjalankan semua konfigurasi percobaan
│   └── plot.py          # membuat grafik dan tabel dari hasil percobaan
├── results/             # CSV, tabel, dan grafik hasil percobaan
└── Makefile
```

### Peran modul

| Modul | Tanggung jawab |
|---|---|
| `timer` | Pengukuran waktu wall-clock (`CLOCK_MONOTONIC`) |
| `loader` | Membaca seluruh file log ke memori |
| `parser` | Mengurai satu baris log menjadi level, oktet IP, status code, dan fingerprint |
| `chunker` | Membagi buffer menjadi N chunk yang selalu dimulai di awal baris |
| `analyzer` | Menghitung statistik pada satu rentang, menggabungkan hasil, dan membandingkan hasil |
| `report` | Mencetak banner Nama + NPM, hasil, waktu, speedup, efisiensi, dan baris CSV |
| `seq_runner` / `thread_runner` / `proc_runner` | Tiga strategi eksekusi yang memakai `analyzer` yang sama dan berbagi satu tanda tangan fungsi |

Aturan antarmuka: runner tidak tahu cara parsing, `analyzer` tidak tahu soal thread atau process, dan hanya `report` serta `main_*` yang mencetak ke layar.

## Kebutuhan

- Linux (dikembangkan di Fedora 44 KDE)
- `gcc`, `make`
- `python3` dan `matplotlib` untuk grafik

```bash
sudo dnf install gcc make python3-matplotlib
```

## Build

```bash
make                  # membangun gen_log, main_final, main_bug
make final            # hanya main_final
make bug              # hanya main_bug
make clean            # hapus build/ dan semua binary
```

Parameter build yang bisa diubah dari command line:

| Variabel | Default | Fungsi |
|---|---|---|
| `ROUNDS` | 100 | Beban CPU per baris untuk `main_final` |
| `BUG_ROUNDS` | 1 | Beban CPU per baris untuk `main_bug` (kecil agar race mudah muncul) |

> **Penting:** `make` hanya melihat waktu modifikasi file, bukan nilai `ROUNDS`. Setiap kali mengganti `ROUNDS`, jalankan `make clean` dulu, misalnya `make clean && make final ROUNDS=30`.
>
> Baris perintah di `Makefile` harus diawali karakter **TAB**, bukan spasi.

## Cara pakai

```bash
# buat data uji (default: 660.000 baris, seed = NPM)
./gen_log data.log
./gen_log data.log 200000            # jumlah baris tertentu

# jalankan versi final: ./main_final <file> <seq|thread|proc> <jumlah_worker> [-q] [-b]
./main_final data.log seq 1          # sequential
./main_final data.log thread 4       # 4 thread
./main_final data.log proc 4         # 4 proses
./main_final data.log thread 4 -b    # + jalankan baseline: speedup, efisiensi, verifikasi hasil
./main_final data.log thread 4 -q    # mode senyap: satu baris CSV

# jalankan versi berbug: ./main_bug <file> <jumlah_thread> [jumlah_percobaan]
./main_bug data.log 8 5
```

Contoh keluaran (ringkas):

```
============================================================
Parallel Log File Analyzer
MODE FINAL
By Farhan Esha Putra Kusuma Atmaja (247006111066)
============================================================
Total baris : 660000
 ...
Checksum        : caf8cf56d20f38d8
Verifikasi      : SAMA dengan hasil sequential
------------------------------------------------------------
Mode            : thread
Jumlah thread   : 4
Waktu proses    : ... detik
Speedup         : ...x
Efisiensi       : ... %
============================================================
```

Flag `-b` menjalankan sequential lebih dulu sebagai pembanding, lalu mencetak speedup, efisiensi, dan baris `Verifikasi` yang membandingkan hasil paralel dengan hasil sequential.

## Pengujian

`tests/test_chunker.c` membagi satu file log menjadi N chunk (N = 1, 2, 3, 4, 7, 8, 64, 1000), memproses tiap chunk, menggabungkannya, lalu membandingkan dengan hasil satu kali jalan. Semua N harus menghasilkan hasil yang sama.

```bash
gcc -Iinclude -Wall -Wextra -O2 -DROUNDS=5 tests/test_chunker.c \
    src/loader.c src/parser.c src/chunker.c src/analyzer.c -o /tmp/uji
./gen_log /tmp/e.log 5000 && /tmp/uji /tmp/e.log     # diharapkan: SEMUA OK
```

## Percobaan dan grafik

```bash
chmod +x script/bench.sh
./script/bench.sh                    # build ulang, jalankan semua konfigurasi
python3 script/plot.py               # buat grafik dan tabel
```

`bench.sh` selalu menjalankan `make clean` dan membangun ulang dengan `ROUNDS` yang diminta. Parameternya bisa diubah lewat environment:

| Variabel | Default | Fungsi |
|---|---|---|
| `ROUNDS` | 30 | Beban CPU per baris |
| `SIZES` | `165000 330000 660000` | Ukuran data (25%, 50%, 100% dari dasar) |
| `WORKERS` | `1 2 4 8` | Variasi jumlah thread dan process |
| `REPS` | 3 | Pengulangan tiap konfigurasi; diambil waktu proses tercepat |
| `BUG_THREADS` | 8 | Jumlah thread untuk demonstrasi bug |

Contoh uji cepat: `SIZES="20000 40000" WORKERS="1 2" REPS=1 ROUNDS=5 ./script/bench.sh`

Keluaran di `results/`:

| File | Isi |
|---|---|
| `results.csv` | Waktu terbaik tiap konfigurasi + kolom `valid` (checksum sama dengan sequential) |
| `raw.csv` | Semua hasil mentah tiap pengulangan |
| `system.txt` | Spesifikasi mesin dan parameter percobaan |
| `bug_run.txt` | Keluaran `main_bug` sebagai bukti race condition |
| `tabel.md` | Tabel waktu, speedup, dan efisiensi |
| `grafik_waktu_thread.png` | Waktu vs jumlah thread |
| `grafik_waktu_process.png` | Waktu vs jumlah process |
| `grafik_speedup.png` | Speedup vs konfigurasi |

## Parameter percobaan

- Seed generator: NPM `247006111066`
- Ukuran data dasar: `66 × 10.000 = 660.000` baris (dua digit terakhir NPM); variasi 25%, 50%, 100%
- Variasi thread dan process: 1, 2, 4, 8 (batas atas dijaga oleh `MAX_WORKER` di `config.h`)
- Beban CPU per baris: `ROUNDS=30` untuk percobaan, `ROUNDS=1` untuk demonstrasi bug

## Bug paralel yang dipelajari

**Race condition.** `main_bug` membuat banyak thread menaikkan penghitung pada satu `Result` global tanpa sinkronisasi. Operasi `counter++` bukan atomik (baca, tambah, tulis), sehingga sebagian pembaruan hilang dan hasil berbeda dari referensi sekuensial. Selain hasil salah, rebutan cache line pada penghitung bersama juga menimbulkan memory contention.

Contoh hasil (8 thread, 660.000 baris, `ROUNDS=1`): kelima percobaan salah, dengan 354.676 sampai 380.611 baris hilang, dan jumlahnya berbeda tiap percobaan. Checksum ikut berbeda.

**Perbaikan di `main_final`.** Setiap worker memakai `Result` lokal (fungsi yang dipanggil sama, hanya alamat hasilnya yang berbeda). Hasil digabung sekali di akhir (reduction), jadi tidak ada data bersama di loop utama dan tidak perlu mutex. Setiap `Result` juga di-align 64 byte agar tidak terjadi *false sharing* antar thread.

## Ringkasan hasil

Contoh satu kali percobaan (4 Oktober 2026), Intel Core i5-1335U (12 thread logis), Fedora 44, `ROUNDS=30`, data 660.000 baris. Semua hasil valid (checksum sama dengan sequential). Tabel lengkap ada di `results/tabel.md`.

| Mode | Worker | Waktu proses (s) | Speedup | Efisiensi |
|---|---:|---:|---:|---:|
| seq | 1 | 3,679 | 1,00x | 100% |
| thread | 2 | 1,917 | 1,92x | 96,0% |
| thread | 4 | 1,574 | 2,34x | 58,4% |
| thread | 8 | 0,879 | 4,18x | 52,3% |
| proc | 2 | 2,037 | 1,81x | 90,3% |
| proc | 4 | 1,547 | 2,38x | 59,5% |
| proc | 8 | 0,906 | 4,06x | 50,7% |

Catatan:

- Thread dan process hampir sama cepat. Program ini CPU-bound: waktu baca file sekitar 1,6% dari total dan komunikasi antar proses hanya beberapa KB lewat pipe.
- Efisiensi turun saat worker bertambah. CPU uji bersifat hybrid (P-core dan E-core), sehingga pembagian data rata menurut byte menyebabkan worker di core yang lebih lambat menjadi penentu waktu total. Ini hipotesis yang konsisten dengan data, belum diuji terpisah.
- Hasil bervariasi sekitar 4 sampai 5% antar run, sehingga tiap konfigurasi diulang dan diambil yang tercepat.

## Status pengerjaan

- [x] `config.h`, `common.h`
- [x] `timer`
- [x] `gen_log`
- [x] `loader`, `parser`
- [x] `chunker`, `analyzer`
- [x] `report`, `seq_runner`
- [x] `thread_runner`, `proc_runner`
- [x] `main_bug`, `main_final`
- [x] `Makefile`
- [x] `bench.sh`, `plot.py`
- [x] Percobaan dan grafik
- [ ] Diagram arsitektur
- [ ] Laporan PDF (`UTS_NIM_Nama.pdf`)

## Lisensi

Proyek ini dibuat untuk keperluan tugas kuliah.
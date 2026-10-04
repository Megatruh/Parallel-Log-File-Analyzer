# Makefile - Parallel Log File Analyzer
# Pemakaian: make | make bug | make final | make gen | make clean
# Contoh   : make ROUNDS=30        (ubah beban CPU versi final)
#            make BUG_ROUNDS=1     (ubah beban CPU versi bug)

CC         ?= gcc
WARN       := -Wall -Wextra
INC        := -Iinclude
ROUNDS     ?= 100
BUG_ROUNDS ?= 1

COMMON := timer loader parser chunker analyzer report
FINAL  := $(COMMON) seq_runner thread_runner proc_runner

FINAL_OBJ := $(addprefix build/final/, $(addsuffix .o, $(FINAL))) build/final/main_final.o
BUG_OBJ   := $(addprefix build/bug/,   $(addsuffix .o, $(COMMON))) build/bug/main_bug.o

.PHONY: all final bug gen clean
all: gen final bug

gen: gen_log
final: main_final
bug: main_bug

gen_log: app/gen_log.c include/config.h
	$(CC) $(WARN) $(INC) -O2 $< -o $@

main_final: $(FINAL_OBJ)
	$(CC) $^ -o $@ -pthread

main_bug: $(BUG_OBJ)
	$(CC) $^ -o $@ -pthread

# --- aturan kompilasi versi final (-O2) ---
build/final/%.o: src/%.c | build/final
	$(CC) $(WARN) $(INC) -O2 -pthread -DROUNDS=$(ROUNDS) -MMD -MP -c $< -o $@
build/final/%.o: app/%.c | build/final
	$(CC) $(WARN) $(INC) -O2 -pthread -DROUNDS=$(ROUNDS) -MMD -MP -c $< -o $@

# --- aturan kompilasi versi bug (-O1, beban CPU kecil agar race mudah muncul) ---
build/bug/%.o: src/%.c | build/bug
	$(CC) $(WARN) $(INC) -O1 -pthread -DROUNDS=$(BUG_ROUNDS) -MMD -MP -c $< -o $@
build/bug/%.o: app/%.c | build/bug
	$(CC) $(WARN) $(INC) -O1 -pthread -DROUNDS=$(BUG_ROUNDS) -MMD -MP -c $< -o $@

build/final build/bug:
	mkdir -p $@

clean:
	rm -rf build gen_log main_final main_bug

-include $(wildcard build/final/*.d build/bug/*.d)
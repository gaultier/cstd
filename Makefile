.POSIX:
.SUFFIXES:

LD = lld

CFLAGS = -fpie -fno-omit-frame-pointer -gsplit-dwarf -march=native -fuse-ld=$(LD) -std=c23 -Wall -Wextra -Wno-cast-function-type-mismatch -Werror -g3

LDFLAGS = -flto

CC = clang

TEST_C_FILES = $(wildcard *.c)

SANITIZERS = address,undefined

.PHONY: test
test: test_debug.bin
	./$<

compile_flags.txt: 
	echo $(CFLAGS) | tr ' ' '\n' > $@

test_debug.bin: $(TEST_C_FILES)
	$(CC) $(CFLAGS) $(LDFLAGS) test.c -o $@ -Wno-unused

test_debug_sanitizer.bin: $(TEST_C_FILES)
	$(CC) $(CFLAGS) $(LDFLAGS) test.c -o $@ -fsanitize=$(SANITIZERS) -Wno-unused

test_release.bin: $(TEST_C_FILES)
	$(CC) $(CFLAGS) $(LDFLAGS) test.c -o $@ -O2 -flto -Wno-unused

test_release_sanitizer.bin: $(TEST_C_FILES)
	$(CC) $(CFLAGS) $(LDFLAGS) test.c -o $@ -O2 -flto -fsanitize=$(SANITIZERS) -Wno-unused

all: test_debug.bin test_debug_sanitizer.bin test_release.bin test_release_sanitizer.bin


.PHONY: clean
clean:
	rm *.o *.bin testdata/*.bin err_testdata/*.bin *.dwo || true

.PHONY: dev
dev: 
	ls *.{c,h} submodules/cstd/*.{c,h} | entr -c make test

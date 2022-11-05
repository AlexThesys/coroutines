TARGET = exec
LOCK=SPINLOCK # default value
TYPE=release
NUM_WORKERS=4
TASKS_PER_WORKER=1
CC = gcc
CFLAGS = -Wall -Wextra 
ifeq ($(TYPE),release)
	CFLAGS+= -O3
else
	CFLAGS+= -O0 -g
endif
LDFLAGS = -lpthread
ifeq ($(LOCK),MUTEX)
	LDFLAGS+= -no-pie
endif

$(TARGET): main.o lib.a
	$(CC) $^ $(CFLAGS) -o $(TARGET) $(LDFLAGS)

main.o: main.c
	$(CC) -c $(CFLAGS) $< -o $@ -D$(LOCK) -DNUM_WORKERS=$(NUM_WORKERS) -DTASKS_PER_WORKER=$(TASKS_PER_WORKER)

lib.a: lib.o
	ar rcs $@  $<

lib.o: lib.asm
	nasm -f elf64 -D$(LOCK) $< -o $@

clean:
	rm -f *.o *.a $(TARGET)

.PHONY: clean 

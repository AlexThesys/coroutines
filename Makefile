TARGET = exec
LOCK=SPINLOCK # default value
TYPE=release
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

#$(TARGET): main.0 lib.a
#	$(CC) $^ $(CFLAGS) -0 $@ $(LDFLAGS)

$(TARGET): main.o lib.a
	$(CC) $^ $(CFLAGS) -o $(TARGET) $(LDFLAGS)

main.o: main.c
	$(CC) -c $(CFLAGS) $< -o $@ -D$(LOCK)

lib.a: lib.o
	ar rcs $@  $<

lib.o: lib.asm
	nasm -f elf64 -D$(LOCK) $< -o $@

clean:
	rm -f *.o *.a $(TARGET)

.PHONY: clean 

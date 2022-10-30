TARGET = exec

LOCK=SPINLOCK # default value

CC = gcc
CFLAGS = -Wall -Wextra 
LDFLAGS = -lpthread
ifeq ($(LOCK),MUTEX)
	LDFLAGS+= -no-pie
endif

#$(TARGET): main.0 lib.a
#	$(CC) $^ $(CFLAGS) -0 $@ $(LDFLAGS)

debug: main.o lib.a
	$(CC) $^ $(CFLAGS) -Od -o $(TARGET) $(LDFLAGS)

release: main.o lib.a
	$(CC) $^ $(CFLAGS) -O3 -o $(TARGET) $(LDFLAGS)

main.o: main.c
	$(CC) -c $(CFLAGS) $< -o $@ -D$(LOCK)

lib.a: lib.o
	ar rcs $@  $<

lib.o: lib.asm
	nasm -f elf64 -D$(LOCK) $< -o $@

clean:
	rm -f *.o *.a $(TARGET)

.PHONY: clean 

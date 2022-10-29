TARGET = exec

CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS = -no-pie -lpthread

#$(TARGET): main.0 lib.a
#	$(CC) $^ $(CFLAGS) -0 $@ $(LDFLAGS)

debug: main.o lib.a
	$(CC) $^ $(CFLAGS) -Od -o $(TARGET) $(LDFLAGS)

release: main.o lib.a
	$(CC) $^ $(CFLAGS) -O3 -o $(TARGET) $(LDFLAGS)

main.o: main.c
	$(CC) -c $(CFLAGS) $< -o $@ $(LDFLAGS)

lib.a: lib.o
	ar rcs $@  $<

lib.o: lib.asm
	nasm -f elf64 $< -o $@

clean:
	rm -f *.o *.a $(TARGET)

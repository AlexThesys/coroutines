TARGET = exec

#$(TARGET): main.0 lib.a
#	gcc $^ -0 $@

debug: main.0 lib.a
	gcc $^ -Od -0 $(TARGET)

release: main.0 lib.a
	gcc $^ -O3 -0 $(TARGET)

main.o: main.c
	gcc -c $< -lpthread -o $@

lib.a: lib.o
	ar rcs $@  $<

lib.o: lib.asm
	nasm -f elf64 $< -o $@

clean:
	rm -f *.o *.a $(TARGET)

CC=g++
CFLAGS= -O3
objects = rasm.o rasm_lib.o

rasm: $(objects)
	$(CC) $(CFLAGS) -o rasm $(objects)

rasm.o: rasm.cpp rasm.h rasm_lib.h
	$(CC) $(CFLAGS) -c rasm.cpp rasm_lib.cpp

rasm_lib.o: rasm_lib.cpp rasm_lib.h
	$(CC) $(CFLAGS) -c rasm_lib.cpp

.PHONY : clean
clean:
	rm rasm $(objects)

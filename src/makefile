CC=g++
CFLAGS= -Ofast
objects = rasm_basic.o rasm_lib.o

rasm_basic: $(objects)
	$(CC) $(CFLAGS) -o rasm_basic $(objects)

rasm_basic.o: rasm_basic.cpp rasm_basic.h rasm_lib.h
	$(CC) $(CFLAGS) -c rasm_basic.cpp rasm_lib.cpp

rasm_lib.o: rasm_lib.cpp rasm_lib.h
	$(CC) $(CFLAGS) -c rasm_lib.cpp

.PHONY : clean
clean:
	rm rasm_basic $(objects)

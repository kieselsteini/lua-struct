CC=gcc -ansi -Wall -O2 -fPIC
OBJ=struct.o

default: $(OBJ)
	$(CC) -shared -o struct.so $(OBJ)

clean:
	rm -f $(OBJ)
	rm -f struct.so


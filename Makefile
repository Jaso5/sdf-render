objects = # help.o

prog:
	gcc src/main.c $(objects) -lm -o sdf-render

# help.o: src/help.h
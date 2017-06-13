nsh: main.o Parse.o Execute.o
	cc -o nsh main.o Parse.o Execute.o

Execute.o: Execute.c Execute.h Parse.h
	cc -c Execute.c
	
Parse.o: Parse.c Parse.h
	cc -c Parse.c	

main.o: main.c Execute.h Parse.h
	cc -c main.c
	
clean:
	rm -f main *.o
main: main.o SignalHandlers.o
	gcc -o main main.o SignalHandlers.o
main.o: main.c SignalHandlers.h
	gcc -Wall -g -c main.c
SignalHandlers.o: SignalHandlers.c SignalHandlers.h
	gcc -Wall -g -c SignalHandlers.c

clean:
	\rm -f *.o main command_history
DEBUG= 
EXECS= a4

all:	$(EXECS)

a4:	a4.c a4-util.o
	gcc $(DEBUG) -o a4 a4.c a4-util.o

clean:
	rm -f a4.o a4

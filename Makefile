COMPILE=gcc
CFLAGS=-c
CFLAGS+=-g

AR=ar
LINK=-L. mythread.a -static

all: clean mythread test

mythread:
	${COMPILE} ${CFLAGS} futex.c mythread.c myqueue.c
	${AR} rcs mythread.a futex.o mythread.o myqueue.o

test:
	${COMPILE} mythread_test.c ${LINK} -o test -g

clean:
	rm -rf *.a *.o test
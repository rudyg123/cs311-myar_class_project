#  Rudy Gonzalez
#  gonzalru@onid.orst.edu
#  CS311-400
#  Homework 4
#

CC=gcc
DEBUG=-g
CFLAGS=$(DEBUG) -Wall
PROGS=sig_demo myar


all: $(PROGS)

myar: myar.o
	$(CC) $(CFLAGS) -o myar myar.o

myar.o: myar.c
	$(CC) $(CFLAGS) -c myar.c
	
sig_demo: sig_demo.o
	$(CC) $(CFLAGS) -o sig_demo sig_demo.o

sig_demo.o: sig_demo.c
	$(CC) $(CFLAGS) -c sig_demo.c

testq12345: 
	rm -f ar12345.ar
	rm -f myar12345.ar
	ar q ar12345.ar 1-s.txt 2-s.txt 3-s.txt 4-s.txt 5-s.txt
	myar -q myar12345.ar 1-s.txt 2-s.txt 3-s.txt 4-s.txt 5-s.txt
	diff ar12345.ar myar12345.ar
	
testq135: 
	rm -f ar135.ar
	rm -f myar135.ar
	ar q ar135.ar 1-s.txt 3-s.txt 5-s.txt
	myar -q myar135.ar 1-s.txt 3-s.txt 5-s.txt
	diff ar135.ar myar135.ar

testq24: 
	rm -f ar24.ar
	rm -f myar24.ar
	ar q ar24.ar 2-s.txt 4-s.txt
	myar -q myar24.ar 2-s.txt 4-s.txt
	diff ar24.ar myar24.ar
	
testq: testq12345 testq135 testq24

testt12345:
	rm -f ar12345.ar
	ar q ar12345.ar 1-s.txt 2-s.txt 3-s.txt 4-s.txt 5-s.txt
	ar t ar12345.ar > ar-ctoc.txt
	myar -t ar12345.ar > myar-ctoc.txt
	diff ar-ctoc.txt myar-ctoc.txt

testt135:
	rm -f ar135.ar
	ar q ar135.ar 1-s.txt 3-s.txt 5-s.txt
	ar t ar135.ar > ar-ctoc.txt
	myar -t ar135.ar > myar-ctoc.txt
	diff ar-ctoc.txt myar-ctoc.txt

testt24:
	rm -f ar24.ar
	ar q ar24.ar 2-s.txt 4-s.txt
	ar t ar24.ar > ar-ctoc.txt
	myar -t ar24.ar > myar-ctoc.txt
	diff ar-ctoc.txt myar-ctoc.txt

testt: testt12345 testt135 testt24
	
testv12345:
	rm -f ar12345.ar
	ar q ar12345.ar 1-s.txt 2-s.txt 3-s.txt 4-s.txt 5-s.txt
	ar tv ar12345.ar > ar-vtoc.txt
	myar -v ar12345.ar > myar-vtoc.txt
	diff ar-vtoc.txt myar-vtoc.txt

testv135:
	rm -f ar135.ar
	ar q ar135.ar 1-s.txt 3-s.txt 5-s.txt
	ar tv ar135.ar > ar-vtoc.txt
	myar -v ar135.ar > myar-vtoc.txt
	diff ar-vtoc.txt myar-vtoc.txt

testv24:
	rm -f ar24.ar
	ar q ar24.ar 2-s.txt 4-s.txt
	ar tv ar24.ar > ar-vtoc.txt
	myar -v ar24.ar > myar-vtoc.txt
	diff ar-vtoc.txt myar-vtoc.txt

testv: testv12345 testv135 testv24

tests: testq testt testv

clean:
	rm -f $(PROGS) *.o *~


#Tim Schisler
#University of Missouri - St. Louis
#Operating Systems CS4760
#Assignment 3: Semaphores and Operating System Simulator
#10/08/2018

#This file borrows from lecture notes by Prof. Sanjiv Bhatia
#http://www.cs.umsl.edu/~sanjiv/classes/cs2750/lectures/make.pdf

CC = gcc
CFLAGS = -g
LIBFLAGS = -lrt

TARGET1 = oss
OBJ1 = oss.o

TARGET2 = user
OBJ2 = user.o

OBJC = P3common.o

LOG = ossLog.txt

.SUFFIXES: .c .o

all: $(TARGET1) $(TARGET2)

$(TARGET1): $(OBJ1) $(OBJC)
	$(CC) -o $@ $(LIBFLAGS) $(OBJ1) $(OBJC)
$(TARGET2): $(OBJ2) $(OBJC)
	$(CC) -o $@ $(LIBFLAGS) $(OBJ2) $(OBJC)
.c.o:
	$(CC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	/bin/rm -f *.o $(TARGET1) $(TARGET2) $(LOG)

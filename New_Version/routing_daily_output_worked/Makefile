# Makefile for calc_monthly_stats program
# Mon Mar 25 12:16:47 EST 2002
# Justin Sheffield <justin@princeton.edu>
CC = gcc
CFLAGS_linux  		= $(CFLAGS_BASE) -I/usr/local/Cellar/netcdf/4.2.1.1/include -g -Wall -Wno-unused
CFLAGS_osx = $(CFLAGS_BASE) -g -Wall -Wno-unused -I/usr/local/Cellar/netcdf/4.2.1.1/include
CFLAGS 			= $(CFLAGS_osx)
LIBRARY = -lm -L/usr/local/Cellar/netcdf/4.2.1.1/lib -lnetcdf

HDRS = 	route.h

OBJS =  route.o route_auxiliary.o route_flow.o read_vic_runoff.o write_flow.o

SRCS = $(OBJS:%.o=%.c) 

EXE = route

all:
	make model
	
default:
	make model

clean:
	/bin/rm -f $(EXE) *.o core log *~
	
	
model: $(OBJS)
	$(CC) -o $(EXE) $(OBJS) $(CFLAGS) $(LIBRARY)

$(OBJS): $(HDRS)

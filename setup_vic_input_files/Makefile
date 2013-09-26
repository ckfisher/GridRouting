# Makefile for calc_monthly_stats program

CC = cc
#CFLAGS_BASE		= -I. -I/home/tjtroy/Research/utils -I/home/tjtroy/Research/include
CFLAGS_BASE		= -I.
CFLAGS_irix  		= $(CFLAGS_BASE) -n32 -fullwarn
CFLAGS_sunos  		= $(CFLAGS_BASE) -O  
CFLAGS_linux  		= $(CFLAGS_BASE) -g -Wall -Wno-unused
#CFLAGS_linuxalpha  	= $(CFLAGS_BASE) -O3 -Wall -Wno-unused -mieee
#CFLAGS_linuxalpha  	= $(CFLAGS_BASE) -g -Wall -Wno-unused
CFLAGS 			= $(CFLAGS_linux)
LIBRARY = -lm
 
HDRS = 	setup_vic_input_files.h

OBJS =  setup_vic_input_files.o setup_vic_auxiliary.o 

SRCS = $(OBJS:%.o=%.c) 

EXE = setup_vic_input_files

all:
	make model
	
default:
	make model

clean:
	/bin/rm -f $(EXE) *.o core log *~
	
	
model: $(OBJS)
	$(CC) -o $(EXE) $(OBJS) $(CFLAGS) $(LIBRARY)

$(OBJS): $(HDRS)

CC = gcc
CFLAGS_linux  		= -fopenmp -I. -g -Wall -Wno-unused
CFLAGS_osx = -g -Wall -Wno-unused -I/usr/local/Cellar/netcdf/4.2.1.1/include
CFLAGS 			= $(CFLAGS_linux)
#LIBRARY = -lm -L/usr/local/Cellar/netcdf/4.2.1.1/lib -lnetcdf
LIBRARY = -lm -L/usr/lib64/libnetcdf.so.7 -lnetcdf
 

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

CINC = -I/usr/openwin/include -I/usr/local/include -I/opt/SUNWmotif/include -I/afs/eos/contrib/tcl80/src/tcl8.0/generic -I/afs/eos/contrib/tcl80/include
CLIBS = -lX11   
LIBPATH =  -L/afs/eos/users/t/tocrawle/2+2/tclXcom -ltcl8.0 -L/usr/openwin/lib -L/usr/lib -L/opt/gnu/lib 
RLIBPATH = -R /afs/eos/contrib/tcl80/lib
SHLIB_CFLAGS = -KPIC
SHLIB_LD = /usr/ccs/bin/ld -G -z text 

CC = cc

CFLAGS=$(CINC) -O -g 

all: tclXcom.so 

clean:
	-rm -f *~ *.o a.out core

tclXcom.so: tclXcom.c
	$(CC) -c $(CFLAGS) ${SHLIB_CFLAGS} tclXcom.c
	${SHLIB_LD}  $(LIBPATH) $(CLIBS) $(RLIBPATH) -o tclXcom.so tclXcom.o 
	









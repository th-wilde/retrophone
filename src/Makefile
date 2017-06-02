IDIR =include
CC=gcc
CFLAGS=-I$(IDIR) -g -std=gnu11 -Wno-multichar

ODIR=obj
LDIR =lib

#LIBS=-lwiringPi -lpthread
LIBS=-lpthread

_DEPS = 
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = retrophone.o rp_signalling.o rp_number2char.o rp_speak.o rp_voip.o popen2.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

retrophone: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 

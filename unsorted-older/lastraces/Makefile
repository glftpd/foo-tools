
include ../../src/Makefile.config

SRCDIR = ../../src
LIBS = $(LIBDIR)/lib/common.o $(LIBDIR)/collection/strlist.o $(LIBDIR)/lib/pwdfile.o \
	$(LIBDIR)/collection/sortedlist.o $(LIBDIR)/util/linefilereader.o \
	$(LIBDIR)/lib/stringtokenizer.o

TARGET = foo-lastraces

$(TARGET): lastraces.o
	$(CC) -o $@ $< $(LIBS)

all: $(TARGET)

clean:
	rm -f *.o *~ $(TARGET)

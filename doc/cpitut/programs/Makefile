CXX = oztool c++

OZDYNLD = oztool ld

CFLAGS = -I/home/ps-home/loeckelt/mozart/platform/emulator
LIBS = -lc -lm

CPPSRCS = ex_a.cc ex_b.cc ex_c.cc reified_less.cc constdata.cc

OBJS = $(CPPSRCS:.cc=.o)

CCSNIPPETS =    ex_a.1.cc ex_a.1a.cc \
                ex_a.2.cc \
                ex_a.3.cc ex_a.3a.cc ex_a.3b.cc \
                ex_a.4.cc \
                ex_a.5.cc \
                ex_a.6.cc \
                ex_b.2.cc \
                ex_b.3.cc ex_b.3a.cc ex_b.3b.cc \
                ex_b.4.cc \
                ex_b.5.cc \
                ex_c.1a.cc ex_c.1b.cc ex_c.1c.cc \
                ex_c.2.cc \
                ex_c.3.cc \
                ex_c.4.cc \
                ex_c.5.cc \
                ex_c.6.cc \
                ex_c.7.cc \
                constData.1.cc \
                constData.2.cc \
                constData.3.cc \
                constData.4.cc \
                constData.5.cc \
                reifiedLess.1.cc

M4 = m4

PLATFORM = $(shell ozplatform)

all: ex_a.so-$(PLATFORM) ex_b.so-$(PLATFORM) \
ex_c.so-$(PLATFORM) constdata.so-$(PLATFORM) \
reified_less.so-$(PLATFORM)

programs: ex_a.cc ex_b.cc ex_c.cc constdata.cc reified_less.cc

ex_a.so-$(PLATFORM): ex_a.o
        $(OZDYNLD) -o $@ ex_a.o $(LDFLAGS) $(LIBS)

ex_b.so-$(PLATFORM): ex_b.o
        $(OZDYNLD) -o $@ ex_b.o $(LDFLAGS) $(LIBS)

ex_c.so-$(PLATFORM): ex_c.o
        $(OZDYNLD) -o $@ ex_c.o $(LDFLAGS) $(LIBS)

constdata.so-$(PLATFORM): constdata.o
        $(OZDYNLD) -o $@ constdata.o $(LDFLAGS) $(LIBS)

reified_less.so-$(PLATFORM): reified_less.o
        $(OZDYNLD) -o $@ reified_less.o $(LDFLAGS) $(LIBS)


%.cc: %.m4cc $(CCSNIPPETS)
        $(M4) $< > $@

%.o: %.cc
        $(CXX) $(OPTIMIZE) $(CFLAGS) -c $< -o $@

clean:
        -rm *.o
        -rm *.so-$(PLATFORM)
        -rm *.s
        -rm *~

PLATFORM    = $(shell ozplatform)

M4          = m4
RM          = rm -f

CXX         = oztool c++

OZDYNLD     = oztool ld

#CFLAGS      = -I/usr/local/oz/include/
#LIBS        = -lc -lm

EXAMPLESM4  =   ex_a.m4cc \
		ex_c.m4cc \
		ex_b.m4cc \
		constdata.m4cc \
		reified_less.m4cc \
		sync.m4cc

EXAMPLES    = $(EXAMPLESM4:.m4cc=.cc)

NATIVEFNCTS = $(EXAMPLES:.cc=.so-$(PLATFORM))

CCSNIPPETS  =   ex_a.1.cc ex_a.1a.cc \
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
		constData.2a.cc \
		constData.2b.cc \
		constData.3.cc \
		constData.4.cc \
		constData.5.cc \
		reifiedLess.1.cc \
		sync.1.cc \
		sync.class.cc \
		sync.nfi.cc \
		sync.header.cc \
		sync.prop.cc

all: $(NATIVEFNCTS)

new: clean all

%.so-$(PLATFORM): %.o
	$(OZDYNLD) -o $@ $< $(LDFLAGS) $(LIBS)

%.cc: %.m4cc $(CCSNIPPETS)
	$(M4) $< > $@

%.o: %.cc
	$(CXX) $(OPTIMIZE) $(CFLAGS) -c $< -o $@

clean:
	$(RM) *.o
	$(RM) *.so-$(PLATFORM)
	$(RM) *.s
	$(RM) *~
	$(RM) $(EXAMPLESM4:.m4cc=.cc)

USECPLEX = 
#USECPLEX = 1

OPTIMIZE = -O3
#OPTIMIZE = -g3 -DDEBUG_CHECK

LIBS     = -lc -lm -llpk
TARGETLIBPATH  = $(HOME)/.oz/cache/http/www.ps.uni-sb.de/~tmueller/mozart

CFLAGS  = -I/usr/local/oz/include
CFLAGS   = -I../ReflectConstraints

CFLAGS   += -ILpsolve

CFLAGS   += -Wall -Wno-unused -Wno-reorder -Wno-uninitialized

LDFLAGS  = -LLpsolve


ifdef USECPLEX
CFLAGS   += -DINCLUDE_CPLEX -DALLWAYS_CLOSE_CPLEX
CFLAGS   += -I/linux/CPLEX
LDFLAGS  += -L/linux/CPLEX
LIBS     += -lcplex
endif

CFLAGS   += -I$(HOME)/mozart/platform/emulator 
CFLAGS   += -I$(HOME)/mozart.build/platform/emulator

CXX     = c++ -fno-exceptions -fno-rtti 

DEPEND   = $(CXX) $(CFLAGS) -MM

OZDYNLD = ozdynld

OZC     = ozc
OZL     = ozl


CPPSRCS  = 	aux.cc \
		ri.cc \
		builtins.cc \
		propagators.cc \
		lp.cc 

CSRCS    = fpgetset.c sigfpe.c

TARGZ    = ri.tar.gz

OBJS     = $(CPPSRCS:.cc=.o) $(CSRCS:.c=.o)

OZSRCS  = 	RI.oz \
		LP.oz

OZGSRCS  = 

OZFCTS  = $(OZSRCS:.oz=.ozf) $(OZGSRCS:.ozg=.ozf) 

PLATFORM = $(shell ozplatform)

all: ri.so-$(PLATFORM) $(OZFCTS)

#
# Oz part
#

%.ozf: %.oz 
	$(OZC) -c $< -o $@ 

%.ozf: %.ozg 
	$(OZC) -c --gump $< -o $@ 

#
# C part
#

ri.so-$(PLATFORM): $(OBJS) Lpsolve/liblpk.a
	$(OZDYNLD) -o $(TARGETLIBPATH)/$@ $^ $(LDFLAGS) $(LIBS)

Lpsolve/liblpk.a:
	(cd Lpsolve; make)
clean: 
	-rm *.ozf
	-rm *.o
	-rm *.so-$(PLATFORM)
	-rm *.s
	-rm *~
	-rm $(TARGZ)

veryclean: clean
	-rm Makefile.deps
	(cd Lpsolve; make clean)

%.o: %.cc 
	$(CXX) $(OPTIMIZE) $(CFLAGS) -c $< -o $@ 

%.o: %.c 
	$(CXX) $(OPTIMIZE) $(CFLAGS) -c $< -o $@ 


depend: 
	$(DEPEND) $(CSRCS) $(CPPSRCS) > Makefile.deps

tar: veryclean
	tar czvf $(TARGZ) --exclude $(TARGZ) .

-include Makefile.deps



PLATFORM = $(shell ozplatform)

CC= gcc -fno-exceptions -fno-rtti -fpic
#CC= oztool cc

#should be OK in most situations:
#CFLAGS= -O

# HP/UX 9.0X optimized code
#CFLAGS= +O3 +Oaggressive +Olibcalls -Aa -D_POSIX_SOURCE -DCHECK +FP VZOUiD
# HP/UX 9.0X debugging
#CFLAGS= -g -Aa -D_POSIX_SOURCE -DCHECK +FP VZOUiD

# nice for gcc
CFLAGS= -O3 -Wall -pedantic -ansi
#CFLAGS= -g -Wall -pedantic -ansi

# Option -DCHECK checks for numerical problems during rounding of numbers.
# It will slow things down a bit.
# You can add a -DREAL=<float type> to the CFLAGS, to change the default float
# type used in lp_solve (double) to float or 'long double'. However, type float
# might be fast on your computer, but it is not accurate enough to solve even
# moderately sized problems without running into numerical problems.
# The use of long doubles does increase the numerical stability of lp_solve,
# if your compiler actually implements them with more bits than a double. But
# it slows down things quite a bit.

# Choose your favorite or available version of lex and yacc

#YACC= yacc
YACC= bison -y

#LEX= lex
LEX= flex -l

LPKSRC.c= lpkit.c solve.c debug.c read.c readmps.c hash.c presolve.c
LEXFILE.l= lex.l
YACCFILE.y= lp.y
TESTFILES= lp_examples/ex1.lp lp_examples/ex2.lp lp_examples/ex3.lp lp_examples/ex4.lp lp_examples/ex5.lp lp_examples/ex6.lp lp_examples/ex7.lp

TARGET=lp_solve
LPKLIB=liblpk.a

LEXFILE.c= $(LEXFILE.l:.l=.c)
YACCFILE.c= $(YACCFILE.y:.y=.c)
YACCFILE.o= $(YACCFILE.y:.y=.o)
CSOURCES=lpkit.c solve.c debug.c read.c readmps.c lp_solve.c demo.c hash.c presolve.c $(LEXFILE.c) $(YACCFILE.c)
COBJ=$(CSOURCES:.c=.o)
LPKSRC= $(LPKSRC.c) $(YACCFILE.c)
LPKOBJ= $(LPKSRC:.c=.o)
HEADERS=lpkit.h lpglob.h patchlevel.h debug.h read.h hash.h

all:	demo $(TARGET) lp2mps mps2lp

$(COBJ): $(HEADERS)

demo:	demo.o $(LPKLIB)
	$(CC) -o demo $(CFLAGS) demo.o $(LPKLIB) -lm

lp2mps:	lp2mps.o $(LPKLIB)
	$(CC) -o lp2mps $(CFLAGS) lp2mps.o $(LPKLIB) -lm

mps2lp: mps2lp.o $(LPKLIB)
	$(CC) -o mps2lp $(CFLAGS) mps2lp.o $(LPKLIB) -lm

$(TARGET): lp_solve.o $(LPKLIB)
	$(CC) -o $(TARGET) $(CFLAGS) lp_solve.o $(LPKLIB) -lm

purify: lp_solve.o $(LPKLIB)
	purify $(CC) -o $(TARGET) $(CFLAGS) lp_solve.o $(LPKLIB) -lm

quantify: lp_solve.o $(LPKLIB)
	quantify $(CC) -o $(TARGET) $(CFLAGS) lp_solve.o $(LPKLIB) -lm

$(LPKLIB): $(LPKOBJ)
	ar rv $@ $(LPKOBJ)
	ranlib $(LPKLIB)
	cp -f $(LPKLIB) ../$(LPKLIB:%.a=%-$(PLATFORM).a)

$(YACCFILE.o): $(LEXFILE.c)

$(LEXFILE.c): $(LEXFILE.l)
	$(LEX) $(LEXFILE.l)
	mv lex.yy.c $(LEXFILE.c)

$(YACCFILE.c): $(YACCFILE.y)
	$(YACC) $(YACCFILE.y)
	mv y.tab.c $(YACCFILE.c)

test:
	-for i in $(TESTFILES); do\
		./$(TARGET) -p -s -time < $$i > xxx.tmp;\
		if diff xxx.tmp lp_examples/`basename $$i .lp`.out > /dev/null; then\
			echo "$$i gives the correct result";\
		else\
			echo "*** $$i gives different result, please check ***";\
		fi;\
	done;\
	rm xxx.tmp

$(TARGET).man: $(TARGET).1
	nroff -man $(TARGET).1 > $(TARGET).man

MANIFEST: clean
	ls -lR > MANIFEST; ls -lR > MANIFEST

clean:
	rm -f *.a *.o TAGS $(LEXFILE.c) $(YACCFILE.c) demo $(TARGET) lp2mps mps2lp .pure .softdebughist datafile

distrib: $(TARGET).man MANIFEST
	cd ..; tar -czvf lp_solve_2.2.tar.gz lp_solve_2.2

TAGS:
	etags *.[chyl]

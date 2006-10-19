MOZART-SRC=/home/javimena/work/mo
GEOZ=..
GECODE=/opt/gecode-1.0.1
CFLAGS=-ggdb -Wno-comment -fexceptions
#CFLAGS=-ggdb -Wall -Wno-comment -Wfatal-errors -fexceptions

#GeAutoIntPropagators.so-linux-i486: GeAutoIntPropagators.o IntBasicMacros.o
#	oztool ld -L/home/javimena/work/gecode-bin-1.1.0/lib $^ -o GeIntVar-Propagators.so-linux-i486 -lgecodekernel -lgecodeint -lgecodeset

#IntBasicMacros.o: IntBasicMacros.cc IntBasicMacros.hh
#	oztool c++ -c ${CFLAGS} -I${MOZART-SRC} -I${MOZART-SRC}/platform/emulator -I${GEOZ} -I${GECODE}/include/gecode $<

#GeIntVar-auto-builtins.o:  GeIntVar-auto-builtint.cc IntBasicMacros.hh
#	oztool c++ -c ${CFLAGS} -I${MOZART-SRC} -I${MOZART-SRC}/platform/emulator -I${GEOZ} -I${GECODE}/include/gecode $<

GeIntVar-auto-builtins.cc: GProp2Oz var_equivalences.txt template_file.cc dump-geint-noboolvars-1.0.0.txt
	./GProp2Oz  -q -i dump-geint-noboolvars-1.0.0.txt -b GeIntVar-builtins.hh > GeIntVar-auto-builtins.icc

GProp2Oz: GProp2Oz.cc symbols_constants.hh
	g++ ${CFLAGS} -o GProp2Oz GProp2Oz.cc

.PHONY: clean

clean:
	rm -f *.o GeIntVar-auto-* *~

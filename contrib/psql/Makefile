#
# Author:
#   Lars Rasmusson (Lars.Rasmusson@sics.se)
#
# Copyright:
#   Lars Rasmusson, 1998
#
# Last change:
#   $Date$ by $Author$
#   $Revision$
#
# This file is part of Mozart, an implementation
# of Oz 3
#    http://mozart.ps.uni-sb.de
#
# See the file "LICENSE" or
#    http://mozart.ps.uni-sb.de/LICENSE.html
# for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#
#

PSQLDIR = /usr/local/pgsql

LDLIBPATH = -L/usr/lib -L$(PSQLDIR)/lib
LDLIBS = -lpq -lcrypt

INCDIRS = -I. -I$(OZHOME)/include  -I$(PSQLDIR)/include

CC        = oztool c++
CCFLAGS   = -Wall
LDFLAGS   = $(LDLIBPATH) $(LDLIBS)
LD        = oztool ld
OZ        = ozc

SONAME    = PsqlLib.so-linux-i486
OBJ       = PsqlLib.o

all:    $(SONAME) Psql.ozf test

test: test.oz
        ozc -x test.oz

Psql.ozf: Psql.oz
        ozc -c Psql.oz


$(SONAME):      $(OBJ)
        $(LD)  -o $@ $(OBJ) $(LDFLAGS)


.cc.o:
        $(CC)  -c $*.cc $(CCFLAGS) $(INCDIRS)

# Dependencies

PsqlLib.o : PsqlLib.hh PsqlLib.cc

# Good stuff

clean::
        rm -f *~
        rm -f *.o
        rm -f core

veryclean::
        make clean
        rm -f $(EXENAME)
        rm -f $(SONAME)
        rm -f *ozf
        rm -f test

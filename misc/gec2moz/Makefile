#
#  Authors:
#    Diana Lorena Velasco <dlvelasco@puj.edu.co>
#    Juan Gabriel Torres  <juantorres@puj.edu.co>
# 
#  Contributors:
# 
#  Copyright:
#    Diana Lorena Velasco, 2007
#    Juan Gabriel Torres, 2007
# 
#  Last change:
#    $Date$ by $Author$
#    $Revision$
# 
#  This file is part of Mozart, an implementation 
#  of Oz 3:
#     http://www.mozart-oz.org
# 
#  See the file "LICENSE" or
#     http://www.mozart-oz.org/LICENSE.html
#  for information on usage and redistribution 
#  of this file, and for a DISCLAIMER OF ALL 
#  WARRANTIES.
#
#


CFLAGS = -Wall -ggdb
OBJS = parser.o lex.yy.o translate.o

all: $(OBJS)
	g++ -o transProp $(OBJS) -lfl

translate.o: translate.c lexer.h parser.h
	g++ $(CFLAGS) -c $<

lex.yy.o: lex.yy.c lexer.h parser.h
	g++ $(CFLAGS) -c $<

lex.yy.c: lexer.l  parser.h
	flex -i lexer.l

parser.c: parser.y
	bison -tdvl -o  parser.c parser.y

parser.o: parser.c parser.h
	g++ $(CFLAGS) -c $<

clean:
	rm -f *.o
	rm -f *.output
	rm -f lex.yy.c
	rm -f parser.h
	rm -f parser.c

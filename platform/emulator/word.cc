/*
 *  Author:
 *    Leif Kornstaedt <kornstae@ps.uni-sb.de>
 *
 *  Copyright:
 *    Leif Kornstaedt, 1999-2000
 *
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 *
 *  This file is part of Mozart, an implementation of Oz 3:
 *    http://www.mozart-oz.org
 *
 *  See the file "LICENSE" or
 *    http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 */

#include "base.hh"
#include "builtins.hh"

//
// Word Extension
//

#define MAXWIDTH 32
#define TRUNCATE(v, n) \
        (((unsigned int) (v << (MAXWIDTH - n))) >> (MAXWIDTH - n))

class MsgBuffer;
void marshalNumber(unsigned int i, MsgBuffer *bs);

#define ROBUST

#ifdef ROBUST
unsigned int unmarshalNumberRobust(MsgBuffer *, int *);
#else
unsigned int unmarshalNumber(MsgBuffer *);
#endif

class Word: public OZ_Extension {
public:
  unsigned int size;
  unsigned int value;

  Word(int s, int v) {
    Assert(s <= MAXWIDTH);
    size = s;
    value = TRUNCATE(v, s);
  }

  Word(const Word &w) {
    size = w.size;
    value = w.value;
  }

  int getIdV(void) {
    return OZ_E_WORD;
  }

  OZ_Term typeV(void) {
    return OZ_atom("word");
  }

  OZ_Return eqV(OZ_Term t) {
    if (OZ_isExtension(t)) {
      OZ_Extension *e = OZ_getExtension(t);
      if (e->getIdV() == OZ_E_WORD) {
        Word *w = (Word *) e;
        if (w->size == size && w->value == value) {
          return PROCEED;
        }
      }
    }
    return FAILED;
  }

  void printStreamV(ostream &out, int depth) {
    out << "<word" << size << " 0w" << value << ">";
  }

  OZ_Term printV(int depth) {
    char s[11];
    sprintf(s, "%u", value);
    return OZ_mkTupleC("#", 5, OZ_atom("<word"), OZ_int(size),
                       OZ_atom(" 0w"), OZ_atom(s), OZ_atom(">"));
  }

  OZ_Extension *gCollectV(void) {
    return new Word(*this);
  }

  OZ_Extension *sCloneV(void) {
    return new Word(*this);
  }

  void gCollectRecurseV(void) {}

  void sCloneRecurseV(void) {}

  OZ_Boolean marshalV(void *p) {
    MsgBuffer *bs = (MsgBuffer *) p;
    marshalNumber(size, bs);
    marshalNumber(value, bs);
    return OZ_TRUE;
  }
};

inline static bool OZ_isWord(OZ_Term t) {
  t = OZ_deref(t);
  return OZ_isExtension(t) && OZ_getExtension(t)->getIdV() == OZ_E_WORD;
}

inline static Word *OZ_WordToC(OZ_Term t) {
  return (Word *) OZ_getExtension(OZ_deref(t));
}

#define OZ_declareWord(ARG, VAR) \
        OZ_declareType(ARG, VAR, Word *, "word", \
                       OZ_isWord, OZ_WordToC)

#define OZ_word(size, value) OZ_extension(new Word(size, value))
#define OZ_RETURN_WORD(size, value) OZ_RETURN(OZ_word(size, value))

OZ_Term unmarshalWord(void *p) {
  MsgBuffer *bs = (MsgBuffer *) p;
#ifdef ROBUST
  int e;
  int size = unmarshalNumberRobust(bs, &e);
  int value = unmarshalNumberRobust(bs, &e);
#else
  int size = unmarshalNumber(bs);
  int value = unmarshalNumber(bs);
#endif
  return OZ_word(size, value);
}

void Word_init() {
  static Bool done = NO;
  if (done == NO) {
    done = OK;
    oz_registerExtension(OZ_E_WORD, unmarshalWord);
  }
}

//
// Builtins
//

OZ_BI_define(BIwordIs, 1, 1) {
  OZ_declareDetTerm(0, x);
  OZ_RETURN_BOOL(OZ_isWord(x));
} OZ_BI_end

OZ_BI_define(BIwordMake, 2, 1) {
  OZ_declareInt(0, size);
  if (size <= 0 || size > MAXWIDTH) {
    return OZ_raiseDebug(OZ_makeException(OZ_atom("system"), OZ_atom("kernel"),
                                          "Word.make", 1, OZ_int(size)));
  }
  OZ_declareInt(1, value);   //--** may truncate too much!
  OZ_RETURN_WORD(size, value);
} OZ_BI_end

OZ_BI_define(BIwordSize, 1, 1) {
  OZ_declareWord(0, w);
  OZ_RETURN_INT(w->size);
} OZ_BI_end

OZ_BI_define(BIwordToInt, 1, 1) {
  OZ_declareWord(0, w);
  OZ_RETURN(OZ_unsignedInt(w->value));
} OZ_BI_end

OZ_BI_define(BIwordToIntX, 1, 1) {
  OZ_declareWord(0, w);
  signed int v = w->value << (MAXWIDTH - w->size);
  OZ_RETURN_INT(v >> (MAXWIDTH - w->size));
} OZ_BI_end

OZ_BI_define(BIwordPlus, 2, 1) {
  OZ_declareWord(0, w1);
  OZ_declareWord(1, w2);
  if (w1->size != w2->size) {
    return OZ_raiseDebug(OZ_makeException(OZ_atom("system"), OZ_atom("kernel"),
                                          "Word.binop", 2,
                                          OZ_in(0), OZ_in(1)));
  }
  OZ_RETURN_WORD(w1->size, TRUNCATE(w1->value + w2->value, w1->size));
} OZ_BI_end

OZ_BI_define(BIwordMinus, 2, 1) {
  OZ_declareWord(0, w1);
  OZ_declareWord(1, w2);
  if (w1->size != w2->size) {
    return OZ_raiseDebug(OZ_makeException(OZ_atom("system"), OZ_atom("kernel"),
                                          "Word.binop", 2,
                                          OZ_in(0), OZ_in(1)));
  }
  OZ_RETURN_WORD(w1->size, TRUNCATE(w1->value - w2->value, w1->size));
} OZ_BI_end

OZ_BI_define(BIwordTimes, 2, 1) {
  OZ_declareWord(0, w1);
  OZ_declareWord(1, w2);
  if (w1->size != w2->size) {
    return OZ_raiseDebug(OZ_makeException(OZ_atom("system"), OZ_atom("kernel"),
                                          "Word.binop", 2,
                                          OZ_in(0), OZ_in(1)));
  }
  OZ_RETURN_WORD(w1->size, TRUNCATE(w1->value * w2->value, w1->size));
} OZ_BI_end

OZ_BI_define(BIwordMod, 2, 1) {
  OZ_declareWord(0, w1);
  OZ_declareWord(1, w2);
  if (w1->size != w2->size) {
    return OZ_raiseDebug(OZ_makeException(OZ_atom("system"), OZ_atom("kernel"),
                                          "Word.binop", 2,
                                          OZ_in(0), OZ_in(1)));
  }
  OZ_RETURN_WORD(w1->size, TRUNCATE(w1->value % w2->value, w1->size));
} OZ_BI_end

OZ_BI_define(BIwordOrb, 2, 1) {
  OZ_declareWord(0, w1);
  OZ_declareWord(1, w2);
  if (w1->size != w2->size) {
    return OZ_raiseDebug(OZ_makeException(OZ_atom("system"), OZ_atom("kernel"),
                                          "Word.binop", 2,
                                          OZ_in(0), OZ_in(1)));
  }
  OZ_RETURN_WORD(w1->size, w1->value | w2->value);
} OZ_BI_end

OZ_BI_define(BIwordXorb, 2, 1) {
  OZ_declareWord(0, w1);
  OZ_declareWord(1, w2);
  if (w1->size != w2->size) {
    return OZ_raiseDebug(OZ_makeException(OZ_atom("system"), OZ_atom("kernel"),
                                          "Word.binop", 2,
                                          OZ_in(0), OZ_in(1)));
  }
  OZ_RETURN_WORD(w1->size, w1->value ^ w2->value);
} OZ_BI_end

OZ_BI_define(BIwordAndb, 2, 1) {
  OZ_declareWord(0, w1);
  OZ_declareWord(1, w2);
  if (w1->size != w2->size) {
    return OZ_raiseDebug(OZ_makeException(OZ_atom("system"), OZ_atom("kernel"),
                                          "Word.binop", 2,
                                          OZ_in(0), OZ_in(1)));
  }
  OZ_RETURN_WORD(w1->size, w1->value & w2->value);
} OZ_BI_end

OZ_BI_define(BIwordNotb, 1, 1) {
  OZ_declareWord(0, w);
  OZ_RETURN_WORD(w->size, ~w->value);
} OZ_BI_end

OZ_BI_define(BIwordShl, 2, 1) {
  OZ_declareWord(0, w);
  OZ_declareWord(1, n);
  OZ_RETURN_WORD(w->size, w->value << n->value);
} OZ_BI_end

OZ_BI_define(BIwordLsr, 2, 1) {
  OZ_declareWord(0, w);
  OZ_declareWord(1, n);
  OZ_RETURN_WORD(w->size, w->value >> n->value);
} OZ_BI_end

OZ_BI_define(BIwordAsr, 2, 1) {
  OZ_declareWord(0, w);
  OZ_declareWord(1, n);
  signed int v = w->value << (MAXWIDTH - w->size);
  OZ_RETURN_WORD(w->size, v >> (MAXWIDTH - w->size + n->value));
} OZ_BI_end

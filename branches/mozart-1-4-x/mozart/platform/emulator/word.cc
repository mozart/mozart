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
#include "pickle.hh"

//
// Word Extension
//

#define MAXWIDTH 32
#define TRUNCATE(v, n) \
	(((unsigned int) (v << (MAXWIDTH - n))) >> (MAXWIDTH - n))

class MarshalerBuffer;
void marshalNumber(MarshalerBuffer *bs, unsigned int i);

unsigned int unmarshalNumber(MarshalerBuffer *);

class Word: public OZ_Extension {
private:
  Word(const Word &w): OZ_Extension() {
    size = w.size;
    value = w.value;
  }
public:
  unsigned int size;
  unsigned int value;

  Word(int s, int v) {
    Assert(s <= MAXWIDTH);
    size = s;
    value = TRUNCATE(v, s);
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

  OZ_Boolean toBePickledV() { return (OZ_TRUE); }
  void pickleV(MarshalerBuffer *bs, GenTraverser *gt) {
    marshalNumber(bs, size);
    marshalNumber(bs, value);
  }

  //
  virtual OZ_Boolean toBeMarshaledV() { return (NO); }
  virtual void marshalSuspV(OZ_Term te, ByteBuffer *bs, GenTraverser *gt) {
    Assert(0);
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

static
OZ_Term unmarshalWord(MarshalerBuffer *bs, Builder*)
{
  int size = unmarshalNumber(bs);
  int value = unmarshalNumber(bs);
  return (OZ_word(size, value));
}

static
OZ_Term suspUnmarshalWord(ByteBuffer *mb, Builder*,
			  GTAbstractEntity* &bae)
{
  Assert(0);
  return (UnmarshalEXT_Error);
}

static
OZ_Term unmarshalWordCont(ByteBuffer *mb, Builder*,
			  GTAbstractEntity* bae)
{
  Assert(0);
  return (UnmarshalEXT_Error);
}


void Word_init()
{
  static Bool done = NO;
  if (done == NO) {
    done = OK;
    oz_registerExtension(OZ_E_WORD, unmarshalWord,
			 suspUnmarshalWord, unmarshalWordCont);
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

OZ_BI_define(BIwordDiv, 2, 1) {
  OZ_declareWord(0, w1);
  OZ_declareWord(1, w2);
  if (w1->size != w2->size) {
    return OZ_raiseDebug(OZ_makeException(OZ_atom("system"), OZ_atom("kernel"),
					  "Word.binop", 2,
					  OZ_in(0), OZ_in(1)));
  } else if (w2->value == 0) {
    return oz_raise(E_ERROR, E_KERNEL, "div0", 1, OZ_in(0));
  }
  OZ_RETURN_WORD(w1->size, TRUNCATE(w1->value / w2->value, w1->size));
} OZ_BI_end

OZ_BI_define(BIwordMod, 2, 1) {
  OZ_declareWord(0, w1);
  OZ_declareWord(1, w2);
  if (w1->size != w2->size) {
    return OZ_raiseDebug(OZ_makeException(OZ_atom("system"), OZ_atom("kernel"),
					  "Word.binop", 2,
					  OZ_in(0), OZ_in(1)));
  } else if (w2->value == 0) {
    return oz_raise(E_ERROR, E_KERNEL, "mod0", 1, OZ_in(0));
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

// --added less, lessEq, greater, greaterEq

OZ_BI_define(BIwordLess, 2, 1) {
  OZ_declareWord(0, w1);
  OZ_declareWord(1, w2);
  if (w1->size != w2->size) {
    return OZ_raiseDebug(OZ_makeException(OZ_atom("system"), OZ_atom("kernel"),
					  "Word.binop", 2,
					  OZ_in(0), OZ_in(1)));
  }
  OZ_RETURN_BOOL(w1->value < w2->value);
} OZ_BI_end

OZ_BI_define(BIwordLessEq, 2, 1) {
  OZ_declareWord(0, w1);
  OZ_declareWord(1, w2);
  if (w1->size != w2->size) {
    return OZ_raiseDebug(OZ_makeException(OZ_atom("system"), OZ_atom("kernel"),
					  "Word.binop", 2,
					  OZ_in(0), OZ_in(1)));
  }
  OZ_RETURN_BOOL(w1->value <= w2->value);
} OZ_BI_end

OZ_BI_define(BIwordGreater, 2, 1) {
  OZ_declareWord(0, w1);
  OZ_declareWord(1, w2);
  if (w1->size != w2->size) {
    return OZ_raiseDebug(OZ_makeException(OZ_atom("system"), OZ_atom("kernel"),
					  "Word.binop", 2,
					  OZ_in(0), OZ_in(1)));
  }
  OZ_RETURN_BOOL(w1->value > w2->value);
} OZ_BI_end

OZ_BI_define(BIwordGreaterEq, 2, 1) {
  OZ_declareWord(0, w1);
  OZ_declareWord(1, w2);
  if (w1->size != w2->size) {
    return OZ_raiseDebug(OZ_makeException(OZ_atom("system"), OZ_atom("kernel"),
					  "Word.binop", 2,
					  OZ_in(0), OZ_in(1)));
  }
  OZ_RETURN_BOOL(w1->value >= w2->value);
} OZ_BI_end



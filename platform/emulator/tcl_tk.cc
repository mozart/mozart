/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: schulte
  Last modified: $Date$ from $Author$
  Version: $Revision$

*/

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "am.hh"

#include "builtins.hh"

#include "genvar.hh"
#include "ofgenvar.hh"
#include "fdbuilti.hh"
#include "solve.hh"


#define SAFETY_MARGIN   256
#define TCL_BUFFER_SIZE 1024

static char * tcl_buffer;
static char * tcl_buffer_start;
static char * tcl_buffer_end;
static char * protect_start;


inline
void init_tcl_buffer(void) {
  tcl_buffer_start = new char[TCL_BUFFER_SIZE + SAFETY_MARGIN];
  tcl_buffer_end   = tcl_buffer_start + TCL_BUFFER_SIZE;
  tcl_buffer       = tcl_buffer_start;
}

void resize_tcl_buffer(void) {
  int new_size = (3 * (tcl_buffer_end - tcl_buffer_start)) / 2;
  char *new_tcl_buffer_start =
    new char[new_size + SAFETY_MARGIN];

  tcl_buffer_end   = new_tcl_buffer_start + new_size;

  char *new_tcl_buffer = new_tcl_buffer_start;

  for (char *j=tcl_buffer_start; j<tcl_buffer ; new_tcl_buffer++, j++)
    *new_tcl_buffer = *j;

  delete tcl_buffer_start;

  tcl_buffer       = (tcl_buffer - tcl_buffer_start) + new_tcl_buffer_start;
  tcl_buffer_start = new_tcl_buffer_start;

}

inline
void delete_tcl_buffer(void) {
  delete tcl_buffer_start;
}



OZ_C_proc_begin(BIgetTclName,1) {
  return OZ_unify(OZ_getCArg(0), NameTclName);
} OZ_C_proc_end



State isVirtualString(TaggedRef vs) {
  DEREF(vs, vs_ptr, vs_tag);

  if (isAnyVar(vs_tag)) {
    return OZ_suspendOnVar(makeTaggedRef(vs_ptr));
  }

  if (isInt(vs_tag) || isFloat(vs_tag) ||
      (isLiteral(vs_tag) && tagged2Literal(vs)->isAtom()))
    return PROCEED;

  if (isSTuple(vs_tag) &&
      sameLiteral(tagged2STuple(vs)->getLabel(),AtomPair)) {
    for (int i=0; i < tagged2STuple(vs)->getSize(); i++) {
      State argstate = isVirtualString(tagged2STuple(vs)->getArg(i));
      if (argstate!=PROCEED)
        return argstate;
    }
    return PROCEED;
  }

  if (isLTuple(vs_tag)) {
    int len = isString(vs);

    if (len == -2) {
      return FAILED;
    } else if (len == -1) {
      return SUSPEND;
    } else {
      return PROCEED;
    }
  }

  return FAILED;

}


inline
State isTclLiteral(TaggedRef tcl) {
  if (tagged2Literal(tcl)->isAtom()) {
    return PROCEED;
  } else if (sameLiteral(tcl,NameTrue) || sameLiteral(tcl,NameFalse)) {
    return PROCEED;
  }
  return FAILED;
}

State isTcl(TaggedRef tcl) {
  DEREF(tcl, tcl_ptr, tcl_tag);

  if (isAnyVar(tcl_tag)) {
    return OZ_suspendOnVar(makeTaggedRef(tcl_ptr));
  } else if (isInt(tcl_tag)) {
    return PROCEED;
  } else if (isFloat(tcl_tag)) {
    return PROCEED;
  } else if (isLiteral(tcl_tag)) {
    return isTclLiteral(tcl);
  } else if (isProcedure(tcl)) {
    TaggedRef v = tagged2SRecord(
                    makeTaggedSRecord(
                      chunkCast(tcl)->getRecord()))->getFeature(NameTclName);

    if (v) {
      DEREF(v, v_ptr, v_tag);
      if (isAnyVar(v_tag)) {
        return OZ_suspendOnVar(makeTaggedRef(v_ptr));
      } else {
        return PROCEED;
      }
    } else {
      return FAILED;
    }
  } else if (isSRecord(tcl_tag)) {
    SRecord * sr = tagged2SRecord(tcl);

    if (tagged2Literal(sr->getLabel())->isAtom()) {
      for (int i=0; i < sr->getWidth(); i++) {
      State argstate = isTcl(sr->getArg(i));
      if (argstate!=PROCEED)
        return argstate;
      }
      return PROCEED;
    } else {
      return FAILED;
    }
  } else if (isSTuple(tcl_tag)) {
    STuple  * st = tagged2STuple(tcl);
    TaggedRef l  = st->getLabel();

    if (isAtom(l)) {
      if (sameLiteral(l,AtomPair)) {
        return isVirtualString(tcl);
      } else if (sameLiteral(l,AtomTclVS)) {
        TaggedRef arg = st->getArg(0);

        DEREF(arg, arg_ptr, arg_tag);

        if (isAnyVar(arg_tag)) {
          return OZ_suspendOnVar(makeTaggedRef(arg_ptr));
        }

        return isVirtualString(arg);
      } else if (sameLiteral(l,AtomTclBatch)) {
        TaggedRef batch = st->getArg(0);

        while (1) {
          DEREF(batch, batch_ptr, batch_tag);

          if (isAnyVar(batch_tag)) {
            return OZ_suspendOnVar(makeTaggedRef(batch_ptr));
          } else if (isLTuple(batch_tag)) {
            State batch_state = isTcl(tagged2LTuple(batch)->getHead());

            if (batch_state!=PROCEED)
              return batch_state;

            batch = tagged2LTuple(batch)->getTail();
          } else if (isLiteral(batch_tag) && sameLiteral(batch,AtomNil)) {
            return PROCEED;
          } else {
            return FAILED;
          }
        }

      } else {
        for (int i=0; i < st->getSize(); i++) {
          State argstate = isTcl(st->getArg(i));
          if (argstate!=PROCEED)
            return argstate;
        }
        return PROCEED;
      }
    }

  } else if (isLTuple(tcl_tag)) {
    int len = isString(tcl);

    if (len == -2) {
      return FAILED;
    } else if (len == -1) {
      return SUSPEND;
    } else {
      return PROCEED;
    }
  }

  return FAILED;

}


OZ_C_proc_begin(BIisTcl, 2) {
  State s = isTcl(OZ_getCArg(0));
  switch (s) {
  case FAILED:
    return OZ_unify(OZ_getCArg(1), NameFalse);
  case PROCEED:
    return OZ_unify(OZ_getCArg(1), NameTrue);
  default:
    return s;
  }
} OZ_C_proc_end




inline
void tcl_put(char c) {
  *tcl_buffer = c; tcl_buffer++;
  if (tcl_buffer>tcl_buffer_end)
    resize_tcl_buffer();
}

inline
void tcl_back(void) {
  tcl_buffer--;
}

inline
void tcl_put2(char c1, char c2) {
  *tcl_buffer = c1; tcl_buffer++;
  *tcl_buffer = c2; tcl_buffer++;
  if (tcl_buffer>tcl_buffer_end)
    resize_tcl_buffer();
}

inline
void tcl_put_octal(char c) {
  unsigned char c1 = (((unsigned char) c & '\300') >> 6) + '0';
  unsigned char c2 = (((unsigned char) c & '\070') >> 3) + '0';
  unsigned char c3 = ((unsigned char) c & '\007') + '0';
  *tcl_buffer = '\\'; tcl_buffer++;
  *tcl_buffer = c1;   tcl_buffer++;
  *tcl_buffer = c2;   tcl_buffer++;
  *tcl_buffer = c3;   tcl_buffer++;
  if (tcl_buffer>tcl_buffer_end)
    resize_tcl_buffer();
}

inline
void tcl_put_quote(char c) {
  unsigned char uc = (unsigned char) c;
  switch (uc) {
  case '\a': tcl_put2('\\', 'a'); break;
  case '\b': tcl_put2('\\', 'b'); break;
  case '\f': tcl_put2('\\', 'f'); break;
  case '\n': tcl_put2('\\', 'n'); break;
  case '\r': tcl_put2('\\', 'r'); break;
  case '\t': tcl_put2('\\', 't'); break;
  case '\v': tcl_put2('\\', 'v'); break;
  case '{':   case '}':   case '\\':  case '$':
  case '[':   case ']':   case '"':   case ';':
  case ' ':
    tcl_put2('\\', c); break;
  default:
    if ((uc<33) || (uc>127)) {
      tcl_put_octal(c);
    } else {
      tcl_put(c);
    }
  }
}


inline
void cstring2buffer(char* s) {
  char c;
  while ((c = *s++))
    tcl_put(c);
}


inline
void atom2buffer(TaggedRef atom) {
  cstring2buffer(tagged2Literal(atom)->getPrintName());
}


inline
void int2buffer(TaggedRef i) {
  char *s = OZ_intToCString(i);
  cstring2buffer(s);
  delete [] s;
}


inline
void float2buffer(TaggedRef f) {
  char *s = OZ_floatToCString(f);
  cstring2buffer(s);
  delete [] s;
}


inline
void string2buffer(TaggedRef tail) {
  do {
    tcl_put((char) smallIntValue(deref(tagged2LTuple(tail)->getHead())));
    tail = deref(tagged2LTuple(tail)->getTail());
  } while (isLTuple(tail));
}


inline
void vs2buffer(TaggedRef vs) {
  DEREF(vs, vs_ptr, vs_tag);

  if (isInt(vs_tag)) {
    int2buffer(vs);
  } else if (isFloat(vs_tag)) {
    float2buffer(vs);
  } else if (isLiteral(vs_tag)) {
    Assert(tagged2Literal(vs)->isAtom());

    if (!sameLiteral(vs, AtomNil) && !sameLiteral(vs, AtomPair))
      atom2buffer(vs);
  } else if (isSTuple(vs_tag)) {
    Assert(sameLiteral(tagged2STuple(vs)->getLabel(),AtomPair));

    for (int i=0; i < tagged2STuple(vs)->getSize(); i++)
      vs2buffer(tagged2STuple(vs)->getArg(i));
  } else if (isLTuple(vs_tag)) {
    string2buffer(vs);
  }
}


inline
void start_protect(void) {
  protect_start = tcl_buffer;
}


inline
void stop_protect(void) {
  if (protect_start == tcl_buffer)
    tcl_put2('"','"');
}


inline
void protect_atom2buffer(TaggedRef atom) {
  if (sameLiteral(atom, AtomPair) || sameLiteral(atom, AtomNil))
    return;

  char *s = tagged2Literal(atom)->getPrintName();
  char c;

  while ((c = *s++))
    tcl_put_quote(c);
}


inline
void protect_string2buffer(TaggedRef tail) {
  do {
    tcl_put_quote((char) smallIntValue(deref(tagged2LTuple(tail)->getHead())));
    tail = deref(tagged2LTuple(tail)->getTail());
  } while (isLTuple(tail));
}


inline
void protect_vs2buffer(TaggedRef vs) {
  DEREF(vs, vs_ptr, vs_tag);

  if (isInt(vs_tag)) {
    int2buffer(vs);
  } else if (isFloat(vs_tag)) {
    float2buffer(vs);
  } else if (isLiteral(vs_tag)) {
    Assert(tagged2Literal(vs)->isAtom());

    if (!sameLiteral(vs, AtomNil) && !sameLiteral(vs, AtomPair))
      protect_atom2buffer(vs);
  } else if (isSTuple(vs_tag)) {
    Assert(sameLiteral(tagged2STuple(vs)->getLabel(),AtomPair));

    for (int i=0; i < tagged2STuple(vs)->getSize(); i++)
      protect_vs2buffer(tagged2STuple(vs)->getArg(i));
  } else if (isLTuple(vs_tag)) {
    protect_string2buffer(vs);
  }
}


void tcl2buffer(TaggedRef);


inline
void tuple2buffer(STuple *st, int start = 0) {
  if (start < st->getSize()) {
    tcl2buffer(st->getArg(start));
    for (int i=start+1; i < st->getSize(); i++) {
      tcl_put(' ');
      tcl2buffer(st->getArg(i));
    }
  }
}


void tcl2buffer(TaggedRef tcl) {

  DEREF(tcl, tcl_ptr, tcl_tag);

  Assert(!isAnyVar(tcl_tag));

  if (isInt(tcl_tag)) {
    int2buffer(tcl);
  } else if (isFloat(tcl_tag)) {
    float2buffer(tcl);
  } else if (isLiteral(tcl_tag)) {
    if (tagged2Literal(tcl)->isAtom()) {
      start_protect();
      protect_atom2buffer(tcl);
      stop_protect();
    } else if (sameLiteral(tcl,NameTrue)) {
      tcl_put('1');
    } else if (sameLiteral(tcl,NameFalse)) {
      tcl_put('0');
    }
  } else if (isLTuple(tcl_tag)) {
    start_protect();
    protect_string2buffer(tcl);
    stop_protect();
  } else if (isSTuple(tcl_tag)) {
    STuple  * st = tagged2STuple(tcl);
    TaggedRef l  = st->getLabel();

    if (sameLiteral(l,AtomPair)) {
      start_protect();
      protect_vs2buffer(tcl);
      stop_protect();
    } else if (sameLiteral(l,AtomTclOption)) {
      tuple2buffer(st);
    } else if (sameLiteral(l,AtomTclList)) {
      tcl_put('['); tuple2buffer(st); tcl_put(']');
    } else if (sameLiteral(l,AtomTclQuote)) {
      tcl_put('{'); tuple2buffer(st); tcl_put('}');
    } else if (sameLiteral(l,AtomTclString)) {
      tcl_put('"'); tuple2buffer(st); tcl_put('"');
    } else if (sameLiteral(l,AtomTclPosition)) {
      if (st->getSize() > 1) {
        tcl_put('{'); tcl2buffer(st->getArg(0));
        tcl_put('.'); tuple2buffer(st, 1);
        tcl_put('}');
      }
    } else if (sameLiteral(l,AtomTclVS)) {
      vs2buffer(st->getArg(0));
    } else if (sameLiteral(l,AtomTclBatch)) {
      TaggedRef b = deref(st->getArg(0));

      if (isLTuple(b)) {
        tcl2buffer(tagged2LTuple(b)->getHead());
        b = deref(tagged2LTuple(b)->getTail());

        while (isLTuple(b)) {
          tcl_put(' ');
          tcl2buffer(tagged2LTuple(b)->getHead());
          b = deref(tagged2LTuple(b)->getTail());
        }
      }
    } else {
      start_protect();
      protect_atom2buffer(st->getLabel());
      stop_protect();
      tcl_put(' '); tuple2buffer(st);
    }

  } else if (isProcedure(tcl)) {
    vs2buffer(tagged2SRecord(
                makeTaggedSRecord(
                  chunkCast(tcl)->getRecord()))->getFeature(NameTclName));
  } else if (isSRecord(tcl_tag)) {
    SRecord * sr = tagged2SRecord(tcl);
    TaggedRef l  = sr->getLabel();

    if (!sameLiteral(l,AtomTclOption)) {
      start_protect();
      protect_atom2buffer(l);
      stop_protect();
      tcl_put(' ');
    }

    TaggedRef as = deref(sr->getArityList());

    while (isLTuple(as)) {
      TaggedRef a = deref(tagged2LTuple(as)->getHead());

      if (tagged2Literal(a)->isAtom()) {
        tcl_put('-');
        start_protect();
        protect_atom2buffer(a);
        stop_protect();
        tcl_put(' ');
        tcl2buffer(sr->getFeature(a));
        tcl_put(' ');
      }

      as = deref(tagged2LTuple(as)->getTail());
    }

    tcl_back();
  }

}


State ret_unix_error(TaggedRef out) {
  STuple * err_tuple = STuple::newSTuple(AtomError, 2);

  err_tuple->setArg(0, OZ_CToInt(errno));
  err_tuple->setArg(1, OZ_CToString(OZ_unixError(errno)));

  return OZ_unify(out,makeTaggedSTuple(err_tuple));
}


State tcl_write(int fd, char * buff, int len, TaggedRef out) {
  int ret = osTestSelect(fd,SEL_WRITE);

  if (ret < 0)  {
    delete_tcl_buffer();
    return ret_unix_error(out);
  } else if (ret==0) {
    return OZ_unifyInt(out,0);
  }

  while ((ret = write(fd, buff, len)) < 0) {
    if (errno != EINTR) {
      delete_tcl_buffer();
      return ret_unix_error(out);
    }
  }

  if (len == ret) {
    delete_tcl_buffer();

    return OZ_unify(out, NameTrue);
  }

  return OZ_unifyInt(out, ret);
}


OZ_C_proc_begin(BItclWrite,3) {
  OZ_declareIntArg("tclWrite", 0, fd);

  init_tcl_buffer();
  tcl2buffer(OZ_getCArg(1));
  tcl_put('\n');

  return tcl_write(fd, tcl_buffer_start, tcl_buffer-tcl_buffer_start,
                   OZ_getCArg(2));
}
OZ_C_proc_end


OZ_C_proc_begin(BItclWriteTuple,4) {
  OZ_declareIntArg("tclWriteTuple", 0, fd);

  init_tcl_buffer();
  tcl2buffer(OZ_getCArg(1));
  tcl_put(' ');
  tuple2buffer(tagged2STuple(deref(OZ_getCArg(2))));
  tcl_put('\n');

  return tcl_write(fd, tcl_buffer_start, tcl_buffer-tcl_buffer_start,
                   OZ_getCArg(3));
}
OZ_C_proc_end


OZ_C_proc_begin(BItclWriteTagTuple,5) {
  OZ_declareIntArg("tclWriteTagTuple", 0, fd);
  TaggedRef tuple = deref(OZ_getCArg(3));

  init_tcl_buffer();
  tcl2buffer(OZ_getCArg(1));
  tcl_put(' ');
  tcl2buffer(tagged2STuple(tuple)->getArg(0));
  tcl_put(' ');
  tcl2buffer(OZ_getCArg(2));
  tcl_put(' ');
  tuple2buffer(tagged2STuple(tuple),1);
  tcl_put('\n');

  return tcl_write(fd, tcl_buffer_start, tcl_buffer-tcl_buffer_start,
                   OZ_getCArg(4));
}
OZ_C_proc_end


OZ_C_proc_begin(BItclWriteCont,3) {
  OZ_declareIntArg("tclWriteCont", 0, fd);
  OZ_declareIntArg("tclWriteCont", 1, written);

  return tcl_write(fd, tcl_buffer_start + written,
                   tcl_buffer-tcl_buffer_start-written,
                   OZ_getCArg(2));
}
OZ_C_proc_end


// ---------------------------------------------------------------------
// Add to Builtin-Table
// ---------------------------------------------------------------------

static
BIspec tclTkSpec[] = {
  {"getTclName",       1, BIgetTclName,       NO, 0},
  {"isTcl",            2, BIisTcl,            NO, 0},
  {"tclWrite",         3, BItclWrite,         NO, 0},
  {"tclWriteTuple",    4, BItclWriteTuple,    NO, 0},
  {"tclWriteTagTuple", 5, BItclWriteTagTuple, NO, 0},
  {"tclWriteCont",     3, BItclWriteCont,     NO, 0},
  {0,0,0,0,0}
};


void BIinitTclTk() {
  BIaddSpec(tclTkSpec);
}

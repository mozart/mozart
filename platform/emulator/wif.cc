/*
 *  Authors:
 *    Christian Schulte (schulte@dfki.de)
 *
 *  Copyright:
 *    Christian Schulte, 1997, 1998
 *
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 *
 *  This file is part of Mozart, an implementation
 *  of Oz 3:
 *     $MOZARTURL$
 *
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#ifdef HAVE_CONFIG_H
#include "conf.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "runtime.hh"
#include "gc.hh"
#include "builtins.hh"
#include "dictionary.hh"
#include "find-alive-entry.hh"


TaggedRef
  WifAtomTclOption, WifAtomTclList, WifAtomTclPosition,
  WifAtomTclQuote, WifAtomTclString, WifAtomTclVS,
  WifAtomTclBatch, WifAtomTclColor, WifAtomTclDelete,
  WifAtomDot,
  WifAtomTagPrefix, WifAtomVarPrefix, WifAtomImagePrefix,

  WifNameTclName,
  WifNameTclClosed,
  WifNameTclSlaves,
  WifNameTclSlaveEntry;

void wifInitLiterals() {

  WifAtomTclOption    = OZ_atom("o");
  WifAtomTclDelete    = OZ_atom("d");
  WifAtomTclList      = OZ_atom("l");
  WifAtomTclPosition  = OZ_atom("p");
  WifAtomTclQuote     = OZ_atom("q");
  WifAtomTclString    = OZ_atom("s");
  WifAtomTclVS        = OZ_atom("v");
  WifAtomTclBatch     = OZ_atom("b");
  WifAtomTclColor     = OZ_atom("c");
  WifAtomDot          = OZ_atom(".");
  WifAtomTagPrefix    = OZ_atom("t");
  WifAtomVarPrefix    = OZ_atom("v");
  WifAtomImagePrefix  = OZ_atom("i");

  WifNameTclName       = OZ_newName(); OZ_protect(&WifNameTclName);
  WifNameTclSlaves     = OZ_newName(); OZ_protect(&WifNameTclSlaves);
  WifNameTclSlaveEntry = OZ_newName(); OZ_protect(&WifNameTclSlaveEntry);
  WifNameTclClosed     = OZ_newName(); OZ_protect(&WifNameTclClosed);

}

/*
 * Exceptions
 */

static OZ_Return raise_os_error() {
  int xx = errno;
  return oz_raise(E_SYSTEM,E_OS,"os",2,OZ_int(xx),
                  OZ_string(OZ_unixError(xx)));
}

static OZ_Return raise_type_error(TaggedRef tcl) {
  oz_typeError(-1,"Tickle");
}

static OZ_Return raise_closed(TaggedRef tcl) {
  return oz_raise(E_SYSTEM,E_TK,"alreadyClosed",1,tcl);
}

static OZ_Return raise_toplevel(void) {
  return oz_raise(E_ERROR,E_KERNEL,"globalState",1,OZ_atom("io"));
}



/*
 * Check for toplevel when execting
 */

#define CHECK_TOPLEVEL \
{ if (!am.onToplevel()) return raise_toplevel(); }



/*
 * Locking
 */

#define ENTER_WIF_LOCK { \
  TaggedRef t = wif.getLock();     \
  DEREF(t, t_ptr, t_tag);          \
  if (isVariableTag(t_tag)) {      \
    am.addSuspendVarList(t_ptr);   \
    return SUSPEND;                \
  } else {                         \
    wif.setLock(oz_newVariable()); \
  }                                \
}

#define LEAVE_WIF_LOCK \
  (void) oz_unify(wif.getLock(), NameUnit);



/*
 * Dynamically expanded string buffer
 */

#define StateExit(Check) \
  { s = Check; if (s != PROCEED) goto exit; }
#define StateReturn(Check) \
  { OZ_Return s = Check; if (s != PROCEED) return s;  }



inline
char hex_digit(unsigned int i) {
  return (i>9) ? (i - 10 + 'a') : (i + '0');
}

#define SAFETY_MARGIN      256
#define STRING_BUFFER_SIZE 2048

static char static_buffer[STRING_BUFFER_SIZE+SAFETY_MARGIN];

class WIF {
  char * buffer;
  char * start;
  char * write_start;
  char * end;
  char * protect_start;

  int wif_fd;

  TaggedRef wif_lock;
  TaggedRef wif_rets;
  TaggedRef wif_dict;

  int widget_ctr;
  int tag_ctr;
  int var_ctr;
  int image_ctr;

  void ensure(int n) {
    while (buffer+n>end)
      resize();
  }

public:
  TaggedRef getLock() {
    return wif_lock;
  }

  void setLock(TaggedRef t) {
    wif_lock = t;
  }

  void enterReturn(TaggedRef ret, TaggedRef cast) {
    TaggedRef newt = OZ_cons(OZ_cons(ret,cast),
                             oz_newVariable());

    (void) oz_unify(newt,wif_rets); // mm_u
    wif_rets = tail(newt);
  }

  TaggedRef genTopName() {
    SRecord * s = SRecord::newSRecord(AtomPair,2);
    s->setArg(0,WifAtomDot);
    s->setArg(1,makeInt(widget_ctr++));
    return makeTaggedSRecord(s);
  }

  TaggedRef genWidgetName(TaggedRef parent) {
    SRecord * s = SRecord::newSRecord(AtomPair,3);
    s->setArg(0,parent);
    s->setArg(1,WifAtomDot);
    s->setArg(2,makeInt(widget_ctr++));
    return makeTaggedSRecord(s);
  }

  TaggedRef genTagName() {
    SRecord * s = SRecord::newSRecord(AtomPair,2);
    s->setArg(0,WifAtomTagPrefix);
    s->setArg(1,makeInt(tag_ctr++));
    return makeTaggedSRecord(s);
  }

  TaggedRef genVarName() {
    SRecord * s = SRecord::newSRecord(AtomPair,2);
    s->setArg(0,WifAtomVarPrefix);
    s->setArg(1,makeInt(var_ctr++));
    return makeTaggedSRecord(s);
  }

  TaggedRef genImageName() {
    SRecord * s = SRecord::newSRecord(AtomPair,2);
    s->setArg(0,WifAtomImagePrefix);
    s->setArg(1,makeInt(image_ctr++));
    return makeTaggedSRecord(s);
  }


  WIF() {
    widget_ctr = 0;
    tag_ctr    = 0;
    var_ctr    = 0;
    image_ctr  = 0;

    start         = static_buffer;
    end           = start + STRING_BUFFER_SIZE;
    buffer        = start;
  }

  void init(int fd, TaggedRef d, TaggedRef r) {

    wifInitLiterals();

    wif_fd        = fd;
    wif_lock      = NameUnit;
    wif_rets      = r;
    wif_dict      = d;

    (void) gcProtect(&wif_lock);
    (void) gcProtect(&wif_rets);
    (void) gcProtect(&wif_dict);
  }

  ~WIF() {
    dispose();
  }

  void reset(void) {
    dispose();
    start  = static_buffer;
    end    = start + STRING_BUFFER_SIZE;
    buffer = start;
  }

  void start_write(void) {
    write_start = start;
  }

  OZ_Return write(void);

  void resize(void);

  void dispose(void) {
    if (start!=static_buffer)
      delete start;
  }

  void put(char c) {
    *buffer++ = c;
    ensure(0);
  }

  void put2(char c1,char c2) {
    *buffer++ = c1;
    *buffer++ = c2;
    ensure(0);
  }

  void start_protect(void) {
    protect_start = buffer;
  }

  void stop_protect(void) {
    if (protect_start == buffer)
      put2('"','"');
  }

  void put_quote(char c) {
    unsigned char uc = (unsigned char) c;
    switch (uc) {
    case '{':   case '}':   case '\\':  case '$':
    case '[':   case ']':   case '"':   case ';':
    case ' ':
      *buffer++ = '\\';
      *buffer++ = c;
      break;
    default:
      if ((uc<33) || (uc>127)) {
        unsigned char c1 = ((uc & '\300') >> 6) + '0';
        unsigned char c2 = ((uc & '\070') >> 3) + '0';
        unsigned char c3 = (uc & '\007') + '0';
        *buffer++ = '\\';
        *buffer++ = c1;
        *buffer++ = c2;
        *buffer++ = c3;
      } else {
        *buffer++ = c;
      }
    }
  }

  /* Tcl Methods */

  void put_int(TaggedRef i) {
    if (oz_isSmallInt(i)) {
      int len;
      sprintf(buffer,"%d%n",smallIntValue(i),&len);
      buffer += len;
      ensure(0);
    } else {
      char * s = toC(i);
      if (*s == '~') *s='-';
      ensure(strlen(s));
      char c;
      while ((c = *s++))
        *buffer++=c;
    }
  }

  void put_float(TaggedRef f) {
    int len;
    sprintf(buffer,"%g%n",floatValue(f),&len);
    buffer += len;
    ensure(0);
  }

  void put_atom(TaggedRef atom) {
    if (literalEq(atom, AtomPair) || literalEq(atom, AtomNil))
      return;
    Assert(oz_isAtom(atom));
    Atom *l = (Atom*) tagged2Literal(atom);
    int n = l->getSize();
    const char *s = l->getPrintName();

    ensure(n);
    for (int i = 0; i < n; i++) {
      *buffer++ = *s++;
    }
  }

  void put_atom_quote(TaggedRef atom) {
    if (literalEq(atom, AtomPair) || literalEq(atom, AtomNil))
      return;

    Assert(oz_isAtom(atom));
    Atom* l = (Atom*)tagged2Literal(atom);
    int n = l->getSize();
    const char *s = l->getPrintName();
    char c;
    ensure(4*n);

    while ((c = *s++))
      put_quote(c);

  }

  OZ_Return put_string_quote(TaggedRef list) {
    while (1) {
      TaggedRef h = head(list);
      DEREF(h, h_ptr, h_tag);

      if (isVariableTag(h_tag)) {
        am.addSuspendVarList(h_ptr);
        return SUSPEND;
      }
      if (!isSmallIntTag(h_tag))
        return raise_type_error(list);
      int i = smallIntValue(h);
      if (i<0 || i>255)
        return raise_type_error(list);

      put_quote((char) i);
      ensure(0);

      TaggedRef t = tail(list);
      DEREF(t, t_ptr, t_tag);

      if (isVariableTag(t_tag)) {
        am.addSuspendVarList(t_ptr);
        return SUSPEND;
      }

      if (isLTupleTag(t_tag)) {
        list = t;
        continue;
      }

      if (isLiteralTag(t_tag) && oz_isNil(t))
        return PROCEED;

      return raise_type_error(list);
    }
  }

  OZ_Return put_string(TaggedRef list) {
    while (1) {
      TaggedRef h = head(list);
      DEREF(h, h_ptr, h_tag);

      if (isVariableTag(h_tag)) {
        am.addSuspendVarList(h_ptr);
        return SUSPEND;
      }
      if (!isSmallIntTag(h_tag))
        return raise_type_error(list);
      int i = smallIntValue(h);
      if (i<0 || i>255)
        return raise_type_error(list);

      put((char) i);

      TaggedRef t = tail(list);
      DEREF(t, t_ptr, t_tag);

      if (isVariableTag(t_tag)) {
        am.addSuspendVarList(t_ptr);
        return SUSPEND;
      }

      if (isLTupleTag(t_tag)) {
        list = t;
        continue;
      }

      if (isLiteralTag(t_tag) && oz_isNil(t))
        return PROCEED;

      return raise_type_error(list);
    }
  }

  OZ_Return put_feature(SRecord * sr, TaggedRef a) {
    if (oz_isSmallInt(a)) {
      return put_tcl(sr->getFeature(a));
    } if (oz_isAtom(a)) {
      put('-');
      put_atom(a);
      put(' ');
      return put_tcl(sr->getFeature(a));
    } else {
      return raise_type_error(makeTaggedSRecord(sr));
    }
  }

  OZ_Return put_tcl(TaggedRef tcl);
  OZ_Return put_tcl_filter(TaggedRef tcl, TaggedRef fs);
  OZ_Return put_tcl_return(TaggedRef tcl, TaggedRef * ret);
  OZ_Return put_vs(TaggedRef vs);
  OZ_Return put_vs_quote(TaggedRef vs);
  OZ_Return put_batch(TaggedRef batch, char delim);
  OZ_Return put_tuple(SRecord *st, int start = 0);
  OZ_Return put_record(SRecord * sr, TaggedRef as);
  OZ_Return put_record_or_tuple(TaggedRef tcl, int start);

  OZ_Return close_hierarchy(Object * o);

};


static WIF wif;


OZ_Return WIF::write() {
redo:
  int ret = osTestSelect(wif_fd, SEL_WRITE);

  if (ret < 0)  {
    reset();
    LEAVE_WIF_LOCK;
    return raise_os_error();
  } else if (ret==0) {
    goto wait_select;
  }

  while ((ret = oswrite(wif_fd, write_start, buffer-write_start)) < 0) {
    if (errno != EINTR) {
      reset();
      LEAVE_WIF_LOCK;
      return raise_os_error();
    }
  }

  if (buffer - write_start == ret) {
    reset();
    LEAVE_WIF_LOCK;
    return PROCEED;
  }

  write_start += ret;
wait_select:
  TaggedRef var = oz_newVariable();

  (void) am.select(wif_fd, SEL_WRITE, NameUnit, var);
  DEREF(var, var_ptr, var_tag);
  if (isVariableTag(var_tag)) {
    am.addSuspendVarList(var_ptr);
    return SUSPEND;
  } else {
    goto redo;
  }
}


void WIF::resize(void) {
  int new_size = (3 * (end - start)) / 2;
  char *new_start = new char[new_size + SAFETY_MARGIN];

  end   = new_start + new_size;

  memcpy(new_start, start, buffer-start);

  dispose();

  buffer = (buffer - start) + new_start;
  start  = new_start;
}



OZ_Return WIF::put_tuple(SRecord *st, int start) {
  if (start < st->getWidth()) {
    StateReturn(put_tcl(st->getArg(start)));

    for (int i=start+1; i < st->getWidth(); i++) {
      put(' ');
      StateReturn(put_tcl(st->getArg(i)));
    }
  }
  return PROCEED;
}

OZ_Return WIF::put_record(SRecord * sr, TaggedRef as) {
  TaggedRef a = head(as);

  StateReturn(put_feature(sr,a));
  as = tail(as);

  while (oz_isCons(as)) {
    a = head(as);
    put(' ');
    StateReturn(put_feature(sr,a));
    as = tail(as);
  }
  return PROCEED;
}

OZ_Return WIF::put_batch(TaggedRef batch, char delim) {

  DEREF(batch, batch_ptr, batch_tag);

  if (isVariableTag(batch_tag)) {
    am.addSuspendVarList(batch_ptr);
    return SUSPEND;
  } else if (isLTupleTag(batch_tag)) {
    OZ_Return batch_state = put_tcl(head(batch));

    if (batch_state!=PROCEED)
      return batch_state;

    batch = tail(batch);
  } else if (isLiteralTag(batch_tag) && literalEq(batch,AtomNil)) {
    return PROCEED;
  } else {
    return raise_type_error(batch);
  }

  while (1) {
    DEREF(batch, batch_ptr, batch_tag);

    if (isVariableTag(batch_tag)) {
      am.addSuspendVarList(batch_ptr);
      return SUSPEND;
    } else if (isLTupleTag(batch_tag)) {
      put(delim);
      OZ_Return batch_state = put_tcl(head(batch));

      if (batch_state!=PROCEED)
        return batch_state;

      batch = tail(batch);
    } else if (isLiteralTag(batch_tag) && literalEq(batch,AtomNil)) {
      return PROCEED;
    } else {
      return raise_type_error(batch);
    }
  }
}

OZ_Return WIF::put_record_or_tuple(TaggedRef tcl, int start = 0) {
  SRecord * st = tagged2SRecord(oz_deref(tcl));

  if (st->isTuple()) {
    if (start < st->getWidth()) {
      StateReturn(put_tcl(st->getArg(start)));

      for (int i=start+1; i < st->getWidth(); i++) {
        put(' ');
        StateReturn(put_tcl(st->getArg(i)));
      }
    }
    return PROCEED;
  } else {
    TaggedRef as = st->getArityList();

    if (start==1 && oz_isCons(as)) {
      Assert(smallIntValue(head(as))==1);
      as=tail(as);
    }
    if (!oz_isCons(as))
      return PROCEED;

    StateReturn(put_feature(st,head(as)));

    as = tail(as);

    while (oz_isCons(as)) {
      TaggedRef a = head(as);
      put(' ');
      StateReturn(put_feature(st,a));
      as = tail(as);
    }
    return PROCEED;
  }
}

OZ_Return WIF::put_vs(TaggedRef vs) {
  DEREF(vs, vs_ptr, vs_tag);

  if (isVariableTag(vs_tag)) {
    am.addSuspendVarList(vs_ptr);
    return SUSPEND;
  } else if (isSmallIntTag(vs_tag) || oz_isBigInt(vs)) {
    put_int(vs);
    return PROCEED;
  } else if (isFloatTag(vs_tag)) {
    put_float(vs);
    return PROCEED;
  } else if (isLiteralTag(vs_tag)) {

    if (!tagged2Literal(vs)->isAtom())
      return raise_type_error(vs);

    put_atom(vs);
    return PROCEED;
  } else if (oz_isSTuple(vs)) {
    SRecord * sr = tagged2SRecord(vs);

    if (!literalEq(sr->getLabel(),AtomPair))
       return raise_type_error(vs);

    for (int i=0; i < sr->getWidth(); i++) {
      StateReturn(put_vs(sr->getArg(i)));
    }
    return PROCEED;
  } else if (isLTupleTag(vs_tag)) {
    return put_string(vs);
  } else {
    return raise_type_error(vs);
  }
}


OZ_Return WIF::put_vs_quote(TaggedRef vs) {
  DEREF(vs, vs_ptr, vs_tag);

  if (isVariableTag(vs_tag)) {
    am.addSuspendVarList(vs_ptr);
    return SUSPEND;
  } else if (isSmallIntTag(vs_tag) || oz_isBigInt(vs)) {
    put_int(vs);
    return PROCEED;
  } else if (isFloatTag(vs_tag)) {
    put_float(vs);
    return PROCEED;
  } else if (isLiteralTag(vs_tag)) {

    if (!tagged2Literal(vs)->isAtom())
      return raise_type_error(vs);

    put_atom_quote(vs);
    return PROCEED;
  } else if (oz_isSTuple(vs)) {
    SRecord * sr = tagged2SRecord(vs);

    if (!literalEq(sr->getLabel(),AtomPair))
      return raise_type_error(vs);

    for (int i=0; i < sr->getWidth(); i++) {
      StateReturn(put_vs_quote(sr->getArg(i)));
    }
    return PROCEED;
  } else if (isLTupleTag(vs_tag)) {
    return put_string_quote(vs);
  } else {
    return raise_type_error(vs);
  }
}


OZ_Return WIF::put_tcl(TaggedRef tcl) {
  DEREF(tcl, tcl_ptr, tcl_tag);

  if (isVariableTag(tcl_tag)) {
    am.addSuspendVarList(tcl_ptr);
    return SUSPEND;
  } else if (isSmallIntTag(tcl_tag) || oz_isBigInt(tcl)) {
    put_int(tcl);
    return PROCEED;
  } else if (isFloatTag(tcl_tag)) {
    put_float(tcl);
    return PROCEED;
  } else if (isLiteralTag(tcl_tag)) {
    if (tagged2Literal(tcl)->isAtom()) {
      start_protect();
      put_atom_quote(tcl);
      stop_protect();
      return PROCEED;
    } else if (literalEq(tcl,NameTrue)) {
      put('1');
      return PROCEED;
    } else if (literalEq(tcl,NameFalse)) {
      put('0');
      return PROCEED;
    } else if (literalEq(tcl,NameUnit)) {
      return PROCEED;
    } else {
      return raise_type_error(tcl);
    }
  } else if (oz_isObject(tcl)) {
    TaggedRef v = tagged2Object(tcl)->getFeature(WifNameTclName);

    if (v!=makeTaggedNULL()) {
      DEREF(v, v_ptr, v_tag);

      if (isVariableTag(v_tag)) {
        am.addSuspendVarList(v_ptr);
        return SUSPEND;
      } else if (isLiteralTag(v_tag) && literalEq(v,WifNameTclClosed)) {
        return raise_closed(tcl);
      } else {
        return put_vs(v);
      }
    } else {
      return raise_type_error(tcl);
    }

  } else if (oz_isSTuple(tcl)) {
    SRecord  * st = tagged2SRecord(tcl);
    TaggedRef l   = st->getLabel();

    if (oz_isAtom(l)) {
      if (literalEq(l,AtomCons)) {
        return raise_type_error(tcl);
      } else if (literalEq(l,AtomPair)) {
        start_protect();
        StateReturn(put_vs_quote(tcl));
        stop_protect();
        return PROCEED;
      } else if (literalEq(l,WifAtomTclVS)) {
        TaggedRef arg = st->getArg(0);

        if (st->getWidth() != 1)
          return raise_type_error(tcl);

        DEREF(arg, arg_ptr, arg_tag);

        if (isVariableTag(arg_tag)) {
          am.addSuspendVarList(arg_ptr);
          return SUSPEND;
        }

        return put_vs(arg);

      } else if (literalEq(l,WifAtomTclBatch)) {
        if (st->getWidth() != 1)
          return raise_type_error(tcl);

        return put_batch(st->getArg(0), ' ');
      } else if (literalEq(l,WifAtomTclColor)) {
        if (st->getWidth() != 3)
          return raise_type_error(tcl);

        put('#');
        for (int i=0; i < 3; i++) {
          TaggedRef arg = st->getArg(i);

          DEREF(arg, arg_ptr, arg_tag);

          if (oz_isVariable(arg)) {
            am.addSuspendVarList(arg_ptr);
            return SUSPEND;
          }

          if (!isSmallIntTag(arg_tag))
            return raise_type_error(tcl);

          int i = smallIntValue(arg);

          if ((i < 0) || (i > 255))
            return raise_type_error(tcl);

          unsigned char c1 = hex_digit(((unsigned char) i & '\xF0') >> 4);
          unsigned char c2 = hex_digit((unsigned char) i & '\x0F');
          put2(c1,c2);
        }
        return PROCEED;
      } else if (literalEq(l,WifAtomTclOption)) {
        return put_tuple(st);
      } else if (literalEq(l,WifAtomTclDelete)) {
        if (st->getWidth() != 1)
          return raise_type_error(tcl);
        TaggedRef rt = st->getArg(0);
        DEREF(rt, rt_ptr, rt_tag);
        if (oz_isVariable(rt)) {
          am.addSuspendVarList(rt_ptr);
          return SUSPEND;
        }
        return put_record_or_tuple(rt);
      } else if (literalEq(l,WifAtomTclList)) {
        put('[');
        StateReturn(put_tuple(st));
        put(']');
        return PROCEED;
      } else if (literalEq(l,WifAtomTclQuote)) {
        put('{');
        StateReturn(put_tuple(st));
        put('}');
        return PROCEED;
      } else if (literalEq(l,WifAtomTclString)) {
        put('"');
        StateReturn(put_tuple(st));
        put('"');
        return PROCEED;
      } else if (literalEq(l,WifAtomTclPosition)) {
        put('{');
        StateReturn(put_tcl(st->getArg(0)));
        put('.');
        StateReturn(put_tuple(st, 1));
        put('}');
        return PROCEED;
      } else {
        put_atom(st->getLabel());
        put(' ');
        return put_tuple(st);
      }
    } else {
      return raise_type_error(tcl);
    }
  } else if (isSRecordTag(tcl_tag)) {
    SRecord * sr = tagged2SRecord(tcl);
    TaggedRef l  = sr->getLabel();
    TaggedRef as = sr->getArityList(); /* arity list is already deref'ed */

    if (tagged2Literal(l)->isAtom()) {

      if (literalEq(l,AtomPair) || literalEq(l,AtomCons) ||
          literalEq(l,WifAtomTclVS) || literalEq(l,WifAtomTclBatch) ||
          literalEq(l,WifAtomTclColor)) {
        return raise_type_error(tcl);
      } else if (literalEq(l,WifAtomTclOption)) {
        return put_record(sr, as);
      } else if (literalEq(l,WifAtomTclList)) {
        put('[');
        StateReturn(put_record(sr, as));
        put(']');
        return PROCEED;
      } else if (literalEq(l,WifAtomTclQuote)) {
        put('{');
        StateReturn(put_record(sr, as));
        put('}');
        return PROCEED;
      } else if (literalEq(l,WifAtomTclString)) {
        put('"');
        StateReturn(put_record(sr, as));
        put('"');
        return PROCEED;
      } else if (literalEq(l,WifAtomTclPosition)) {
        put('{');
        StateReturn(put_feature(sr, head(as)));
        put('.');
        if (sr->getWidth() > 1)
          StateReturn(put_record(sr, tail(as)));
        put('}');
        return PROCEED;
      } else {
        start_protect();
        put_atom(l);
        stop_protect();
        put(' ');
        return put_record(sr, as);
      }
    } else {
      return raise_type_error(tcl);
    }

  } else if (isLTupleTag(tcl_tag)) {
    start_protect();
    StateReturn(put_string_quote(tcl));
    stop_protect();
    return PROCEED;
  }

  return raise_type_error(tcl);

}


OZ_Return WIF::put_tcl_filter(TaggedRef tcl, TaggedRef fs) {
  DEREF(tcl, tcl_ptr, tcl_tag);

  if (isLiteralTag(tcl_tag)) {
    return PROCEED;
  } else if (isSRecordTag(tcl_tag)) {
    OZ_Return s = PROCEED;
    SRecord * sr = tagged2SRecord(tcl);
    TaggedRef as = sr->getArityList(); /* arity list is already deref'ed */
    fs = oz_deref(fs);

    while (oz_isCons(as) && oz_isCons(fs)) {
      TaggedRef a = head(as);
      TaggedRef f = oz_deref(head(fs));

      if (oz_isName(a))
        return raise_type_error(tcl);

      switch (featureCmp(a,f)) {
      case 0:
        fs = oz_deref(tail(fs));
        as = tail(as);
        break;
      case 1:
        fs = oz_deref(tail(fs));
        break;
      case -1:
        StateReturn(put_feature(sr,a));
        put(' ');
        as = tail(as);
        break;
      }

    }

    if (oz_isCons(as)) {
      return put_record(sr,as);
    } else {
      return PROCEED;
    }
  } else {
    return raise_type_error(tcl);
  }
}


OZ_Return WIF::put_tcl_return(TaggedRef tcl, TaggedRef * ret) {
  *ret = makeTaggedNULL();
  DEREF(tcl, tcl_ptr, tcl_tag);

  if (oz_isSTuple(tcl)) {
    SRecord * sr = tagged2SRecord(tcl);
    int w = sr->getWidth();

    if (w == 1)
      return raise_type_error(tcl);

    for (int i=1; i < w-1; i++) {
      put(' ');
      StateReturn(put_tcl(sr->getArg(i)));
    }

    *ret = sr->getArg(w-1);
    return PROCEED;

  } else if (isSRecordTag(tcl_tag)) {
    SRecord * sr = tagged2SRecord(tcl);
    TaggedRef as = tail(sr->getArityList()); /* arity list is already deref'ed */

    while (oz_isCons(as)) {
      TaggedRef a1  = head(as);
      TaggedRef ar  = tail(as);

      if (oz_isSmallInt(a1)) {
        if (oz_isCons(ar)) {
          TaggedRef a2 = head(ar);

          if (oz_isSmallInt(a2)) {
            put(' ');
            StateReturn(put_tcl(sr->getFeature(a1)));
          } else {
            *ret = sr->getFeature(a1);
          }

        } else {
          *ret = sr->getFeature(a1);
          return PROCEED;
        }

      } else if (oz_isAtom(a1)) {
        put2(' ','-');
        put_atom(a1);
        put(' ');
        StateReturn(put_tcl(sr->getFeature(a1)));
      } else {
        return raise_type_error(tcl);
      }

      as = ar;
    }

    Assert(*ret);
    return PROCEED;
  } else {
    return raise_type_error(tcl);
  }

}


OZ_BI_define(BIwif_init, 3, 0) {

  wif.init(smallIntValue(oz_deref(OZ_in(0))),
           oz_deref(OZ_in(1)),
           OZ_in(2));

  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIwif_write, 1, 0) {

  /*
   * OZ_in(0): tickle to be written
   */

  if (OZ_in(0) == WifNameTclClosed) {

    return wif.write();

  } else {

    CHECK_TOPLEVEL;
    // not yet put into buffer!
    ENTER_WIF_LOCK;
    OZ_Return s;

    wif.reset();
    StateExit(wif.put_tcl(OZ_in(0)));
    wif.put('\n');

    wif.start_write();
    OZ_in(0) = WifNameTclClosed;
    return wif.write();

  exit:
    wif.reset();
    LEAVE_WIF_LOCK;
    return s;

  }

} OZ_BI_end


OZ_BI_define(BIwif_writeReturn,3,0) {

  /*
   * OZ_in(0):
   * OZ_in(1):
   * OZ_in(2):
   */

  if (OZ_in(0) == WifNameTclClosed) {
    return wif.write();
  } else {
    CHECK_TOPLEVEL;
    // not yet put into buffer!
    ENTER_WIF_LOCK;
    OZ_Return s;

    wif.reset();
    wif.put2('o', 'z');
    wif.put2('r', ' ');
    wif.put('[');
    StateExit(wif.put_tcl(OZ_in(0)));
    wif.put2(']','\n');

    wif.enterReturn(OZ_in(2), OZ_in(1));

    wif.start_write();
    OZ_in(0) = WifNameTclClosed;
    return wif.write();
  exit:
    wif.reset();
    LEAVE_WIF_LOCK;
    return s;
  }
} OZ_BI_end


OZ_BI_define(BIwif_writeReturnMess,4,0) {

  /*
   * OZ_in(0): tickle object and modifier (for tags and marks)
   * OZ_in(1): return message
   * OZ_in(2): modifier to be put after arg 1 of above (may be unit)
   * OZ_in(3): type cast for return value
   */

  if (OZ_in(0) == WifNameTclClosed) {
    return wif.write();
  } else {
    CHECK_TOPLEVEL;
    // not yet put into buffer!
    ENTER_WIF_LOCK;
    OZ_Return s;
    TaggedRef ret = WifNameTclClosed;
    TaggedRef mess = oz_deref(OZ_in(1));
    TaggedRef frst;

    Assert(!oz_isVariable(mess));

    if (!oz_isSRecord(mess)) {
      s = raise_type_error(mess);
      goto exit;
    }

    frst = tagged2SRecord(mess)->getFeature(newSmallInt(1));

    if (!frst) {
      s = raise_type_error(mess);;
      goto exit;
    }

    wif.reset();
    wif.put2('o', 'z');
    wif.put2('r', ' ');
    wif.put('[');
    StateExit(wif.put_tcl(OZ_in(0)));
    wif.put(' ');
    StateExit(wif.put_tcl(frst));
    wif.put(' ');
    StateExit(wif.put_tcl(OZ_in(2)));
    wif.put(' ');

    StateExit(wif.put_tcl_return(mess, &ret));
    wif.put2(']','\n');

    // Enter return variable and cast
    wif.enterReturn(ret, OZ_in(3));

    wif.start_write();
    OZ_in(0) = WifNameTclClosed;
    return wif.write();

  exit:
    wif.reset();
    LEAVE_WIF_LOCK;
    return s;
  }

} OZ_BI_end


OZ_BI_define(BIwif_writeBatch,1,0) {

  if (OZ_in(0) == WifNameTclClosed) {
    return wif.write();
  } else {
    CHECK_TOPLEVEL;
    // not yet put into buffer!
    ENTER_WIF_LOCK;
    OZ_Return s;

    wif.reset();
    StateExit(wif.put_batch(oz_deref(OZ_in(0)),';'));
    wif.put('\n');

    wif.start_write();
    OZ_in(0) = WifNameTclClosed;
    return wif.write();

  exit:
    wif.reset();
    LEAVE_WIF_LOCK;
    return s;
  }
} OZ_BI_end


OZ_BI_define(BIwif_writeTuple,2,0) {

  if (OZ_in(0) == WifNameTclClosed) {
    return wif.write();
  } else {
    CHECK_TOPLEVEL;
    // not yet put into buffer!
    ENTER_WIF_LOCK;
    OZ_Return s;

    TaggedRef mess = oz_deref(OZ_in(1));
    TaggedRef frst;

    if (!oz_isSRecord(mess)) {
      s = raise_type_error(mess);;
      goto exit;
    }

    Assert(!oz_isVariable(mess));

    frst = tagged2SRecord(mess)->getFeature(newSmallInt(1));

    if (!frst) {
      s = raise_type_error(mess);;
      goto exit;
    }

    wif.reset();
    StateExit(wif.put_tcl(OZ_in(0)));
    wif.put(' ');
    StateExit(wif.put_record_or_tuple(OZ_in(1)));
    wif.put('\n');

    wif.start_write();
    OZ_in(0) = WifNameTclClosed;
    return wif.write();

  exit:
    wif.reset();
    LEAVE_WIF_LOCK;
    return s;
  }

} OZ_BI_end




OZ_BI_define(BIwif_writeTagTuple,3,0) {

  if (OZ_in(0) == WifNameTclClosed) {
    return wif.write();
  } else {
    CHECK_TOPLEVEL;
    // not yet put into buffer!
    ENTER_WIF_LOCK;
    OZ_Return s;
    TaggedRef tuple = oz_deref(OZ_in(2));
    TaggedRef fst;

    Assert(!oz_isVariable(tuple));

    if (!oz_isSRecord(tuple)) {
      s = raise_type_error(tuple);
      goto exit;
    }

    fst = tagged2SRecord(tuple)->getFeature(newSmallInt(1));

    if (!fst) {
      s = raise_type_error(tuple);
      goto exit;
    }

    wif.reset();
    StateExit(wif.put_tcl(OZ_in(0)));
    wif.put(' ');
    StateExit(wif.put_tcl(fst));
    wif.put(' ');
    StateExit(wif.put_tcl(OZ_in(1)));
    wif.put(' ');
    StateExit(wif.put_record_or_tuple(tuple,1));
    wif.put('\n');

    wif.start_write();
    OZ_in(0) = WifNameTclClosed;
    return wif.write();

  exit:
    wif.reset();
    LEAVE_WIF_LOCK;
    return s;
  }
} OZ_BI_end




OZ_BI_define(BIwif_writeFilter,5,0) {

  if (OZ_in(0) == WifNameTclClosed) {
    return wif.write();
  } else {
    CHECK_TOPLEVEL;
    // not yet put into buffer!
    ENTER_WIF_LOCK;
    OZ_Return s;

    wif.reset();
    StateExit(wif.put_tcl(OZ_in(0)));
    wif.put(' ');
    StateExit(wif.put_vs(OZ_in(1)));
    wif.put(' ');
    StateExit(wif.put_tcl_filter(OZ_in(2), oz_deref(OZ_in(3))));
    wif.put(' ');
    StateExit(wif.put_tcl(OZ_in(4)));
    wif.put('\n');

    wif.start_write();
    OZ_in(0) = WifNameTclClosed;
    return wif.write();

  exit:
    wif.reset();
    LEAVE_WIF_LOCK;
    return s;
  }
} OZ_BI_end



OZ_Return WIF::close_hierarchy(Object * o) {
  TaggedRef v = o->replaceFeature(WifNameTclName, WifNameTclClosed);

  if (v == makeTaggedNULL()) {
    return raise_type_error(makeTaggedConst(o));;
  }

  DEREF(v, v_ptr, v_tag);

  Assert(!isVariableTag(v_tag));
  // since the message has been assembled for closing already!

  if (isLiteralTag(v_tag) && literalEq(v,WifNameTclClosed)) {
    // okay, has been closed already
    return PROCEED;
  } else {
    TaggedRef slaves      = o->getFeature(WifNameTclSlaves);

    // close slaves
    if (slaves != makeTaggedNULL()) {
      slaves = oz_deref(slaves);

      while (oz_isCons(slaves)) {
        TaggedRef slave = oz_deref(head(slaves));

        if (oz_isSmallInt(slave)) {
          // this an entry in the event dictionary

          Assert(oz_isDictionary(wif_dict));
          tagged2Dictionary(wif_dict)->remove(slave);

        } else if (oz_isObject(slave)) {
          // this is an object which needs to be closed as well
          OZ_Return s = close_hierarchy(tagged2Object(slave));
          if (s != PROCEED)
            return s;
        }

        slaves = oz_deref(tail(slaves));
      }
    }

    return PROCEED;
  }

}


OZ_BI_define(BIwif_close,2,0) {

  if (OZ_in(0) == WifNameTclClosed) {
    return wif.write();
  } else {
    CHECK_TOPLEVEL;
    // not yet put into buffer!
    ENTER_WIF_LOCK;
    OZ_Return s;

    // Perform closing of objects
    TaggedRef to = OZ_in(1);
    DEREF(to, to_ptr, to_tag);

    Assert(oz_isObject(to));

    Object  * o = tagged2Object(to);
    TaggedRef v = o->getFeature(WifNameTclName);
    TaggedRef slave_entry;

    if (v == makeTaggedNULL()) {
      s = raise_type_error(to);;
      goto exit;
    }

    {
      DEREF(v, v_ptr, v_tag);

      if (isVariableTag(v_tag)) {
        am.addSuspendVarList(v_ptr);
        s = SUSPEND;
        goto exit;
      } else if (isLiteralTag(v_tag) && literalEq(v,WifNameTclClosed)) {
        LEAVE_WIF_LOCK;
        return PROCEED;
      }
    }

    // Create close tcl
    wif.reset();
    StateExit(wif.put_tcl(OZ_in(0)));
    wif.put('\n');


    // okay, let us close it
    slave_entry = o->getFeature(WifNameTclSlaveEntry);

    // remove from parent
    if (slave_entry != makeTaggedNULL()) {
      slave_entry = oz_deref(slave_entry);

      if (oz_isCons(slave_entry)) {
        LTuple * l = tagged2LTuple(slave_entry);
        l->setHead(NameGroupVoid);
        l->setTail(findAliveEntry(tail(slave_entry)));
      }
    }

    wif.close_hierarchy(o);

    wif.start_write();
    OZ_in(0) = WifNameTclClosed;
    return wif.write();

  exit:
    wif.reset();
    LEAVE_WIF_LOCK;
    return s;
  }
} OZ_BI_end


// ---------------------------------------------------------------------
// Counters
// ---------------------------------------------------------------------


OZ_BI_define(BIwif_genTopName,0,1) {
  OZ_RETURN(wif.genTopName());
} OZ_BI_end


OZ_BI_define(BIwif_genWidgetName,1,1) {
  TaggedRef parent = OZ_in(0);

  DEREF(parent, p_ptr, p_tag);

  if (isVariableTag(p_tag))
    OZ_suspendOn(makeTaggedRef(p_ptr));

  OZ_RETURN(wif.genWidgetName(parent));
} OZ_BI_end


OZ_BI_define(BIwif_genTagName,0,1) {
  OZ_RETURN(wif.genTagName());
} OZ_BI_end


OZ_BI_define(BIwif_genVarName,0,1) {
  OZ_RETURN(wif.genVarName());
} OZ_BI_end


OZ_BI_define(BIwif_genImageName,0,1) {
  OZ_RETURN(wif.genImageName());
} OZ_BI_end


OZ_BI_define(BIwif_getNames,0,3) {
  OZ_out(0) = WifNameTclSlaves;
  OZ_out(1) = WifNameTclSlaveEntry;
  OZ_out(2) = WifNameTclName;
  return PROCEED;
} OZ_BI_end



/*
 * The builtin table
 */

#ifndef STATIC_LIBWIF

OZ_C_proc_interface oz_interface[] = {
#include "libwif.tbl"
 {0,0,0,0}
};

#endif

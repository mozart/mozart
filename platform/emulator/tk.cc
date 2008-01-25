/*
 *  Authors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
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
 *     http://www.mozart-oz.org
 *
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#include "builtins.hh"
#include "os.hh"
#include "am.hh"
#include "dictionary.hh"
#include "bytedata.hh"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

TaggedRef
  TkAtomTclOption, TkAtomTclList, TkAtomTclPosition,
  TkAtomTclQuote, TkAtomTclString, TkAtomTclVS,
  TkAtomTclBatch, TkAtomTclColor, TkAtomTclDelete,
  TkAtomDot,
  TkAtomTagPrefix, TkAtomVarPrefix, TkAtomImagePrefix,
  TkAtomFontPrefix,

  TkNameTclName,
  TkNameTclClosed,
  TkNameTclSlaves,
  TkNameTclSlaveEntry;


void tkInitLiterals() {
  TkAtomTclOption    = OZ_atom("o");
  TkAtomTclDelete    = OZ_atom("d");
  TkAtomTclList      = OZ_atom("l");
  TkAtomTclPosition  = OZ_atom("p");
  TkAtomTclQuote     = OZ_atom("q");
  TkAtomTclString    = OZ_atom("s");
  TkAtomTclVS        = OZ_atom("v");
  TkAtomTclBatch     = OZ_atom("b");
  TkAtomTclColor     = OZ_atom("c");
  TkAtomDot          = OZ_atom(".");
  TkAtomTagPrefix    = OZ_atom("t");
  TkAtomVarPrefix    = OZ_atom("v");
  TkAtomImagePrefix  = OZ_atom("i");
  TkAtomFontPrefix   = OZ_atom("f");

  TkNameTclName       = OZ_newName(); OZ_protect(&TkNameTclName);
  TkNameTclSlaves     = OZ_newName(); OZ_protect(&TkNameTclSlaves);
  TkNameTclSlaveEntry = OZ_newName(); OZ_protect(&TkNameTclSlaveEntry);
  TkNameTclClosed     = OZ_newName(); OZ_protect(&TkNameTclClosed);
}

/*
 * Exceptions
 */

static OZ_Return raise_os_error(const char*s) {
  int xx = errno;
  return oz_raise(E_SYSTEM,E_OS,"os",3,OZ_atom((OZ_CONST char*)s),OZ_int(xx),
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
{ if (!oz_onToplevel()) return raise_toplevel(); }



/*
 * Locking
 */

#define ENTER_TK_LOCK { \
  TaggedRef t = tk.getLock();           \
  DEREF(t, t_ptr);                      \
  Assert(!oz_isRef(t));                 \
  if (oz_isVarOrRef(t)) {               \
    return oz_addSuspendVarList(t_ptr); \
  } else {                              \
    tk.setLock(oz_newVariable());       \
  }                                     \
}

#define LEAVE_TK_LOCK \
  (void) oz_unify(tk.getLock(), NameUnit);



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

class TK {
  char * buffer;
  char * start;
  char * write_start;
  char * end;
  char * protect_start;

  int tk_fd;

  TaggedRef tk_lock;
  TaggedRef tk_rets;
  TaggedRef tk_dict;

  int widget_ctr;
  int tag_ctr;
  int var_ctr;
  int image_ctr;
  int font_ctr;

  void ensure(int n) {
    while (buffer+n>end)
      resize();
  }

public:
  TaggedRef getLock() {
    return tk_lock;
  }

  void setLock(TaggedRef t) {
    tk_lock = t;
  }

  void enterReturn(TaggedRef ret, TaggedRef cast) {
    TaggedRef newt = OZ_cons(OZ_cons(ret,cast),
                             oz_newVariable());
    OZ_Return ures = oz_unify(newt, tk_rets);
    // kost@ : the 'Tk.oz' is supposed to take care of it:
    Assert(ures == PROCEED);
    tk_rets = oz_tail(newt);
  }

  TaggedRef genTopName() {
    SRecord * s = SRecord::newSRecord(AtomPair,2);
    s->setArg(0,TkAtomDot);
    s->setArg(1,oz_int(widget_ctr++));
    return makeTaggedSRecord(s);
  }

  TaggedRef genWidgetName(TaggedRef parent) {
    SRecord * s = SRecord::newSRecord(AtomPair,3);
    s->setArg(0,parent);
    s->setArg(1,TkAtomDot);
    s->setArg(2,oz_int(widget_ctr++));
    return makeTaggedSRecord(s);
  }

  TaggedRef genTagName() {
    SRecord * s = SRecord::newSRecord(AtomPair,2);
    s->setArg(0,TkAtomTagPrefix);
    s->setArg(1,oz_int(tag_ctr++));
    return makeTaggedSRecord(s);
  }

  TaggedRef genVarName() {
    SRecord * s = SRecord::newSRecord(AtomPair,2);
    s->setArg(0,TkAtomVarPrefix);
    s->setArg(1,oz_int(var_ctr++));
    return makeTaggedSRecord(s);
  }

  TaggedRef genImageName() {
    SRecord * s = SRecord::newSRecord(AtomPair,2);
    s->setArg(0,TkAtomImagePrefix);
    s->setArg(1,oz_int(image_ctr++));
    return makeTaggedSRecord(s);
  }

  TaggedRef genFontName() {
    SRecord * s = SRecord::newSRecord(AtomPair,2);
    s->setArg(0,TkAtomFontPrefix);
    s->setArg(1,oz_int(font_ctr++));
    return makeTaggedSRecord(s);
  }

  TK() {}

  void init(int fd, TaggedRef d, TaggedRef r) {

    tkInitLiterals();

    widget_ctr = 0;
    tag_ctr    = 0;
    var_ctr    = 0;
    image_ctr  = 0;
    font_ctr   = 0;

    start         = static_buffer;
    end           = start + STRING_BUFFER_SIZE;
    buffer        = start;

    tk_fd        = fd;
    tk_lock      = NameUnit;
    tk_rets      = r;
    tk_dict      = d;

    (void) oz_protect(&tk_lock);
    (void) oz_protect(&tk_rets);
    (void) oz_protect(&tk_dict);
  }

  void dispose(void) {
    if (start!=static_buffer)
      delete start;
  }

  ~TK() {}

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
      buffer += sprintf(buffer,"%d",tagged2SmallInt(i));
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
    buffer += sprintf(buffer,"%g",floatValue(f));
    ensure(0);
  }

  void put_atom(TaggedRef atom) {
    if (oz_eq(atom, AtomPair) || oz_eq(atom, AtomNil))
      return;
    Assert(oz_isAtom(atom));
    Atom *l = (Atom*) tagged2Literal(atom);
    int n = l->getSize();
    const char *s = l->getPrintName();

    ensure(n);
    memcpy(buffer, s, n * sizeof(char));
    buffer += n;
  }

  void put_atom_quote(TaggedRef atom) {
    if (oz_eq(atom, AtomPair) || oz_eq(atom, AtomNil))
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

  void put_byteString(TaggedRef term) {
    Assert(oz_isByteString(term));
    // term must already be derefed
    ByteString*bs = tagged2ByteString(term);
    int n = bs->getWidth();
    ensure(n);
    for (int i=0;i<n;i++) *buffer++ = bs->get(i);
  }

  void put_byteString_quote(TaggedRef term) {
    Assert(oz_isByteString(term));
      // term must already be derefed
      ByteString*bs = tagged2ByteString(term);
    int n = bs->getWidth();
    ensure(4*n);
    for (int i=0;i<n;i++) put_quote(bs->get(i));
  }

  OZ_Return put_string_quote(TaggedRef list) {
    while (1) {
      TaggedRef h = oz_head(list);
      DEREF(h, h_ptr);

      Assert(!oz_isRef(h));
      if (oz_isVarOrRef(h)) {
        return oz_addSuspendVarList(h_ptr);
      }
      if (!oz_isSmallInt(h))
        return raise_type_error(list);
      int i = tagged2SmallInt(h);
      if (i<0 || i>255)
        return raise_type_error(list);

      put_quote((char) i);
      ensure(0);

      TaggedRef t = oz_tail(list);
      DEREF(t, t_ptr);

      Assert(!oz_isRef(t));
      if (oz_isVarOrRef(t)) {
        return oz_addSuspendVarList(t_ptr);
      }

      if (oz_isLTupleOrRef(t)) {
        list = t;
        continue;
      }

      if (oz_isLiteral(t) && oz_isNil(t))
        return PROCEED;

      return raise_type_error(list);
    }
  }

  OZ_Return put_string(TaggedRef list) {
    while (1) {
      TaggedRef h = oz_head(list);
      DEREF(h, h_ptr);

      Assert(!oz_isRef(h));
      if (oz_isVarOrRef(h)) {
        return oz_addSuspendVarList(h_ptr);
      }
      if (!oz_isSmallInt(h))
        return raise_type_error(list);
      int i = tagged2SmallInt(h);
      if (i<0 || i>255)
        return raise_type_error(list);

      put((char) i);

      TaggedRef t = oz_tail(list);
      DEREF(t, t_ptr);

      Assert(!oz_isRef(t));
      if (oz_isVarOrRef(t)) {
        return oz_addSuspendVarList(t_ptr);
      }

      if (oz_isLTupleOrRef(t)) {
        list = t;
        continue;
      }

      if (oz_isLiteral(t) && oz_isNil(t))
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


static TK tk;


OZ_Return TK::write() {
redo:
  int ret = osTestSelect(tk_fd, SEL_WRITE);

  if (ret < 0)  {
    reset();
    LEAVE_TK_LOCK;
    return raise_os_error("select");
  } else if (ret==0) {
    goto wait_select;
  }

  while ((ret = oswrite(tk_fd, write_start, buffer-write_start)) < 0) {
    if (errno != EINTR) {
      reset();
      LEAVE_TK_LOCK;
      return raise_os_error("write");
    }
  }

  if (buffer - write_start == ret) {
    reset();
    LEAVE_TK_LOCK;
    return PROCEED;
  }

  write_start += ret;
wait_select:
  TaggedRef var = oz_newVariable();

  // Right now we cannot write to tk_fd. So, no additional 'select()'
  // is needed: just suspend until next I/O handling;
  (void) oz_io_select(tk_fd, SEL_WRITE, NameUnit, var);
  DEREF(var, var_ptr);
  Assert(!oz_isRef(var));
  if (oz_isVarOrRef(var)) {
    return oz_addSuspendVarList(var_ptr);
  } else {
    goto redo;
  }
}


void TK::resize(void) {
  int new_size = (3 * (end - start)) / 2;
  char *new_start = new char[new_size + SAFETY_MARGIN];

  end   = new_start + new_size;

  memcpy(new_start, start, buffer-start);

  dispose();

  buffer = (buffer - start) + new_start;
  start  = new_start;
}



OZ_Return TK::put_tuple(SRecord *st, int start) {
  if (start < st->getWidth()) {
    StateReturn(put_tcl(st->getArg(start)));

    for (int i=start+1; i < st->getWidth(); i++) {
      put(' ');
      StateReturn(put_tcl(st->getArg(i)));
    }
  }
  return PROCEED;
}

OZ_Return TK::put_record(SRecord * sr, TaggedRef as) {
  TaggedRef a = oz_head(as);

  StateReturn(put_feature(sr,a));
  as = oz_tail(as);

  while (oz_isCons(as)) {
    a = oz_head(as);
    put(' ');
    StateReturn(put_feature(sr,a));
    as = oz_tail(as);
  }
  return PROCEED;
}

OZ_Return TK::put_batch(TaggedRef batch, char delim) {

  DEREF(batch, batch_ptr);

  Assert(!oz_isRef(batch));
  if (oz_isVarOrRef(batch)) {
    return oz_addSuspendVarList(batch_ptr);
  } else if (oz_isLTuple(batch)) {
    OZ_Return batch_state = put_tcl(oz_head(batch));

    if (batch_state!=PROCEED)
      return batch_state;

    batch = oz_tail(batch);
  } else if (oz_isLiteral(batch) && oz_eq(batch,AtomNil)) {
    return PROCEED;
  } else {
    return raise_type_error(batch);
  }

  while (1) {
    DEREF(batch, batch_ptr);

    Assert(!oz_isRef(batch));
    if (oz_isVarOrRef(batch)) {
      return oz_addSuspendVarList(batch_ptr);
    } else if (oz_isLTupleOrRef(batch)) {
      put(delim);
      OZ_Return batch_state = put_tcl(oz_head(batch));

      if (batch_state!=PROCEED)
        return batch_state;

      batch = oz_tail(batch);
    } else if (oz_isLiteral(batch) && oz_eq(batch,AtomNil)) {
      return PROCEED;
    } else {
      return raise_type_error(batch);
    }
  }
}

OZ_Return TK::put_record_or_tuple(TaggedRef tcl, int start = 0) {
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
      Assert(tagged2SmallInt(oz_head(as))==1);
      as=oz_tail(as);
    }
    if (!oz_isCons(as))
      return PROCEED;

    StateReturn(put_feature(st,oz_head(as)));

    as = oz_tail(as);

    while (oz_isCons(as)) {
      TaggedRef a = oz_head(as);
      put(' ');
      StateReturn(put_feature(st,a));
      as = oz_tail(as);
    }
    return PROCEED;
  }
}

OZ_Return TK::put_vs(TaggedRef vs) {
  DEREF(vs, vs_ptr);

  Assert(!oz_isRef(vs));
  if (oz_isVarOrRef(vs)) {
    return oz_addSuspendVarList(vs_ptr);
  } else if (oz_isSmallInt(vs) || oz_isBigInt(vs)) {
    put_int(vs);
    return PROCEED;
  } else if (oz_isLiteral(vs)) {

    if (!tagged2Literal(vs)->isAtom())
      return raise_type_error(vs);

    put_atom(vs);
    return PROCEED;
  } else if (oz_isSTuple(vs)) {
    SRecord * sr = tagged2SRecord(vs);

    if (!oz_eq(sr->getLabel(),AtomPair))
       return raise_type_error(vs);

    for (int i=0; i < sr->getWidth(); i++) {
      StateReturn(put_vs(sr->getArg(i)));
    }
    return PROCEED;
  } else if (oz_isLTupleOrRef(vs)) {
    return put_string(vs);
  } else if (oz_isFloat(vs)) {
    put_float(vs);
    return PROCEED;
  } else if (oz_isByteString(vs)) {
    put_byteString(vs);
    return PROCEED;
  } else {
    return raise_type_error(vs);
  }
}


OZ_Return TK::put_vs_quote(TaggedRef vs) {
  DEREF(vs, vs_ptr);

  Assert(!oz_isRef(vs));
  if (oz_isVarOrRef(vs)) {
    return oz_addSuspendVarList(vs_ptr);
  } else if (oz_isSmallInt(vs) || oz_isBigInt(vs)) {
    put_int(vs);
    return PROCEED;
  } else if (oz_isLiteral(vs)) {

    if (!tagged2Literal(vs)->isAtom())
      return raise_type_error(vs);

    put_atom_quote(vs);
    return PROCEED;
  } else if (oz_isSTuple(vs)) {
    SRecord * sr = tagged2SRecord(vs);

    if (!oz_eq(sr->getLabel(),AtomPair))
      return raise_type_error(vs);

    for (int i=0; i < sr->getWidth(); i++) {
      StateReturn(put_vs_quote(sr->getArg(i)));
    }
    return PROCEED;
  } else if (oz_isLTupleOrRef(vs)) {
    return put_string_quote(vs);
  } else if (oz_isFloat(vs)) {
    put_float(vs);
    return PROCEED;
  } else if (oz_isByteString(vs)) {
    put_byteString_quote(vs);
    return PROCEED;
  } else {
    return raise_type_error(vs);
  }
}


OZ_Return TK::put_tcl(TaggedRef tcl) {
  DEREF(tcl, tcl_ptr);

  Assert(!oz_isRef(tcl));
  if (oz_isVarOrRef(tcl)) {
    return oz_addSuspendVarList(tcl_ptr);
  } else if (oz_isSmallInt(tcl) || oz_isBigInt(tcl)) {
    put_int(tcl);
    return PROCEED;
  } else if (oz_isLiteral(tcl)) {
    if (tagged2Literal(tcl)->isAtom()) {
      start_protect();
      put_atom_quote(tcl);
      stop_protect();
      return PROCEED;
    } else if (oz_isTrue(tcl)) {
      put('1');
      return PROCEED;
    } else if (oz_isFalse(tcl)) {
      put('0');
      return PROCEED;
    } else if (oz_eq(tcl,NameUnit)) {
      return PROCEED;
    } else {
      return raise_type_error(tcl);
    }
  } else if (oz_isFloat(tcl)) {
    put_float(tcl);
    return PROCEED;
  } else if (oz_isObject(tcl)) {
    TaggedRef v = tagged2Object(tcl)->getFeature(TkNameTclName);

    if (v!=makeTaggedNULL()) {
      DEREF(v, v_ptr);

      Assert(!oz_isRef(v));
      if (oz_isVarOrRef(v)) {
        return oz_addSuspendVarList(v_ptr);
      } else if (oz_isLiteral(v) && oz_eq(v,TkNameTclClosed)) {
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
      if (oz_eq(l,AtomCons)) {
        return raise_type_error(tcl);
      } else if (oz_eq(l,AtomPair)) {
        start_protect();
        StateReturn(put_vs_quote(tcl));
        stop_protect();
        return PROCEED;
      } else if (oz_eq(l,TkAtomTclVS)) {
        TaggedRef arg = st->getArg(0);

        if (st->getWidth() != 1)
          return raise_type_error(tcl);

        DEREF(arg, arg_ptr);

        Assert(!oz_isRef(arg));
        if (oz_isVarOrRef(arg)) {
          return oz_addSuspendVarList(arg_ptr);
        }

        return put_vs(arg);

      } else if (oz_eq(l,TkAtomTclBatch)) {
        if (st->getWidth() != 1)
          return raise_type_error(tcl);

        return put_batch(st->getArg(0), ' ');
      } else if (oz_eq(l,TkAtomTclColor)) {
        if (st->getWidth() != 3)
          return raise_type_error(tcl);

        put('#');
        for (int i=0; i < 3; i++) {
          TaggedRef arg = st->getArg(i);

          DEREF(arg, arg_ptr);

          Assert(!oz_isRef(arg));
          if (oz_isVarOrRef(arg)) {
            return oz_addSuspendVarList(arg_ptr);
          }

          if (!oz_isSmallInt(arg))
            return raise_type_error(tcl);

          int j = tagged2SmallInt(arg);

          if ((j < 0) || (j > 255))
            return raise_type_error(tcl);

          unsigned char c1 = hex_digit(((unsigned char) j & '\xF0') >> 4);
          unsigned char c2 = hex_digit((unsigned char) j & '\x0F');
          put2(c1,c2);
        }
        return PROCEED;
      } else if (oz_eq(l,TkAtomTclOption)) {
        return put_tuple(st);
      } else if (oz_eq(l,TkAtomTclDelete)) {
        if (st->getWidth() != 1)
          return raise_type_error(tcl);
        TaggedRef rt = st->getArg(0);
        DEREF(rt, rt_ptr);
        Assert(!oz_isRef(rt));
        if (oz_isVarOrRef(rt)) {
          return oz_addSuspendVarList(rt_ptr);
        }
        return put_record_or_tuple(rt);
      } else if (oz_eq(l,TkAtomTclList)) {
        put('[');
        StateReturn(put_tuple(st));
        put(']');
        return PROCEED;
      } else if (oz_eq(l,TkAtomTclQuote)) {
        put('{');
        StateReturn(put_tuple(st));
        put('}');
        return PROCEED;
      } else if (oz_eq(l,TkAtomTclString)) {
        put('"');
        StateReturn(put_tuple(st));
        put('"');
        return PROCEED;
      } else if (oz_eq(l,TkAtomTclPosition)) {
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
  } else if (oz_isSRecord(tcl)) {
    SRecord * sr = tagged2SRecord(tcl);
    TaggedRef l  = sr->getLabel();
    TaggedRef as = sr->getArityList(); /* arity list is already deref'ed */

    if (tagged2Literal(l)->isAtom()) {

      if (oz_eq(l,AtomPair) || oz_eq(l,AtomCons) ||
          oz_eq(l,TkAtomTclVS) || oz_eq(l,TkAtomTclBatch) ||
          oz_eq(l,TkAtomTclColor)) {
        return raise_type_error(tcl);
      } else if (oz_eq(l,TkAtomTclOption)) {
        return put_record(sr, as);
      } else if (oz_eq(l,TkAtomTclList)) {
        put('[');
        StateReturn(put_record(sr, as));
        put(']');
        return PROCEED;
      } else if (oz_eq(l,TkAtomTclQuote)) {
        put('{');
        StateReturn(put_record(sr, as));
        put('}');
        return PROCEED;
      } else if (oz_eq(l,TkAtomTclString)) {
        put('"');
        StateReturn(put_record(sr, as));
        put('"');
        return PROCEED;
      } else if (oz_eq(l,TkAtomTclPosition)) {
        put('{');
        StateReturn(put_feature(sr, oz_head(as)));
        put('.');
        if (sr->getWidth() > 1)
          StateReturn(put_record(sr, oz_tail(as)));
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

  } else if (oz_isLTupleOrRef(tcl)) {
    start_protect();
    StateReturn(put_string_quote(tcl));
    stop_protect();
    return PROCEED;
  } else if (oz_isByteString(tcl)) {
    start_protect();
    put_byteString_quote(tcl);
    stop_protect();
    return PROCEED;
  }

  return raise_type_error(tcl);

}


OZ_Return TK::put_tcl_filter(TaggedRef tcl, TaggedRef fs) {
  DEREF(tcl, tcl_ptr);

  if (oz_isLiteral(tcl)) {
    return PROCEED;
  } else if (oz_isSRecord(tcl)) {
    SRecord * sr = tagged2SRecord(tcl);
    TaggedRef as = sr->getArityList(); /* arity list is already deref'ed */
    fs = oz_deref(fs);

    while (oz_isCons(as) && oz_isCons(fs)) {
      TaggedRef a = oz_head(as);
      TaggedRef f = oz_deref(oz_head(fs));

      if (oz_isName(a))
        return raise_type_error(tcl);

      int res = featureCmp(a,f);
      if (!res) {
        fs = oz_deref(oz_tail(fs));
        as = oz_tail(as);
      } else if (res > 0) {
        fs = oz_deref(oz_tail(fs));
      } else {
        StateReturn(put_feature(sr,a));
        put(' ');
        as = oz_tail(as);
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


OZ_Return TK::put_tcl_return(TaggedRef tcl, TaggedRef * ret) {
  *ret = makeTaggedNULL();
  DEREF(tcl, tcl_ptr);

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

  } else if (oz_isSRecord(tcl)) {
    SRecord * sr = tagged2SRecord(tcl);
    TaggedRef as = oz_tail(sr->getArityList()); /* arity list is already deref'ed */

    while (oz_isCons(as)) {
      TaggedRef a1  = oz_head(as);
      TaggedRef ar  = oz_tail(as);

      if (oz_isSmallInt(a1)) {
        if (oz_isCons(ar)) {
          TaggedRef a2 = oz_head(ar);

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


OZ_BI_define(BItk_init, 3, 0) {

  tk.init(tagged2SmallInt(oz_deref(OZ_in(0))),
           oz_deref(OZ_in(1)),
           OZ_in(2));

  return PROCEED;
} OZ_BI_end


OZ_BI_define(BItk_write, 1, 0) {

  /*
   * OZ_in(0): tickle to be written
   */

  if (OZ_in(0) == TkNameTclClosed) {

    return tk.write();

  } else {

    CHECK_TOPLEVEL;
    // not yet put into buffer!
    ENTER_TK_LOCK;
    OZ_Return s;

    tk.reset();
    StateExit(tk.put_tcl(OZ_in(0)));
    tk.put('\n');

    tk.start_write();
    OZ_in(0) = TkNameTclClosed;
    return tk.write();

  exit:
    tk.reset();
    LEAVE_TK_LOCK;
    return s;

  }

} OZ_BI_end


OZ_BI_define(BItk_writeReturn,3,0) {

  /*
   * OZ_in(0):
   * OZ_in(1):
   * OZ_in(2):
   */

  if (OZ_in(0) == TkNameTclClosed) {
    return tk.write();
  } else {
    CHECK_TOPLEVEL;
    // not yet put into buffer!
    ENTER_TK_LOCK;
    OZ_Return s;

    tk.reset();
    tk.put2('o', 'z');
    tk.put2('r', ' ');
    tk.put('[');
    StateExit(tk.put_tcl(OZ_in(0)));
    tk.put2(']','\n');

    tk.enterReturn(OZ_in(2), OZ_in(1));

    tk.start_write();
    OZ_in(0) = TkNameTclClosed;
    return tk.write();
  exit:
    tk.reset();
    LEAVE_TK_LOCK;
    return s;
  }
} OZ_BI_end


OZ_BI_define(BItk_writeReturnMess,4,0) {

  /*
   * OZ_in(0): tickle object and modifier (for tags and marks)
   * OZ_in(1): return message
   * OZ_in(2): modifier to be put after arg 1 of above (may be unit)
   * OZ_in(3): type cast for return value
   */

  if (OZ_in(0) == TkNameTclClosed) {
    return tk.write();
  } else {
    CHECK_TOPLEVEL;
    // not yet put into buffer!
    ENTER_TK_LOCK;
    OZ_Return s;
    TaggedRef ret = TkNameTclClosed;
    TaggedRef mess = oz_deref(OZ_in(1));
    TaggedRef frst;

    Assert(!oz_isVar(mess));

    if (!oz_isSRecord(mess)) {
      s = raise_type_error(mess);
      goto exit;
    }

    frst = tagged2SRecord(mess)->getFeature(makeTaggedSmallInt(1));

    if (!frst) {
      s = raise_type_error(mess);;
      goto exit;
    }

    tk.reset();
    tk.put2('o', 'z');
    tk.put2('r', ' ');
    tk.put('[');
    StateExit(tk.put_tcl(OZ_in(0)));
    tk.put(' ');
    StateExit(tk.put_tcl(frst));
    tk.put(' ');
    StateExit(tk.put_tcl(OZ_in(2)));
    tk.put(' ');

    StateExit(tk.put_tcl_return(mess, &ret));
    tk.put2(']','\n');

    // Enter return variable and cast
    tk.enterReturn(ret, OZ_in(3));

    tk.start_write();
    OZ_in(0) = TkNameTclClosed;
    return tk.write();

  exit:
    tk.reset();
    LEAVE_TK_LOCK;
    return s;
  }

} OZ_BI_end


OZ_BI_define(BItk_writeBatch,1,0) {

  if (OZ_in(0) == TkNameTclClosed) {
    return tk.write();
  } else {
    CHECK_TOPLEVEL;
    // not yet put into buffer!
    ENTER_TK_LOCK;
    OZ_Return s;

    tk.reset();
    StateExit(tk.put_batch(oz_deref(OZ_in(0)),';'));
    tk.put('\n');

    tk.start_write();
    OZ_in(0) = TkNameTclClosed;
    return tk.write();

  exit:
    tk.reset();
    LEAVE_TK_LOCK;
    return s;
  }
} OZ_BI_end


OZ_BI_define(BItk_writeTuple,2,0) {

  if (OZ_in(0) == TkNameTclClosed) {
    return tk.write();
  } else {
    CHECK_TOPLEVEL;
    // not yet put into buffer!
    ENTER_TK_LOCK;
    OZ_Return s;

    TaggedRef mess = oz_deref(OZ_in(1));
    TaggedRef frst;

    if (!oz_isSRecord(mess)) {
      s = raise_type_error(mess);;
      goto exit;
    }

    Assert(!oz_isVar(mess));

    frst = tagged2SRecord(mess)->getFeature(makeTaggedSmallInt(1));

    if (!frst) {
      s = raise_type_error(mess);;
      goto exit;
    }

    tk.reset();
    StateExit(tk.put_tcl(OZ_in(0)));
    tk.put(' ');
    StateExit(tk.put_record_or_tuple(OZ_in(1)));
    tk.put('\n');

    tk.start_write();
    OZ_in(0) = TkNameTclClosed;
    return tk.write();

  exit:
    tk.reset();
    LEAVE_TK_LOCK;
    return s;
  }

} OZ_BI_end




OZ_BI_define(BItk_writeTagTuple,3,0) {

  if (OZ_in(0) == TkNameTclClosed) {
    return tk.write();
  } else {
    CHECK_TOPLEVEL;
    // not yet put into buffer!
    ENTER_TK_LOCK;
    OZ_Return s;
    TaggedRef tuple = oz_deref(OZ_in(2));
    TaggedRef fst;

    Assert(!oz_isVar(tuple));

    if (!oz_isSRecord(tuple)) {
      s = raise_type_error(tuple);
      goto exit;
    }

    fst = tagged2SRecord(tuple)->getFeature(makeTaggedSmallInt(1));

    if (!fst) {
      s = raise_type_error(tuple);
      goto exit;
    }

    tk.reset();
    StateExit(tk.put_tcl(OZ_in(0)));
    tk.put(' ');
    StateExit(tk.put_tcl(fst));
    tk.put(' ');
    StateExit(tk.put_tcl(OZ_in(1)));
    tk.put(' ');
    StateExit(tk.put_record_or_tuple(tuple,1));
    tk.put('\n');

    tk.start_write();
    OZ_in(0) = TkNameTclClosed;
    return tk.write();

  exit:
    tk.reset();
    LEAVE_TK_LOCK;
    return s;
  }
} OZ_BI_end




OZ_BI_define(BItk_writeFilter,5,0) {

  if (OZ_in(0) == TkNameTclClosed) {
    return tk.write();
  } else {
    CHECK_TOPLEVEL;
    // not yet put into buffer!
    ENTER_TK_LOCK;
    OZ_Return s;

    tk.reset();
    StateExit(tk.put_tcl(OZ_in(0)));
    tk.put(' ');
    StateExit(tk.put_vs(OZ_in(1)));
    tk.put(' ');
    StateExit(tk.put_tcl_filter(OZ_in(2), oz_deref(OZ_in(3))));
    tk.put(' ');
    StateExit(tk.put_tcl(OZ_in(4)));
    tk.put('\n');

    tk.start_write();
    OZ_in(0) = TkNameTclClosed;
    return tk.write();

  exit:
    tk.reset();
    LEAVE_TK_LOCK;
    return s;
  }
} OZ_BI_end



OZ_Return TK::close_hierarchy(Object * o) {
  TaggedRef v = o->replaceFeature(TkNameTclName, TkNameTclClosed);

  if (v == makeTaggedNULL()) {
    return raise_type_error(makeTaggedConst(o));;
  }

  DEREF(v, v_ptr);

  Assert(!oz_isVar(v));
  // since the message has been assembled for closing already!

  if (oz_isLiteral(v) && oz_eq(v,TkNameTclClosed)) {
    // okay, has been closed already
    return PROCEED;
  } else {
    TaggedRef slaves      = o->getFeature(TkNameTclSlaves);

    // close slaves
    if (slaves != makeTaggedNULL()) {
      slaves = oz_deref(slaves);

      Assert(!oz_isRef(slaves));
      while (oz_isLTuple(slaves)) {
        TaggedRef slave = oz_deref(oz_head(slaves));

        if (oz_isSmallInt(slave)) {
          // this an entry in the event dictionary

          Assert(oz_isDictionary(tk_dict));
          tagged2Dictionary(tk_dict)->remove(slave);

        } else if (oz_isObject(slave)) {
          // this is an object which needs to be closed as well
          OZ_Return s = close_hierarchy(tagged2Object(slave));
          if (s != PROCEED)
            return s;
        }

        slaves = oz_deref(oz_tail(slaves));
        Assert(!oz_isRef(slaves));
      }
    }

    return PROCEED;
  }

}

inline
TaggedRef findAliveEntry(TaggedRef group) {
  group = oz_deref(group);

  Assert(!oz_isRef(group));
  while (oz_isLTuple(group)) {
      TaggedRef ahead = oz_deref(oz_head(group));

      if (!(oz_isLiteral(ahead) && oz_eq(ahead,NameGroupVoid)))
        return group;

      group = oz_deref(oz_tail(group));
  }

  return group;
}

OZ_BI_define(BItk_close,2,0) {

  if (OZ_in(0) == TkNameTclClosed) {
    return tk.write();
  } else {
    CHECK_TOPLEVEL;
    // not yet put into buffer!
    ENTER_TK_LOCK;
    OZ_Return s;

    // Perform closing of objects
    TaggedRef to = OZ_in(1);
    DEREF(to, to_ptr);

    Assert(oz_isObject(to));

    Object  * o = tagged2Object(to);
    TaggedRef v = o->getFeature(TkNameTclName);
    TaggedRef slave_entry;

    if (v == makeTaggedNULL()) {
      s = raise_type_error(to);;
      goto exit;
    }

    {
      DEREF(v, v_ptr);

      Assert(!oz_isRef(v));
      if (oz_isVarOrRef(v)) {
        s = oz_addSuspendVarList(v_ptr);
        goto exit;
      } else if (oz_isLiteral(v) && oz_eq(v,TkNameTclClosed)) {
        LEAVE_TK_LOCK;
        return PROCEED;
      }
    }

    // Create close tcl
    tk.reset();
    StateExit(tk.put_tcl(OZ_in(0)));
    tk.put('\n');


    // okay, let us close it
    slave_entry = o->getFeature(TkNameTclSlaveEntry);

    // remove from parent
    if (slave_entry != makeTaggedNULL()) {
      slave_entry = oz_deref(slave_entry);

      Assert(!oz_isRef(slave_entry));
      if (oz_isLTuple(slave_entry)) {
        LTuple * l = tagged2LTuple(slave_entry);
        l->setHead(NameGroupVoid);
        l->setTail(findAliveEntry(oz_tail(slave_entry)));
      }
    }

    tk.close_hierarchy(o);

    tk.start_write();
    OZ_in(0) = TkNameTclClosed;
    return tk.write();

  exit:
    tk.reset();
    LEAVE_TK_LOCK;
    return s;
  }
} OZ_BI_end


// ---------------------------------------------------------------------
// Counters
// ---------------------------------------------------------------------


OZ_BI_define(BItk_genTopName,0,1) {
  OZ_RETURN(tk.genTopName());
} OZ_BI_end


OZ_BI_define(BItk_genWidgetName,1,1) {
  TaggedRef parent = OZ_in(0);

  DEREF(parent, p_ptr);

  Assert(!oz_isRef(parent));
  if (oz_isVarOrRef(parent))
    OZ_suspendOn(makeTaggedRef(p_ptr));

  OZ_RETURN(tk.genWidgetName(parent));
} OZ_BI_end


OZ_BI_define(BItk_genTagName,0,1) {
  OZ_RETURN(tk.genTagName());
} OZ_BI_end


OZ_BI_define(BItk_genVarName,0,1) {
  OZ_RETURN(tk.genVarName());
} OZ_BI_end


OZ_BI_define(BItk_genImageName,0,1) {
  OZ_RETURN(tk.genImageName());
} OZ_BI_end


OZ_BI_define(BItk_genFontName,0,1) {
  OZ_RETURN(tk.genFontName());
} OZ_BI_end


OZ_BI_define(BItk_getNames,0,3) {
  OZ_out(0) = TkNameTclSlaves;
  OZ_out(1) = TkNameTclSlaveEntry;
  OZ_out(2) = TkNameTclName;
  return PROCEED;
} OZ_BI_end


/*
 * Groups
 */


OZ_BI_define(BItk_addGroup,2,1)
{
  OZ_expectDet(0);
  TaggedRef group = oz_deref(OZ_in(0));

  Assert(!oz_isRef(group));
  if (oz_isLTupleOrRef(group)) {
    TaggedRef member = oz_cons(OZ_in(1),findAliveEntry(oz_tail(group)));
    tagged2LTuple(group)->setTail(member);
    OZ_RETURN(member);
  }
  return OZ_typeError(0,"List");
} OZ_BI_end


OZ_BI_define(BItk_delGroup,1,0)
{
  TaggedRef member = oz_deref(OZ_in(0));

  Assert(!oz_isRef(member));
  if (oz_isLTuple(member)) {
    tagged2LTuple(member)->setHead(NameGroupVoid);
    tagged2LTuple(member)->setTail(findAliveEntry(oz_tail(member)));
  }

  return PROCEED;
} OZ_BI_end


/*
 * The builtin table
 */

#ifndef MODULES_LINK_STATIC

#include "modTk-if.cc"

#endif

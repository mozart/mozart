/*
 *  Authors:
 *    Ralf Scheidhauer <Ralf.Scheidhauer@ps.uni-sb.de>
 *    Konstantin Popov <kost@sics.se>
 * 
 *  Contributors:
 *    Per Brand <perbrand@sics.se>
 *    Michael Mehl <mehl@dfki.de>
 *    Denys Duchier <duchier@ps.uni-sb.de>
 *    Andreas Sundstroem <andreas@sics.se>
 * 
 *  Copyright:
 *    Organization or Person (Year(s))
 * 
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 * 
 *  This file is part of Mozart, an implementation 
 *  of Oz 3:
 *     http://mozart.ps.uni-sb.de
 * 
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#ifndef __PICKLEBASE_H
#define __PICKLEBASE_H

#if defined(INTERFACE)
#pragma interface
#endif

#include "base.hh"
#include "marshalerBase.hh"

// text format;
#define TAG_STRING    'S'
#define TAG_INT       'I'
#define TAG_DIF       'D'
#define TAG_OPCODE    'O'
#define TAG_LABELREF  'L'
#define TAG_LABELDEF  'l'
#define TAG_BYTE      'B'
#define TAG_COMMENT   '#'
#define TAG_CODESTART 'E'
#define TAG_CODEEND   'e'
#define TAG_TERMREF   'T'
#define TAG_TERMDEF   't'
#define TAG_EOF       -1

#if !defined(TEXT2PICKLE)

#define MSGFLAG_TEXTMODE  0x1
#define MSGFLAG_ATEND     0x2

//
class PickleBuffer : public MarshalerBuffer {
private:
  int flags;

  //
public:
  PickleBuffer() : flags(0) {}

  void setTextmode() { flags |= MSGFLAG_TEXTMODE; }
  Bool textmode()    { return (flags&MSGFLAG_TEXTMODE); }

  void markEnd()  { flags |= MSGFLAG_ATEND; }
  Bool atEnd()    { return (flags&MSGFLAG_ATEND); }
};

#endif

//
void marshalDIF(PickleBuffer *bs, MarshalTag tag);
void marshalByte(PickleBuffer *bs, unsigned char c);
void marshalShort(PickleBuffer *bs, unsigned short i);
void marshalNumber(PickleBuffer *bs, unsigned int i);
void marshalString(PickleBuffer *bs, const char *s);
void marshalLabel(PickleBuffer *bs, int start, int lbl);
void marshalOpCode(PickleBuffer *bs, int lbl, Opcode op, Bool showLabel = 1);
void marshalCodeStart(PickleBuffer *bs);
void marshalCodeEnd(PickleBuffer *bs);
void marshalTermDef(PickleBuffer *bs, int lbl);
void marshalTermRef(PickleBuffer *bs, int lbl);

#if !defined(TEXT2PICKLE)

//
void marshalFloat(PickleBuffer *bs, double d);
void marshalGName(PickleBuffer *bs, GName *gname);
void marshalSmallInt(PickleBuffer *bs, OZ_Term siTerm);
void marshalFloat(PickleBuffer *bs, OZ_Term floatTerm);
void marshalLiteral(PickleBuffer *bs, OZ_Term litTerm, int litTermInd);
void marshalBigInt(PickleBuffer *bs, OZ_Term biTerm, ConstTerm *biConst);
//
void marshalProcedureRef(GenTraverser *gt,
			 AbstractionEntry *entry, PickleBuffer *bs);
void marshalRecordArity(GenTraverser *gt,
			SRecordArity sra, PickleBuffer *bs);
void marshalPredId(GenTraverser *gt, PrTabEntry *p, PickleBuffer *bs);
void marshalCallMethodInfo(GenTraverser *gt,
			   CallMethodInfo *cmi, PickleBuffer *bs);
void marshalGRegRef(AssRegArray *gregs, PickleBuffer *bs);
void marshalLocation(Builtin *bi, OZ_Location *loc, PickleBuffer *bs);
void marshalHashTableRef(GenTraverser *gt,
			 int start, IHashTable *table, PickleBuffer *bs);


//
#ifdef USE_FAST_UNMARSHALER

char *unmarshalString(PickleBuffer *bs);
char *unmarshalVersionString(PickleBuffer *bs);

#else

char *unmarshalStringRobust(PickleBuffer *bs, int *error);
char *unmarshalVersionStringRobust(PickleBuffer *, int *error);

#endif

#endif

/* magic marker for start of saved components */
const char SYSLETHEADER = 2;

//
typedef unsigned int32 crc_t;
//
crc_t update_crc(crc_t crc, unsigned char *buf, int len);
inline crc_t init_crc() { return 0; }

//
char* makeHeader(crc_t crc, int *headerSize);

#endif

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
class PickleMarshalerBuffer : public MarshalerBuffer {
private:
  int flags;

  //
public:
  PickleMarshalerBuffer() : flags(0) {}

  void setTextmode() { flags |= MSGFLAG_TEXTMODE; }
  Bool textmode()    { return (flags&MSGFLAG_TEXTMODE); }

  void markEnd()  { flags |= MSGFLAG_ATEND; }
  Bool atEnd()    { return (flags&MSGFLAG_ATEND); }
};

#endif

//
void marshalDIF(PickleMarshalerBuffer *bs, MarshalTag tag);
void marshalByte(PickleMarshalerBuffer *bs, unsigned char c);
void marshalShort(PickleMarshalerBuffer *bs, unsigned short i);
void marshalNumber(PickleMarshalerBuffer *bs, unsigned int i);
void marshalString(PickleMarshalerBuffer *bs, const char *s);
void marshalLabel(PickleMarshalerBuffer *bs, int start, int lbl);
void marshalOpCode(PickleMarshalerBuffer *bs, int lbl, Opcode op, Bool showLabel = 1);
void marshalCodeStart(PickleMarshalerBuffer *bs);
void marshalCodeEnd(PickleMarshalerBuffer *bs);
void marshalTermDef(PickleMarshalerBuffer *bs, int lbl);
void marshalTermRef(PickleMarshalerBuffer *bs, int lbl);

#if !defined(TEXT2PICKLE)

//
void marshalFloat(PickleMarshalerBuffer *bs, double d);
void marshalGName(PickleMarshalerBuffer *bs, GName *gname);
void marshalSmallInt(PickleMarshalerBuffer *bs, OZ_Term siTerm);
void marshalFloat(PickleMarshalerBuffer *bs, OZ_Term floatTerm);
void marshalLiteral(PickleMarshalerBuffer *bs, OZ_Term litTerm, int litTermInd);
void marshalBigInt(PickleMarshalerBuffer *bs, OZ_Term biTerm, ConstTerm *biConst);
//
void marshalProcedureRef(GenTraverser *gt,
			 AbstractionEntry *entry, PickleMarshalerBuffer *bs);
void marshalRecordArity(GenTraverser *gt,
			SRecordArity sra, PickleMarshalerBuffer *bs);
void marshalPredId(GenTraverser *gt, PrTabEntry *p, PickleMarshalerBuffer *bs);
void marshalCallMethodInfo(GenTraverser *gt,
			   CallMethodInfo *cmi, PickleMarshalerBuffer *bs);
void marshalGRegRef(AssRegArray *gregs, PickleMarshalerBuffer *bs);
void marshalLocation(Builtin *bi, OZ_Location *loc, PickleMarshalerBuffer *bs);
void marshalHashTableRef(GenTraverser *gt,
			 int start, IHashTable *table, PickleMarshalerBuffer *bs);


//
#ifdef USE_FAST_UNMARSHALER

char *unmarshalString(PickleMarshalerBuffer *bs);
char *unmarshalVersionString(PickleMarshalerBuffer *bs);

#else

char *unmarshalStringRobust(PickleMarshalerBuffer *bs, int *error);
char *unmarshalVersionStringRobust(PickleMarshalerBuffer *, int *error);

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

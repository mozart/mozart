/* -*- C++ -*-
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

#ifdef INTERFACE
#pragma implementation "pickleBase.hh"
#endif

#include "base.hh"
#include "pickleBase.hh"

//
// kost@ : The idea of 'pickleBase' is to hide those 'marshal*(...)'
// procedures that are supposed to understand the 'text' format.

//
// Aux supporting stuff;
static
void putVerbatim(MarshalerBuffer *bs, const char *s)
{
  while (*s) {
    bs->put(*s);
    s++;
  }
}

//
#define oz_isalnum(c) ((c) >= 'a' && (c) <= 'z' || \
		       (c) >= 0337 && (c) <= 0366 || \
		       (c) >= 0370 && (c) <= 0377 || \
		       (c) >= 'A' && (c) <= 'Z' || \
		       (c) >= 0300 && (c) <= 0326 || \
		       (c) >= 0330 && (c) <= 0336 || \
		       (c) >= '0' && (c) <= '9' || \
		       (c) == '_')

//
static inline
void putQuotedString(MarshalerBuffer *bs, const char *s)
{
  unsigned char c;
  bs->put('\'');
  while ((c = *s)) {
    if (c == '\'' || c == '\\') {
      bs->put('\\');
      bs->put(c);
    } else if (c >= 32 && c <= 126 || c >= 160) {
      bs->put(c);
    } else {
      bs->put('\\');
      switch (c) {
      case '\'':
	bs->put('\'');
	break;
      case '\a':
	bs->put('a');
	break;
      case '\b':
	bs->put('b');
	break;
      case '\f':
	bs->put('f');
	break;
      case '\n':
	bs->put('n');
	break;
      case '\r':
	bs->put('r');
	break;
      case '\t':
	bs->put('t');
	break;
      case '\v':
	bs->put('v');
	break;
      default:
	bs->put((char) (((c >> 6) & '\007') + '0'));
	bs->put((char) (((c >> 3) & '\007') + '0'));
	bs->put((char) (( c       & '\007') + '0'));
	break;
      }
    }
    s++;
  }
  bs->put('\'');
}

//
static
void putString(MarshalerBuffer *bs, const char *s)
{
  const char *t = s;
  unsigned char c = *t++;
  if (c == '\0' || !oz_isalnum(c)) {
    putQuotedString(bs, s);
  } else {
    c = *t++;
    while (c) {
      if (!oz_isalnum(c)) {
	putQuotedString(bs, s);
	return;
      }
      c = *t++;
    }
    while (*s) {
      bs->put(*s);
      s++;
    }
  }
}

//
static
void putNumber(PickleMarshalerBuffer *bs, unsigned int i)
{
  Assert(bs->textmode());
  char buf[100];
  sprintf(buf,"%u",i);
  putVerbatim(bs, buf);
}

//
static
void putTag(PickleMarshalerBuffer *bs, char tag)
{
  if (bs->textmode()) {
    switch (tag) {
      //  case TAG_DIF:
    case TAG_OPCODE:
    case TAG_LABELDEF:
      bs->put('\n');
      break;
    default:
      bs->put(' ');
      break;
    }
    bs->put(tag);
    bs->put(':');
  }
}

//
void marshalDIF(PickleMarshalerBuffer *bs, MarshalTag tag)
{
  if (bs->textmode()) {
    putTag(bs, TAG_DIF);
    putVerbatim(bs, dif_names[tag].name);
  } else {
    marshalDIF((MarshalerBuffer *) bs, tag);
  }
}

//
void marshalByte(PickleMarshalerBuffer *bs, unsigned char c)
{
  if (bs->textmode()) {
    putTag(bs, TAG_BYTE);
    putNumber(bs, c);
  } else { 
    marshalByte((MarshalerBuffer *) bs, c);
  }
}

//
void marshalShort(PickleMarshalerBuffer *bs, unsigned short i)
{
  if (bs->textmode()) {
    for (int k=0; k<shortSize; k++) {
      putTag(bs, TAG_BYTE);
      putNumber(bs, i&0xFF);
      i = i>>8;
    }
  } else {
    marshalShort((MarshalerBuffer *) bs, i);
  }
}

//
void marshalNumber(PickleMarshalerBuffer *bs, unsigned int i)
{
  if (bs->textmode()) {
    putTag(bs, TAG_INT);
    putNumber(bs, i);
  } else {
    marshalNumber((MarshalerBuffer *) bs, i);
  }
}

//
void marshalString(PickleMarshalerBuffer *bs, const char *s)
{
  if (bs->textmode()) {
    putTag(bs, TAG_STRING);
    putString(bs, s);
  } else {
    marshalString((MarshalerBuffer *) bs, s);
  }
}

//
void marshalLabel(PickleMarshalerBuffer *bs, int start, int lbl)
{
  if (bs->textmode()) {
    putTag(bs, TAG_LABELREF);
    putNumber(bs, start+lbl);
    return;
  } else {
    marshalLabel((MarshalerBuffer *)bs, start, lbl);
  }
}

//
void marshalOpCode(PickleMarshalerBuffer *bs, int lbl, Opcode op, Bool showLabel)
{
  if (bs->textmode()) {
    if (showLabel) {
      putTag(bs, TAG_LABELDEF);
      putNumber(bs, lbl);
    }
    putTag(bs, TAG_OPCODE);
    putVerbatim(bs, opcodeToString(op));
  } else {
    marshalOpCode((MarshalerBuffer *) bs, lbl, op, showLabel);
  }
}

//
void marshalCodeStart(PickleMarshalerBuffer *bs)
{	
  if (bs->textmode()) {
    putTag(bs, TAG_CODESTART);
    return;
  } else {
    marshalCodeStart((MarshalerBuffer *) bs);
  }
}

//
void marshalCodeEnd(PickleMarshalerBuffer *bs)
{	
  if (bs->textmode()) {
    putTag(bs, TAG_CODEEND);
  } else {
    marshalCodeEnd((MarshalerBuffer *) bs);
  }
}

//
void marshalTermDef(PickleMarshalerBuffer *bs, int lbl)
{
  if (bs->textmode()) {
    putTag(bs, TAG_TERMDEF);
    putNumber(bs, lbl);
  } else {
    marshalTermDef((MarshalerBuffer *) bs, lbl);
  }
}

//
void marshalTermRef(PickleMarshalerBuffer *bs, int lbl)
{
  if (bs->textmode()) {
    putTag(bs, TAG_TERMREF);
    putNumber(bs, lbl);
  } else {
    marshalTermRef((MarshalerBuffer *) bs, lbl);
  }
}

#if !defined(TEXT2PICKLE)

//
#define MARSHALERBUFFER		PickleMarshalerBuffer
#include "marshalerBaseShared.cc"
#undef  MARSHALERBUFFER

//
#ifdef USE_FAST_UNMARSHALER

//
static
char *getString(PickleMarshalerBuffer *bs, unsigned int i)
{
  char *ret = new char[i+1];
  if (ret == (char *) 0)
    return ((char *) 0);
  for (unsigned int k=0; k<i; k++) {
    if (bs->atEnd()) {
      delete ret;
      return ((char *) 0);
    }
    ret[k] = bs->get();
  }
  ret[i] = '\0';
  return (ret);
}

//
char *unmarshalString(PickleMarshalerBuffer *bs)
{
  unsigned int i = unmarshalNumber(bs);
  return (getString(bs,i));
}

//
char *unmarshalVersionString(PickleMarshalerBuffer *bs)
{
  unsigned int i = bs->get();
  return getString(bs,i);
}

#else

//
static
char *getStringRobust(PickleMarshalerBuffer *bs, unsigned int i, int *error)
{
  char *ret = new char[i+1];
  if (ret == (char *) 0) {
    *error = OK;
    return ((char *) 0);
  }
  for (unsigned int k=0; k<i; k++) {
    if (bs->atEnd()) {
      delete ret;
      *error = OK;
      return ((char *) 0);
    }
    ret[k] = bs->get();
  }
  ret[i] = '\0';
  *error = NO;
  return (ret);
}

//
char *unmarshalStringRobust(PickleMarshalerBuffer *bs, int *error)
{
  char *string;
  unsigned int i = unmarshalNumberRobust(bs,error);
  if(*error) return NULL;
  string = getStringRobust(bs,i,error);
  return string;
}

//
char *unmarshalVersionStringRobust(PickleMarshalerBuffer *bs, int *error)
{
  unsigned int i = bs->get();
  return getStringRobust(bs,i,error);
}

#endif

#endif

//
// The following sample code represents a practical implementation of the
// CRC (Cyclic Redundancy Check) employed in PNG chunks. (See also ISO
// 3309 [ISO-3309] or ITU-T V.42 [ITU-V42] for a formal specification.) 
//

/* Table of CRCs of all 8-bit messages. */
static crc_t crc_table[256];
   
/* Make the table for a fast CRC. */
static
void make_crc_table(void) 
{
  crc_t c;
  int n, k;
   
  for (n = 0; n < 256; n++) {
    c = (crc_t) n;
    for (k = 0; k < 8; k++) {
      if (c & 1)
	c = 0xedb88320L ^ (c >> 1);
      else
	c = c >> 1;
    }
    crc_table[n] = c;
  }

}

/* Update a running CRC with the bytes buf[0..len-1]--the CRC
   should be initialized to all 1's, and the transmitted value
   is the 1's complement of the final running CRC (see the
   crc() routine below)). */

crc_t update_crc(crc_t crc, unsigned char *buf, int len) 
{
  static int tablemade = 0;
  if (!tablemade) {
    make_crc_table();
    tablemade = 1;
  }

  crc_t c = crc;
  int n;

  for (n = 0; n < len; n++) {
    c = crc_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);
  }
  return c;
}

//
char *makeHeader(crc_t crc, int *headerSize)
{
  static char buf[20];
  sprintf(buf,"%c%c%c%c%c%c%c",
	  SYSLETHEADER,SYSLETHEADER,SYSLETHEADER,
	  (char) (crc>> 0)&0xff,
	  (char) (crc>> 8)&0xff,
	  (char) (crc>>16)&0xff,
	  (char) (crc>>24)&0xff);
  *headerSize = 7;
  return buf;
}

/*
 *  Authors:
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
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

#ifdef TEXT2PICKLE
#define EmulatorOnly(Code)
#else
#define EmulatorOnly(Code) Code
#endif


#include "pickle.hh"


void putVerbatim(const char *s, MsgBuffer *bs)
{
  while (*s) {
    bs->put(*s);
    s++;
  }
}

#define oz_isalnum(c) ((c) >= 'a' && (c) <= 'z' || \
                       (c) >= 0337 && (c) <= 0366 || \
                       (c) >= 0370 && (c) <= 0377 || \
                       (c) >= 'A' && (c) <= 'Z' || \
                       (c) >= 0300 && (c) <= 0326 || \
                       (c) >= 0330 && (c) <= 0336 || \
                       (c) >= '0' && (c) <= '9' || \
                       (c) == '_')

inline
void putQuotedString(const char *s, MsgBuffer *bs)
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

void putString(const char *s, MsgBuffer *bs)
{
  const char *t = s;
  unsigned char c = *t++;
  if (c == '\0' || !oz_isalnum(c)) {
    putQuotedString(s,bs);
  } else {
    c = *t++;
    while (c) {
      if (!oz_isalnum(c)) {
        putQuotedString(s,bs);
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

void putTag(char tag, MsgBuffer *bs)
{
  if (!bs->textmode()) {
    // bs->put(tag);
    return;
  }

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

void putNumber(unsigned int i,MsgBuffer *bs)
{
  char buf[100];
  sprintf(buf,"%u",i);
  putVerbatim(buf,bs);
}




void putComment(char *s,MsgBuffer *bs)
{
  if (bs->textmode()) {
    putTag(TAG_COMMENT,bs);
    while (*s) {
      bs->put(*s);
      s++;
    }
    bs->put('\n');
  }
}



void marshalDIF(MsgBuffer *bs, MarshalTag tag)
{
  EmulatorOnly(dif_counter[tag].send());
  if (bs->textmode()) {
    putTag(TAG_DIF,bs);
    putVerbatim(dif_names[tag].name,bs);
    return;
  }
  bs->put(tag);
}


void marshalByte(unsigned char c, MsgBuffer *bs)
{
  if (bs->textmode()) {
    putTag(TAG_BYTE,bs);
    putNumber(c,bs);
    return;
  }
  bs->put(c);
}


const int shortSize = 2;

void marshalShort(unsigned short i, MsgBuffer *bs)
{
  if (bs->textmode()) {
    for (int k=0; k<shortSize; k++) {
      putTag(TAG_BYTE,bs);
      putNumber(i&0xFF,bs);
      i = i>>8;
    }
    return;
  }
  for (int k=0; k<shortSize; k++) {
    bs->put(i&0xFF);
    i = i>>8;
  }
}



#define SBit (1<<7)

void marshalNumber(unsigned int i, MsgBuffer *bs)
{
  if (bs->textmode()) {
    putTag(TAG_INT,bs);
    putNumber(i,bs);
    return;
  }
  while(i >= SBit) {
    bs->put((i%SBit)|SBit);
    i /= SBit;
  }
  bs->put(i);
}

void marshalString(const char *s, MsgBuffer *bs)
{
  EmulatorOnly(misc_counter[MISC_STRING].send());
  if (bs->textmode()) {
    putTag(TAG_STRING,bs);
    putString(s,bs);
    return;
  }

  marshalNumber(strlen(s),bs);
  while(*s) {
    bs->put(*s);
    s++;
  }
}

#ifndef TEXT2PICKLE
BYTE unmarshalByte(MsgBuffer *bs)
{
  return bs->get();
}

int unmarshalNumber(MsgBuffer *bs)
{
  unsigned int ret = 0, shft = 0;
  unsigned int c = bs->get();
  while (c >= SBit) {
    ret += ((c-SBit) << shft);
    c = bs->get();
    shft += 7;
  }
  ret |= (c<<shft);
  return (int) ret;
}
#endif


void marshalLabel(int start, int lbl, MsgBuffer *bs)
{
  //fprintf(stderr,"Label: %d\n",lbl);

  if (bs->textmode()) {
    putTag(TAG_LABELREF,bs);
    putNumber(start+lbl,bs);
    return;
  }

  marshalNumber(lbl,bs);
}


void marshalLabelDef(char *lbl, MsgBuffer *bs)
{
  if (bs->textmode()) {
    putTag(TAG_LABELDEF,bs);
    putString(lbl,bs);
  }
}


void marshalOpCode(int lbl, Opcode op, MsgBuffer *bs, Bool showLabel)
{
  if (bs->textmode()) {
    if (showLabel) {
      putTag(TAG_LABELDEF,bs);
      putNumber(lbl,bs);
    }
    putTag(TAG_OPCODE,bs);
    putVerbatim(opcodeToString(op),bs);
    return;
  }
  bs->put(op);
}


void marshalCodeStart(int codesize, MsgBuffer *bs)
{
  if (bs->textmode()) {
    putTag(TAG_CODESTART,bs);
    return;
  }
  marshalNumber(codesize,bs);
}


void marshalCodeEnd(MsgBuffer *bs)
{
  if (bs->textmode()) {
    putTag(TAG_CODEEND,bs);
  }
}


void marshalTermDef(int lbl, MsgBuffer *bs)
{
  if (bs->textmode()) {
    putTag(TAG_TERMDEF,bs);
    putNumber(lbl,bs);
  } else {
    marshalNumber(lbl,bs);
  }
}

void marshalTermRef(int lbl, MsgBuffer *bs)
{
  if (bs->textmode()) {
    putTag(TAG_TERMREF,bs);
    putNumber(lbl,bs);
  } else {
    marshalNumber(lbl,bs);
  }
}


char *makeHeader(crc_t crc, int *headerSize)
{
  static char buf[20];
  unsigned int crc32 = (unsigned int) crc;
  sprintf(buf,"%c%c%c%c%c%c%c",
          SYSLETHEADER,SYSLETHEADER,SYSLETHEADER,
          (char) (crc>> 0)&0xff,
          (char) (crc>> 8)&0xff,
          (char) (crc>>16)&0xff,
          (char) (crc>>24)&0xff);
  *headerSize = 7;
  return buf;
}


// The following sample code represents a practical implementation of the
// CRC (Cyclic Redundancy Check) employed in PNG chunks. (See also ISO
// 3309 [ISO-3309] or ITU-T V.42 [ITU-V42] for a formal specification.)


/* Table of CRCs of all 8-bit messages. */
crc_t crc_table[256];

/* Make the table for a fast CRC. */
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

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

//
// This file is #include"d.
//

//
// kost@ : there must be essentially copies of these routines, but
// with different 'marshalNumber(...)' etc. used. The reason:
// 'marshalNumber' etc. are not virtual methods, but static procedures
// with distinct signatures.

//
void marshalFloat(MARSHALERBUFFER *bs, double d)
{
  static DoubleConv dc;
  dc.u.d = d;
#if defined(ARCH_LITTLE_ENDIAN) && !defined(ARCH_BIG_WORDIAN)
    marshalNumber(bs, dc.u.i[0]);
    marshalNumber(bs, dc.u.i[1]);
#else
    marshalNumber(bs, dc.u.i[1]);
    marshalNumber(bs, dc.u.i[0]);
#endif
}

//
void marshalSmallInt(MARSHALERBUFFER *bs, OZ_Term siTerm)
{
  marshalDIF(bs, DIF_SMALLINT);
  marshalNumber(bs, tagged2SmallInt(siTerm));
}

//
void marshalFloat(MARSHALERBUFFER *bs, OZ_Term floatTerm)
{
  marshalDIF(bs, DIF_FLOAT);
  marshalFloat(bs, tagged2Float(floatTerm)->getValue());
}

//
void marshalGName(MARSHALERBUFFER *bs, GName *gname)
{
  // generic one (not distributions'!)
  gname->site->marshalSiteForGName(bs);
  for (int i = 0; i < fatIntDigits; i++) {
    marshalNumber(bs, gname->id.number[i]);
  }
  marshalNumber(bs, (int) gname->gnameType);
}

//
void marshalLiteral(MARSHALERBUFFER *bs, OZ_Term litTerm, int litTermInd)
{
  Literal *lit = tagged2Literal(litTerm);

  MarshalTag litTag;
  GName *gname = NULL;

  if (lit->isAtom()) {
    litTag = DIF_ATOM;
  } else if (lit->isUniqueName()) {
    litTag = DIF_UNIQUENAME;
  } else if (lit->isCopyableName()) {
    litTag = DIF_COPYABLENAME;
  } else {
    litTag = DIF_NAME;
    gname = ((Name *) lit)->globalize();
  }

  marshalDIF(bs, litTag);
  const char *name = lit->getPrintName();
  marshalTermDef(bs, litTermInd);
  marshalString(bs, name);
  if (gname) marshalGName(bs, gname);
}

//
void marshalBigInt(MARSHALERBUFFER *bs, OZ_Term biTerm, ConstTerm *biConst)
{ 
  marshalDIF(bs, DIF_BIGINT);
  marshalString(bs, toC(biTerm));
}


//
void marshalProcedureRef(GenTraverser *gt,
			 AbstractionEntry *entry, MARSHALERBUFFER *bs)
{
  Bool copyable = entry && entry->isCopyable();
  marshalNumber(bs, copyable);
  if (copyable) {
    int ind = gt->findLocation(entry);
    if (ind >= 0) {
      marshalDIF(bs, DIF_REF);
      marshalTermRef(bs, ind);
    } else {
      marshalDIF(bs, DIF_ABSTRENTRY);
      rememberLocation(gt, bs, entry);
    }
  } else {
    Assert(entry==NULL || entry->getAbstr() != NULL);
  }
}

//
static inline
void marshalRecordArityType(RecordArityType type, MARSHALERBUFFER *bs)
{
  marshalNumber(bs, type);
}

//
//
// NOTE: this code cannot be changed without changing
// marshaling/unmarshaling of pred id"s, hash table refs, gen call
// info"s and app meth info"s !!! Sorry, i (kost@) don't know how to
// express an appropriate assertion... may be the be way is just not
// to use this procedure for marshaling structures named above;
void marshalRecordArity(GenTraverser *gt,
			SRecordArity sra, MARSHALERBUFFER *bs)
{
  if (sraIsTuple(sra)) {
    marshalRecordArityType(TUPLEWIDTH, bs);
    marshalNumber(bs, getTupleWidth(sra));
  } else {
    marshalRecordArityType(RECORDARITY, bs);
    gt->traverseOzValue(getRecordArity(sra)->getList());
  }
}

//
void marshalPredId(GenTraverser *gt, PrTabEntry *p, MARSHALERBUFFER *bs)
{
  gt->traverseOzValue(p->getName());
  marshalRecordArity(gt, p->getMethodArity(), bs);
  gt->traverseOzValue(p->getFile());
  marshalNumber(bs, p->getLine());
  marshalNumber(bs, p->getColumn());
  gt->traverseOzValue(p->getFlagsList());
  marshalNumber(bs, p->getMaxX());
  // Actually, 'gSize' is marshaled twice: the other copy is for
  // 'gRegRef'. I see no easy way to fix that, and it is also used for
  // redundancy check in the 'DEFINITION' instruction;
  marshalNumber(bs, p->getGSize());
}

//
void marshalCallMethodInfo(GenTraverser *gt,
			   CallMethodInfo *cmi, MARSHALERBUFFER *bs)
{
  int compact = (cmi->regIndex<<1) | (cmi->isTailCall);
  marshalNumber(bs, compact);
  gt->traverseOzValue(cmi->mn);
  marshalRecordArity(gt, cmi->arity, bs);
}

//
void marshalGRegRef(AssRegArray *gregs, MARSHALERBUFFER *bs)
{ 
  int nGRegs = gregs->getSize();
  marshalNumber(bs, nGRegs);

  for (int i = 0; i < nGRegs; i++) {
    int out = ((*gregs)[i].getIndex()<<2) | (int)(*gregs)[i].getKind();
    marshalNumber(bs, out);
  }
}

//
void marshalLocation(Builtin *bi, OZ_Location *loc, MARSHALERBUFFER *bs) 
{ 
  int inAr  = bi->getInArity();
  int outAr = bi->getOutArity();
  marshalNumber(bs, inAr);
  marshalNumber(bs, outAr);

  for (int i = 0; i < (inAr + outAr); i++) {
    int out = loc->getIndex(i);
    marshalNumber(bs, out);
  }
}

//
// The hash table is considered to be compound: its subtrees are table
// nodes;
void marshalHashTableRef(GenTraverser *gt,
			 int start, IHashTable *table, MARSHALERBUFFER *bs)
{
  int sz = table->getSize();
  marshalNumber(bs, sz);	// kost@ : but it's not used anymore;
  marshalLabel(bs, start, table->lookupElse());
  marshalLabel(bs, start, table->lookupLTuple());
  int entries = table->getEntries();
  // total number of entries (and, thus, of 'ht_???_entry' tasks);
  marshalNumber(bs, entries);

  //
  for (int i = table->getSize(); i--; ) {
    if (table->entries[i].val) {
      if (oz_isLiteral(table->entries[i].val)) {
	if (table->entries[i].sra == mkTupleWidth(0)) {
	  // That's a literal entry
	  marshalNumber(bs, ATOMTAG);
	  marshalLabel(bs, start, table->entries[i].lbl);
	  gt->traverseOzValue(table->entries[i].val);
	} else {
	  // That's a record entry
	  marshalNumber(bs, RECORDTAG);
	  marshalLabel(bs,start, table->entries[i].lbl);
	  gt->traverseOzValue(table->entries[i].val);
	  marshalRecordArity(gt, table->entries[i].sra, bs);
	}
      } else {
	Assert(oz_isNumber(table->entries[i].val));
	// That's a number entry
	marshalNumber(bs,NUMBERTAG);
	marshalLabel(bs, start, table->entries[i].lbl);
	gt->traverseOzValue(table->entries[i].val);
      }
    }
  }
}

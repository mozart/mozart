/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#ifdef __GNUC__
#pragma implementation "fdomn.hh"
#endif

#include "fdomn.hh"

#include "misc.hh"


#ifdef PROFILE_FD
unsigned FiniteDomain::constrCalled = 0;
unsigned FiniteDomain::unifyCalled = 0;
unsigned FiniteDomain::varsCreated = 0;
#endif

//-----------------------------------------------------------------------------
//                           class BitArray


// Some algorithms of bit fiddeling in this class have been
// taken from Daniel Diaz' CLP(FD).

void BitArray::findLowerUpperSize(unsigned short &lower, unsigned short &upper)
{
  int i, v;

// find lower bound of the bit array
  for (i = 0, v = fdMinBA; i < baSize; v += bits, i++) 
    if (array[i])
      break;
  
  // find lsb from array[i]
  int word = array[i];
  if (!(word << 16)) {
    word >>= 16; v += 16;
  }
  if (!(word << 24)) {
    word >>= 8; v += 8;
  }
  if (!(word << 28)) {
    word >>= 4; v += 4;
  }
  if (!(word << 30)) {
    word >>= 2; v += 2;
  }
  if (!(word << 31))
    v++;

  lower = v;

// find upper bound of the bit array
  for (i = baSize - 1, v = fdMaxBA; i >= 0; v -= bits, i--) 
    if (array[i])
      break;

  // find msb from array[i]
  word = array[i];
  if (!(word >> 16)) {
    word <<= 16; v -= 16;
  }
  if (!(word >> 24)) {
    word <<= 8; v -= 8;
  }
  if (!(word >> 28)) {
    word <<= 4; v -= 4;
  }
  if (!(word >> 30)) {
    word <<= 2; v -= 2;
  }
  if (!(word >> 31))
    v--;

  upper = v;

// find size of the bit array
  static unsigned char numOfBitsInByte[256] = {
    0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
    1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
    1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
    2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
    1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
    2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
    2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
    3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,4,5,5,6,5,6,6,7,5,6,6,7,6,7,7,8
  };
  unsigned auxBitWord;
  i = baSize;
  size = 0;
  while (i--)
    for(auxBitWord = (unsigned) array[i]; auxBitWord; auxBitWord >>= 8)
      size += numOfBitsInByte[auxBitWord & 0xff];
} // BitArray::findLowerUpperSize


void BitArray::setFromTo(int from, int to)
{
#ifdef DEBUG_FD
  if (from < fdMinBA || to > fdMaxBA)
    error("from or to out of range.");
#endif
  
  static int toTheLowerEnd[32] = {
    0x00000001,0x00000003,0x00000007,0x0000000f,
    0x0000001f,0x0000003f,0x0000007f,0x000000ff,
    0x000001ff,0x000003ff,0x000007ff,0x00000fff,
    0x00001fff,0x00003fff,0x00007fff,0x0000ffff,
    0x0001ffff,0x0003ffff,0x0007ffff,0x000fffff,
    0x001fffff,0x003fffff,0x007fffff,0x00ffffff,
    0x01ffffff,0x03ffffff,0x07ffffff,0x0fffffff,
    0x1fffffff,0x3fffffff,0x7fffffff,0xffffffff
  };
  static int toTheUpperEnd[32] = {
    0xffffffff,0xfffffffe,0xfffffffc,0xfffffff8,
    0xfffffff0,0xffffffe0,0xffffffc0,0xffffff80,
    0xffffff00,0xfffffe00,0xfffffc00,0xfffff800,
    0xfffff000,0xffffe000,0xffffc000,0xffff8000,
    0xffff0000,0xfffe0000,0xfffc0000,0xfff80000,
    0xfff00000,0xffe00000,0xffc00000,0xff800000,
    0xff000000,0xfe000000,0xfc000000,0xf8000000,
    0xf0000000,0xe0000000,0xc0000000,0x80000000
  };
  
  int lowerWord = from / bits;
  int lowerBit = from % bits;
  int upperWord = to / bits;
  int upperBit = to % bits;
  int i;

  for (i = 0; i < lowerWord; i++)
    array[i] = 0;
  for (i = upperWord+1; i < baSize; i++)
    array[i] = 0;    

  if (lowerWord == upperWord){
    for (i = lowerBit; i <= upperBit; i++)
      array[lowerWord] = toTheLowerEnd[upperBit] & toTheUpperEnd[lowerBit];
  } else {
    array[lowerWord] = toTheUpperEnd[lowerBit];
    for (i = lowerWord + 1; i < upperWord; i++)
      array[i] = ~0;
    array[upperWord] = toTheLowerEnd[upperBit];
  }

  size = to - from + 1;
} //  BitArray::setFromTo


void BitArray::print(ostream &ofile, int offset) const
{
  ofile << indent(offset) << '{';
  
  int i, r, l;
  for (i = fdMinBA, r = 1; i < fdMaxBA + 2; i++)
    if (isInDomain(i) == OK){
      if (r) ofile << ' '<< i;
      l = r ? i : l;
      r = 0;
    } else {
      if (!r){
	r = 1;
	if ((i - l) > 2)
	  ofile << ".." << i - 1;
	else if ((i - l) > 1)
	  ofile << ' ' << i - 1;
      }
    }

  ofile << " }";
}


void BitArray::printLong(ostream &ofile, int offset) const
{
  ofile << "  baSize: " << baSize << endl;
  print(ofile, offset);
  ofile << endl;
  for (int i = 0; i < baSize; i++) {
    ofile << indent(offset + 2) << '[' << i << "]:  ";
    for (int j = 31; j >= 0; j--) {
      cout << ((array[i] & (1 << j)) ? '1' : 'o');
      if (j % 8 == 0) ofile << ' ';
    }
    ofile<< endl;
  }
}


 
//-----------------------------------------------------------------------------
//                         class FiniteDomain

FDState FiniteDomain::leftDom;
FDState FiniteDomain::rightDom;


void FiniteDomain::print(ostream &ofile, int offset) const
{
  if (isRange() == OK) {
    if (*this == empty)
      ofile << indent(offset) <<"{ - empty - }";
    else
      ofile << indent(offset) << "{ " << lower << ".." << upper << " }";
  } else {
    bitArray->print(ofile, offset);
  }
}


void  FiniteDomain::printLong(ostream &ofile, int offset) const
{
  ofile << indent(offset);
  ofile << "lower: " << lower << "  upper: " << upper
	<< "  size: " << getSize() << "  bitArray: " << bitArray;
  
  if (isRange() == OK) {
    ofile << endl;
    print(ofile,  offset);
    ofile << endl;
  } else {
    bitArray->printLong(ofile, offset);
  }
}

#if defined(OUTLINE) || defined(FDOUTLINE)
#define inline 
#include "fdomn.icc"
#undef inline 
#endif

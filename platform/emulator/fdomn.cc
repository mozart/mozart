/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#if defined(__GNUC__)
#pragma implementation "fdomn.hh"
#endif

#include "fdomn.hh"
#include "misc.hh"

#if defined(OUTLINE) || defined(FDOUTLINE)
#define inline
#include "fdomn.icc"
#undef inline
#endif


#ifdef PROFILE_FD
unsigned FiniteDomain::constrCalled = 0;
unsigned FiniteDomain::unifyCalled = 0;
unsigned FiniteDomain::varsCreated = 0;
#endif

//-----------------------------------------------------------------------------
//                           class BitArray

// Some algorithms of bit fiddeling in this class have been
// taken from Daniel Diaz' CLP(FD).

// find lower bound of the bit array
unsigned short BitArray::findLower(void)
{
  int v = fdMinBA;

  for (int i = 0; i < baSize; v += bits, i++)
    if (array[i] != 0)
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

  return v;
}


// find upper bound of the bit array
unsigned short BitArray::findUpper(void)
{
  int v = fdMaxBA;

  for (int i = baSize - 1; i >= 0; v -= bits, i--)
    if (array[i] != 0)
      break;

  // find msb from array[i]
  int word = array[i];
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

  return v;
}


// find size of the bit array
void BitArray::findSize(void)
{
  unsigned auxBitWord;
  size = 0;
  for (int i = baSize; i--; )
    for(auxBitWord = (unsigned) array[i]; auxBitWord; auxBitWord >>= 16)
      size += numOfBitsInByte[auxBitWord & 0xffff];
}


int BitArray::findLowerUpperSize(unsigned short &lower, unsigned short &upper)
{
  lower = findLower();
  upper = findUpper();
  findSize();
  return size;
}


void BitArray::setFromTo(int from, int to)
{
  if (from > fdMaxBA || to < fdMinBA) {
    empty();
    return;
  }
  if (from < fdMinBA) from = fdMinBA;
  if (to > fdMaxBA) to = fdMaxBA;

  int lowerWord = from / bits;
  int lowerBit = from % bits;
  int upperWord = to / bits;
  int upperBit = to % bits;
  int i;

  for (i = 0; i < lowerWord; i++)
    array[i] = 0;
  for (i = upperWord + 1; i < baSize; i++)
    array[i] = 0;

  if (lowerWord == upperWord){
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
    if (contains(i) == OK){
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


void FiniteDomain::printDebug(void) const
{
  print(cout, 0);
  cout << endl;
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

/*
 *  Authors:
 *    Andreas Sundstrom (andreas@sics.se)
 *    Kostja Popov (kost@sics.se)
 *
 *  Contributors:
 *
 *  Copyright:
 *    1997-1998 Konstantin Popov
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

//---------------------------------------------------------------------
// General unmarshaling procedures included in vs_comm.cc
//---------------------------------------------------------------------

//
// The 'segKeys' array is allocated with "
#ifdef ROBUST_UNMARSHALER
void VirtualSite::unmarshalResourcesRobust(MsgBuffer *mb, int *error)
#else
void VirtualSite::unmarshalResources(MsgBuffer *mb)
#endif
{
#ifdef ROBUST_UNMARSHALER
  segKeysNum = unmarshalNumberRobust(mb, error);
#else
  segKeysNum = unmarshalNumber(mb);
#endif
  Assert(segKeysNum);
  if (segKeysNum > segKeysArraySize) {
    int acc = segKeysNum, bits = 0;
    while (acc) {
      acc = acc/2;
      bits++;
    }
    bits = max(bits+2, 4);      // heuristic...
    segKeysArraySize = (int) (0x1 << bits);

    //
    if (segKeys) free (segKeys);
    segKeys = (key_t *) malloc(sizeof(key_t) * segKeysArraySize);
  }
  Assert(segKeys);
  Assert(segKeysArraySize >= segKeysNum);

  //
#ifdef ROBUST_UNMARSHALER
  for (int i = 0; i < segKeysNum; i++) {
    int e;
    segKeys[i] = (key_t) unmarshalNumberRobust(mb, &e);
    *error = *error || e;
  }
#else
  for (int i = 0; i < segKeysNum; i++)
    segKeys[i] = (key_t) unmarshalNumber(mb);
#endif
}

/*
 *  Authors:
 *    Erik Klintskog, 2002
 *    Zacharias El Banna, 2002
 *
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 *
 *  Copyright:
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

#if defined(INTERFACE)
#pragma implementation "glue_utils.hh"
#endif

#include "glue_utils.hh"
#include <stdlib.h>

//*********************************************************************
//* Base64 - a simple base64 encoder and decoder.
//*
//*     Copyright (c) 1999, Bob Withers - bwit@pobox.com
//*
//* This code may be freely used for any purpose, either personal
//* or commercial, provided the authors copyright notice remains
//* intact.
//*********************************************************************

const char          fillchar = '=';

char* cvt = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+-";


int find(char c)
{
  int ans = 0;
  for(  char *ptr = cvt; *ptr!=c;ptr++)
    ans ++;
  return ans;
}

char* encodeB64(char* data, int len)
{
  int  i;
  int  c;
  char *ret = (char*) malloc(len * 2);
  char *ptr = ret;

  for (i = 0; i < len; ++i)
    {
        c = (data[i] >> 2) & 0x3f;
        *ret++ =  cvt[c];
        c = (data[i] << 4) & 0x3f;
        if (++i < len)
            c |= (data[i] >> 4) & 0x0f;

        *ret++ =  cvt[c];
        if (i < len)
          {
            c = (data[i] << 2) & 0x3f;
            if (++i < len)
              c |= (data[i] >> 6) & 0x03;

            *ret++ = cvt[c];
          }
        else
          {
            ++i;
            *ret++ = fillchar;
          }

        if (i < len)
          {
            c = data[i] & 0x3f;
            *ret++=cvt[c];
          }
        else
          {
            *ret++ =  fillchar;
          }
    }
  *ret = 0;

  return(ptr);
}

char* decodeB64(char* data, int len)
{
  int   i;
  char               c;
  char               c1;
  char*             ret = (char*)malloc(len);
  char* ptr = ret;
  for (i = 0; i < len; ++i)
    {
      c = (char) find(data[i]);
      ++i;
      c1 = (char) find(data[i]);
      c = (c << 2) | ((c1 >> 4) & 0x3);
      *ret++= c;
      if (++i < len)
        {
          c = data[i];
          if (fillchar == c)   break;

          c = (char) find(c);
          c1 = ((c1 << 4) & 0xf0) | ((c >> 2) & 0xf);
          *ret++ = c1;
        }

      if (++i < len)
        {
          c1 = data[i];
          if (fillchar == c1)
            break;

          c1 = (char)find(c1);
          c = ((c << 6) & 0xc0) | c1;
          *ret++ = c;
        }
    }

  return(ptr);
}

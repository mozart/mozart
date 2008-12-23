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

#ifndef __BASE64H
#define __BASE64H

//------------------------- Base 64 encoding --------------------------

//*********************************************************************
//* C_Base64 - a simple base64 encoder and decoder.
//*
//*     Copyright (c) 1999, Bob Withers - bwit@pobox.com
//*
//* This code may be freely used for any purpose, either personal
//* or commercial, provided the authors copyright notice remains
//* intact.
//*********************************************************************

char* encodeB64(char*,int);
char* decodeB64(char*, int);

#endif



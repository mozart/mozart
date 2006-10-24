/*
 *  Authors:
 *    Leif Kornstaedt <kornstae@ps.uni-sb.de>
 *
 *  Copyright:
 *    Leif Kornstaedt, 1999
 *
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 *
 *  This file is part of Mozart, an implementation of Oz 3:
 *    http://www.mozart-oz.org
 *
 *  See the file "LICENSE" or
 *    http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#include "builtins.hh"
#include "bytedata.hh"

#ifdef WINDOWS
#include "winreg.h"
#endif

OZ_Term AtomHKCR, AtomHKCC, AtomHKCU, AtomHKLM, AtomHKU, AtomHKPD, AtomHKDD;

void win32_init() {
  AtomHKCR = OZ_atom("HKEY_CLASSES_ROOT");
  AtomHKCC = OZ_atom("HKEY_CURRENT_CONFIG");
  AtomHKCU = OZ_atom("HKEY_CURRENT_USER");
  AtomHKLM = OZ_atom("HKEY_LOCAL_MACHINE");
  AtomHKU = OZ_atom("HKEY_USERS");
  AtomHKPD = OZ_atom("HKEY_PERFORMANCE_DATA");
  AtomHKDD = OZ_atom("HKEY_DYN_DATA");
}

#ifdef WINDOWS
OZ_BI_define(win32_getRegistryKey,3,1)
{
  OZ_declareAtom(0,rootKey0);
  OZ_declareVirtualString(1,subKey);
  OZ_declareVirtualString(2,valueName);

  HKEY root;
  OZ_Term rootKey = OZ_in(0);
  if (OZ_eq(rootKey, AtomHKCR))
    root = HKEY_CLASSES_ROOT;
  else if (OZ_eq(rootKey, AtomHKCC))
    root = HKEY_CURRENT_CONFIG;
  else if (OZ_eq(rootKey, AtomHKCU))
    root = HKEY_CURRENT_USER;
  else if (OZ_eq(rootKey, AtomHKLM))
    root = HKEY_LOCAL_MACHINE;
  else if (OZ_eq(rootKey, AtomHKU))
    root = HKEY_USERS;
  else if (OZ_eq(rootKey, AtomHKPD))
    root = HKEY_PERFORMANCE_DATA;
  else if (OZ_eq(rootKey, AtomHKDD))
    root = HKEY_DYN_DATA;
  else
    OZ_RETURN(OZ_false());

  HKEY hk;
  if (RegOpenKeyEx(root, subKey, 0, KEY_QUERY_VALUE, &hk) != ERROR_SUCCESS) {
    OZ_RETURN(OZ_false());
  }

  DWORD type;
  char buf[MAX_PATH];
  DWORD buf_size = MAX_PATH;
  OZ_Term res;
  if (RegQueryValueEx(hk, valueName, NULL, &type, (LPBYTE) buf, &buf_size)
      == ERROR_SUCCESS) {
    switch (type) {
    case REG_BINARY:
      {
	ByteString *s = new ByteString(buf_size);
	for (int i = 0; i < buf_size; i++)
	  s->getByte(i) = buf[i];
	res = makeTaggedExtension(s);
      }
      break;
    case REG_DWORD:
      res = OZ_int(*((DWORD *) buf));
      break;
    case REG_SZ:
      res = OZ_string(buf);
      break;
    case REG_EXPAND_SZ:
      {
	char buf2[MAX_PATH];
	DWORD n = ExpandEnvironmentStrings(buf, buf2, MAX_PATH);
	if (n == 0 || n == MAX_PATH) {
	  res = OZ_unit();
	} else {
	  res = OZ_string(buf2);
	}
      }
      break;
    default:
      res = OZ_unit();
      break;
    }
  } else {
    res = OZ_false();
  }
  RegCloseKey(hk);

  OZ_RETURN(res);
} OZ_BI_end
#endif


/*
 * The builtin table
 */

#ifndef MODULES_LINK_STATIC

#include "modWin32-if.cc"

#endif

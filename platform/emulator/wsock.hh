/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: $Author$
  Last modified: $Date$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

/* "windows.h" defines some constants, that are also used in Oz,
 * so this file MUST BE INCLUDED BEFORE ANY OTHER FILE
 */


#ifdef WINDOWS

#define NOMINMAX
#define Bool WinBool

#define NOGDI
#include <windows.h>
#undef FAILED /* used in oz.h as well */

#undef Bool

#endif

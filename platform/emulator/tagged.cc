/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: scheidhr
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  */

#ifdef __GNUC__
#pragma implementation "tagged.hh"
#endif

#include "tagged.hh"

char * TypeOfTermString[16] = {
  "REF", // 0
  "UVAR", // 1
  "LTUPLE", // 2
  "SMALLINT", //3
  "REF", // 4
  "CVAR", // 5
  "STUPLE", // 6
  "BIGINT", // 7
  "REF", // 8
  "SVAR", // 9
  "CONST", // 10
  "FLOAT", // 11
  "REF", // 12
  "UNUSEDVARIABLE", // 13
  "SRECORD", // 14
  "ATOM", // 15
};

/* -*- C++ -*- */
#include <stdio.h>
#include "config.h"

#define AMVersion OZVERSION
#define AMDate "HEREGOESDATE"


char *ozplatform = "OZPLATFORM";

void version()
{
  printf("Mozart playing Oz 3. Engine %s %s of %s.\n",
         AMVersion,ozplatform,AMDate);
  fflush(stdout);
}

#include <stdio.h>
#include "../include/config.h"

#define AMVersion OZVERSION
#define AMDate "HEREGOESDATE"

void version() {
  printf("DFKI Oz Emulator %s (OZPLATFORM) of %s\n",AMVersion,AMDate);
}

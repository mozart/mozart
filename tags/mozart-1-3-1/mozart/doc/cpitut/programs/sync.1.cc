#include <stdio.h>
#include "mozart_cpi.hh"

#define FailOnEmpty(X) if((X) == 0) goto failure;

OZ_BI_proto(test_sync);

#include "mozart.h"

static long n;

OZ_BI_define(counter_next,0,1)
{
  OZ_RETURN_INT(n++);
}
OZ_BI_end

OZ_C_proc_interface * oz_init_module(void)
{
  static OZ_C_proc_interface table[] = {
    {"next",0,1,counter_next},
    {0,0,0,0}
  };
  n = 1;
  return table;
}

OZ_BI_define(fd_init, 0, 0)
{
#ifndef NDEBUG
  printf("fd_start=0x%p\n", (void *) fd_start); 
  fflush(stdout);
#endif
  return PROCEED;
} 
OZ_BI_end


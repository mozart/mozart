OZ_C_proc_begin(fd_init, 0)
{
#ifndef NDEBUG
  printf("fd_start=0x%p\n", (void *) fd_start);
  fflush(stdout);
#endif
  return PROCEED;
}
OZ_C_proc_end

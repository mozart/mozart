OZ_C_proc_interface *oz_init_module(void)
{
  static OZ_C_proc_interface i_table[] = {
    {"connect", 2, 0, connect},
    {0, 0, 0, 0}
  };

  return i_table;
}

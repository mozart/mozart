
OZ_BI_proto(fd_add);

OZ_C_proc_interface *oz_init_module(void)
{
  static OZ_C_proc_interface i_table[] = {
    {"add", 3, 0, fd_add},
    {0,0,0,0}
  };

  AddProp::profile      = "addition/3";

  printf("addition propagator loaded\n");
  return i_table;
}


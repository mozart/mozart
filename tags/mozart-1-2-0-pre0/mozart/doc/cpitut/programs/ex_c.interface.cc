OZ_BI_proto(fd_add);
OZ_BI_proto(fd_twice);
OZ_BI_proto(fd_init);
OZ_BI_proto(fd_nestable);
OZ_BI_proto(fd_element);

OZ_C_proc_interface *oz_init_module(void)
{
  static OZ_C_proc_interface i_table[] = {
    {"init", 0, 0, fd_init},
    {"add", 3, 0, fd_add},
    {"twice", 2, 0, fd_twice},
    {"add_nestable", 3, 0, fd_add_nestable},
    {"element", 3, 0, fd_element},
    {0,0,0,0}
  };

  printf("propagators loaded\n");
  return i_table;
}

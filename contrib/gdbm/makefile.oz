makefile(
   lib   : ['gdbm.ozf' 'gdbm.so']
   rules :
      o(
	 'gdbm.so' : ld('gdbm.o' [library('gdbm')]))
   )
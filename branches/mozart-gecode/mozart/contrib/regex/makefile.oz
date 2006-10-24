makefile(
   lib : ['regex.ozf' 'regex.so']
   %% we need to check (1) if regex stuff can be obtained
   %% without any -l options, (2) other check that -lregex
   %% works
   rules :
      o('regex.so':ld('regex.o')
	'regex.o' :cc('regex.cc'['define'('HAVE_STRDUP')])
       )
   )

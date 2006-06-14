local INCS = [include('/home/denys/Mozart/build-1-2-0-fixes/platform/emulator')
	      include('/home/denys/Mozart/mozart-1-2-0-fixes/platform/emulator')]
in
makefile(
   lib : ['io.ozf' 'io.so' 'mode.ozf' 'open.ozf'
	  'process.ozf' 'process.so']
   rules :
      o(
	 'io.o'      : cc('io.cc'      INCS)
	 'process.o' : cc('process.cc' INCS)
	 )
   )
end

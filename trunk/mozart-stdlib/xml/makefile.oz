makefile(
   bin : ['demo.exe']
   lib : ['Tokenizer.ozf'
	  'Parser.ozf'
	  'NameSpaces.ozf'
	  'TEST.oz'
	  %'apptut.xml'
	 ]
   uri : 'x-oz://system/xml'
   mogul : 'mogul:/mozart/stdlib'
   )
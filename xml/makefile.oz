makefile(
   lib : ['Tokenizer.ozf'
	  'Parser.ozf'
	  'NameSpaces.ozf'
	  'SAX.ozf'
	  'TEST.oz'
	  %'apptut.xml'
	 ]
   uri : 'x-oz://system/xml'
   mogul : 'mogul:/mozart/stdlib'
   )
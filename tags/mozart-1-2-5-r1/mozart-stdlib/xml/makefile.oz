makefile(
    bin : ['ozmakedoc.exe']
   lib : ['Tokenizer.ozf'
	  'FastTokenizer.ozf'
	  'Parser.ozf'
	  'NameSpaces.ozf'
	  'SAX.ozf'
	  'NewParser.ozf'
	  'FastSAX.ozf'
	  'XSLTGenerator.ozf'
	 ]
   depends :
      o('ozmakedoc.exe':['Parser.ozf' 'NameSpaces.ozf'])
   uri : 'x-oz://system/xml'
   mogul : 'mogul:/mozart/stdlib'
   )
makefile(
   lib        : ['Parser.ozf'
		 'ParserAgain.ozf'
		 %%'FastParser.ozf' 'XSLTParser.ozf'
		]
   doc        : ['index.html' 'fig1.gif' 'fig2.gif' 'fig3.gif' 'example.xml']
   version    : '0.2'
   uri        : 'x-ozlib://duchier/xml'
   mogul      : 'mogul:/duchier/xml/parser'
   author     : 'mogul:/duchier/denys'
   blurb      : 'namespace aware XML parser'
   categories : ['xml']
   )
makefile(
   mogul  : 'mogul:/mozart/stdlib'
   uri    : 'x-oz://system'
   lib : ['String.ozf' 'Mapping.ozf' 'Generator.ozf']
   subdirs: [
	     %%'adt' 'os' 'op'
	     'adt'
	     'wp'
	     'xml'
	     'os'
	     'net'
	    ]
   doc    : [
	     'index.html'
	     'ozdoc.css'
	     'page.gif'
	    ]
   )

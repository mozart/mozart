makefile(
   mogul  : 'mogul:/mozart/stdlib'
   uri    : 'x-oz://system'
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

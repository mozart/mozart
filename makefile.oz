makefile(
   mogul  : 'mogul:/mozart/stdlib'
   uri    : 'x-oz://system'
   subdirs: [
	     %%'ds' 'os' 'op'
	     'wp'
	    ]
   doc    : [
	     'index.html'
	     'ozdoc.css'
	     'page.gif'
	    ]
   )

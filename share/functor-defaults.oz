%%%
%%% Authors:
%%%   Christian Schulte (schulte@dfki.de)
%%%
%%% Contributors:
%%%   Denys Duchier (duchier@ps.uni-sb.de)
%%%
%%% Copyright:
%%%   Christian Schulte, 1998
%%%   Denys Duchier, 1998
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    $MOZARTURL$
%%%
%%% See the file "LICENSE" or
%%%    $LICENSEURL$
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

defaults(dirs:
	    [lib tools]
	 lib:
	    ['Application' 'GetArgs' 'Applet' 'Syslet' 'Servlet'
	     'Search' 'FD' 'Schedule' 'FS'
	     'System'
	     'Error' 'ErrorRegistry' 'Debug' 'Finalize' 'Foreign'
	     'Fault' 'Connection' 'Remote' 'VirtualSite'
	     'Open'
	     'Tk' 'TkTools'
	     'Compiler'
	     'Misc']

	 tools:
	    ['Panel' 'Browser' 'Explorer' 'CompilerPanel'
	     'Emacs' 'Ozcar' 'Profiler' 'Gump' 'GumpScanner'
	     'GumpParser']

	 volatile:
	    ['Module' 'URL' 'OS' 'Property' 'Pickle']
	)

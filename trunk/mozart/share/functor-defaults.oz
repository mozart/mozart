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
%%%    http://mozart.ps.uni-sb.de
%%%
%%% See the file "LICENSE" or
%%%    http://mozart.ps.uni-sb.de/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

defaults(lib:
	    ['Application' 'GetArgs' 'Applet' 'Syslet' 'Servlet'
	     'Search' 'FD' 'Schedule' 'FS'
	     'Error' 'ErrorRegistry' 'Finalize' 'Foreign'
	     'Fault' 'Connection' 'Remote' 'VirtualSite' 'URL'
	     'Open'
	     'Tk' 'TkTools'
	     'Compiler'
	     'Type'
	     'Misc']

	 volatile:
	    ['Module' 'Resolve' 'OS' 'Property' 'Pickle' 'System']

	 tools:
	    ['Panel' 'Browser' 'Explorer' 'CompilerPanel'
	     'Emacs' 'Ozcar' 'Profiler' 'Gump' 'GumpScanner'
	     'GumpParser' 'ProductionTemplates']
	)

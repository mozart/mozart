%%%
%%% Authors:
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Leif Kornstaedt, 1997
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation of Oz 3:
%%%    $MOZARTURL$
%%%
%%% See the file "LICENSE" or
%%%    $LICENSEURL$
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor $

import
   System.{get
	   valueToVirtualString
	   Show = 'Show'}

   Error.{formatLine
	  msg}

   Foreign.{pointer
	   }

   FS.{value}

   Open.{file}

   Component.{Load = load
	      save}

   Tk

   TkTools

   Compiler.{genericInterface}

   Browser.{Browse = 'Browse'}

   Emacs.{condSend}

export
   panel: CompilerPanel

body

   \insert compilerPanel/CompilerPanelClass

end

%%%
%%% Authors:
%%%   Benjamin Lorenz (lorenz@ps.uni-sb.de)
%%%
%%% Contributor:
%%%   Christian Schulte
%%%
%%% Copyright:
%%%   Benjamin Lorenz, 1997
%%%   Christian Schulte, 1998
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

\ifdef LILO

functor $

import
   SP.{Show    = 'Show'
       Foreign = 'Foreign'
       Error   = 'Error'
       System  = 'System'
       Debug   = 'Debug'}

   CP.{FD = 'FD'
       FS = 'FS'}

   WP.{Tk      = 'Tk'
       TkTools = 'TkTools'}

   Browser.{Browse = 'Browse'}

   Emacs.{RealEmacs = 'Emacs'}

   Compiler.{RealCompiler = 'Compiler'}
   
export
   'Ozcar': Ozcar

body
   Emacs    = RealEmacs
   Compiler = RealCompiler
   
   \insert 'ozcar/config'
   \insert 'ozcar/prelude'

   \insert 'ozcar/tree'
   \insert 'ozcar/thread'
   \insert 'ozcar/stack'

   \insert 'ozcar/source'

   \insert 'ozcar/menu'
   \insert 'ozcar/dialog'
   \insert 'ozcar/help'
   \insert 'ozcar/gui'

   \insert 'ozcar/ozcar'
in
   skip
end

\else

fun instantiate {$ IMPORT}
   \insert 'SP.env'
      = IMPORT.'SP'
   \insert 'CP.env'
      = IMPORT.'CP'
   \insert 'WP.env'
      = IMPORT.'WP'
   \insert 'Browser.env'
      = IMPORT.'Browser'
   \insert 'Emacs.env'
      = IMPORT.'Emacs'
   \insert 'Compiler.env'
      = IMPORT.'Compiler'

   \insert 'ozcar/config'
   \insert 'ozcar/prelude'

   \insert 'ozcar/tree'
   \insert 'ozcar/thread'
   \insert 'ozcar/stack'

   \insert 'ozcar/source'

   \insert 'ozcar/menu'
   \insert 'ozcar/dialog'
   \insert 'ozcar/help'
   \insert 'ozcar/gui'

   \insert 'ozcar/ozcar'
in
   \insert 'Ozcar.env'
end

\endif


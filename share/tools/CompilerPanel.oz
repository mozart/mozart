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

\ifdef LILO

functor $

import
   SP.{System  = 'System'
       Error   = 'Error'
       Foreign = 'Foreign'
       Show    = 'Show'}

   CP.{FS = 'FS'}

   OP.{Open      = 'Open'
       Load      = 'Load'
       Component = 'Component'}

   WP.{Tk      = 'Tk'
       TkTools = 'TkTools'}

   Compiler.{RealCompiler = 'Compiler'}

   Browser.{Browse = 'Browse'}

   Emacs.{RealEmacs = 'Emacs'}

export
   'CompilerPanel': CompilerPanel

body
   Compiler = RealCompiler
   Emacs    = RealEmacs
in
   \insert compilerPanel/CompilerPanelClass

end

\else


fun instantiate {$ IMPORT}
   \insert 'SP.env'
   = IMPORT.'SP'
   \insert 'CP.env'
   = IMPORT.'CP'
   \insert 'OP.env'
   = IMPORT.'OP'
   \insert 'WP.env'
   = IMPORT.'WP'
   \insert 'Compiler.env'
   = IMPORT.'Compiler'
   \insert 'Browser.env'
   = IMPORT.'Browser'
   \insert 'Emacs.env'
   = IMPORT.'Emacs'
in
   local
      \insert compilerPanel/CompilerPanelClass
   in
      \insert 'CompilerPanel.env'
   end
end


\endif

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

functor prop once
import
   Application.save
   System.{valueToVirtualString show}
   Error.{formatLine msg}
   FS.value
   Open.file
   Pickle.{load save}
   Tk
   TkTools
   Compiler.genericInterface
   CompilerSupport.{isBuiltin} from 'x-oz-boot:CompilerSupport'
   Browser.browse
   Emacs.condSend
export
   panel:           CompilerPanel
   'CompilerPanel': CompilerPanel
body
   local
      UrlDefaults = \insert '../url-defaults.oz'
   in
      \insert compilerPanel/CompilerPanelClass
   end
end

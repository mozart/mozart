%%%
%%% Authors:
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%   Tobias Mueller <tmueller@ps.uni-sb.de>
%%%   Christian Schulte <schulte@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Tobias Mueller and Leif Kornstaedt, 1998
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation of Oz 3:
%%%    http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%    http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor
import
   OS(system tmpnam unlink stat)
   File(baseName changeExtension)
export
   'class': PostScriptToGIFClass
define

   fun {Exists Name}
      try
	 {OS.stat Name}=_ true
      catch _ then false
      end
   end
	 
	 
   proc {PsToPpm PsName PpmName}
      PsCat = if {Exists PsName} then
		 'cat '#PsName
	      else
		 'gzip -dc '#PsName#'.gz'
	      end
      Cmd   = ('('#PsCat#'; echo quit) | '#
	       'gs -q -dNOPAUSE '#
	       '-dTextAlphaBits=4 -dGraphicsAlphaBits=4 -r102 '#
	       '-sDEVICE=ppmraw -sOutputFile='#PpmName#' - 1>&2')
   in
      case {OS.system Cmd} of 0 then skip
      elseof I then
	 {Exception.raiseError ozDoc(gs PsName PpmName I)}
      end
   end

   proc {PpmToGif PpmName Info GifName} Cmd in
      Cmd  = ('pnmcrop < '#PpmName#' 2> /dev/null | '#
	      if Info == '' then ''
	      else 'pnmscale '#Info#' | '
	      end#
	      'ppmquant 256 2> /dev/null | '#
	      'ppmtogif -interlace -transparent rgbi:1/1/1 2> /dev/null > '#
	      GifName)
      case {OS.system Cmd} of 0 then skip
      elseof I then
	 {Exception.raiseError ozDoc(ppmtogif GifName GifName I)}
      end
   end

   class PostScriptToGIFClass
      attr
	 DirName: unit
	 Keep:    false
      meth init(Dir KeepPictures)
	 DirName <- Dir
	 Keep    <- KeepPictures
      end
      meth convertPostScript(InName Info ?OutName)
	 !OutName  = {File.changeExtension {File.baseName InName} '.ps' '.gif'}
	 FullName  = @DirName#'/'#OutName
      in
	 if {Not @Keep andthen {Exists FullName}} then
	    PpmName = {OS.tmpnam}
	 in
	    try
	       {PsToPpm InName PpmName}
	       {PpmToGif PpmName Info @DirName#'/'#OutName}
	    finally
	       try {OS.unlink PpmName} catch _ then skip end
	    end
	 end
      end
   end
end

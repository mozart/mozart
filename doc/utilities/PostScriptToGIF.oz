%%%
%%% Author:
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%   Tobias Mueller <tmueller@ps.uni-sb.de>
%%%
%%% Contributor:
%%%   Christian Schulte <schulte@dfki.de>
%%%
%%% Copyright:
%%%   Tobias Mueller and Leif Kornstaedt, 1998
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation of Oz 3:
%%%    http://mozart.ps.uni-sb.de
%%%
%%% See the file "LICENSE" or
%%%    http://mozart.ps.uni-sb.de/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor
import
   OS(system tmpnam)
export
   'class': PostScriptToGIFClass
define

   proc {PsToPpm PsName PpmName}
      Stat = {OS.system ('(cat '#PsName#'; echo quit) | '#
                         'gs -q -dNOPAUSE '#
                         '-dTextAlphaBits=4 -dGraphicsAlphaBits=4 -r102 '#
                         '-sDEVICE=ppmraw -sOutputFile='#PpmName#' - 1>&2')}
   in
      if Stat\=0 then
         {Exception.raiseError ozDoc(gs(PsName) Stat)}
      end
   end

   proc {PpmToGif PpmName Info GifName}
      Stat = {OS.system ('pnmcrop < '#PpmName#' 2>/dev/null | '#
                         if Info=='' then ''
                         else 'pnmscale '#Info#'  2>/dev/null | '
                         end #
                         'ppmtogif -interlace -transparent rgbi:1/1/1 2>/dev/null > '#GifName)}
   in
      if Stat\=0 then
         {Exception.raiseError ozDoc(ppmtogif(GifName) Stat)}
      end
   end

   class PostScriptToGIFClass
      attr
         DirName: unit

      meth init(Dir)
         DirName <- Dir
      end

      meth convertPostScript(InName Info $)
         OutName = @DirName#'/'#InName#'.gif'
         PpmName = {OS.tmpnam}
      in
         {PsToPpm InName PpmName}
         {PpmToGif PpmName Info OutName}
         OutName
      end
   end
end

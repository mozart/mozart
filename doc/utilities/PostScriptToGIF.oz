%%%
%%% Authors:
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
   OS(system tmpnam unlink)
   File(baseName changeExtension)
export
   'class': PostScriptToGIFClass
define
   proc {PsToPpm PsName PpmName} Cmd in
      Cmd = ('(cat '#PsName#'; echo quit) | '#
             'gs -q -dNOPAUSE '#
             '-dTextAlphaBits=4 -dGraphicsAlphaBits=4 -r102 '#
             '-sDEVICE=ppmraw -sOutputFile='#PpmName#' - 1>&2')
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
      attr DirName: unit
      meth init(Dir)
         DirName <- Dir
      end
      meth convertPostScript(InName Info ?OutName) PpmName in
         OutName = {File.changeExtension {File.baseName InName} '.ps' '.gif'}
         PpmName = {OS.tmpnam}
         try
            {PsToPpm InName PpmName}
            {PpmToGif PpmName Info @DirName#'/'#OutName}
         finally
            {OS.unlink PpmName}
         end
      end
   end
end

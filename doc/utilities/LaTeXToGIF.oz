%%%
%%% Author:
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Leif Kornstaedt, 1998
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
   OS(tmpnam system unlink)
   Open(file)
export
   'class': LaTeXToGIFClass
define
   LATEX2GIF = 'latex2gif'

   class LaTeXToGIFClass
      attr DirName: unit N: unit FileName: unit File: unit
      meth init(Dir)
         DirName <- Dir
         N <- 0
      end
      meth convertPicture(VS ?OutFileName)
         LaTeXToGIFClass, OpenLaTex()
         {@File write(vs: VS#'\n\\clearpage\n')}
         N <- @N + 1
         OutFileName = 'latex'#@N#'.gif'
      end
      meth convertMath(VS Display ?OutFileName)
         LaTeXToGIFClass, OpenLaTex()
         {@File write(vs: case Display of display then '\\[\n'#VS#'\n\\]\n'
                          [] inline then '$'#VS#'$\n'
                          end#'\\clearpage\n')}
         N <- @N + 1
         OutFileName = 'latex'#@N#'.gif'
      end
      meth OpenLaTex()
         case @File of unit then
            FileName <- {OS.tmpnam}
            File <- {New Open.file init(name: @FileName
                                        flags: [write create truncate])}
            {@File write(vs: ('\\documentclass{report}\n'#
                              '\\usepackage{wasysym,pstricks,pst-node}\n'#
                              '\\pagestyle{empty}\n'#
                              '\\begin{document}\n'))}
         else skip
         end
      end
      meth process(Reporter)
         case @File of unit then skip
         else
            {@File write(vs: '\\end{document}\n')}
            {@File close()}
            {Reporter startSubPhase('converting LaTeX sections to GIF')}
            try
               case
                  {OS.system LATEX2GIF#' '#@FileName#' '#@DirName}
               of 0 then skip
               elseof I then
                  {Exception.raiseError ozDoc(latexToGif I)}
               end
            finally
               {OS.unlink @FileName}
               File <- unit
               FileName <- unit
            end
         end
      end
   end
end

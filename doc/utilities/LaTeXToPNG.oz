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
   OS(tmpnam system unlink)
   Open(file)
   Gdbm at 'x-oz://contrib/gdbm'
   File(read: ReadFile write: WriteFile)
export
   'class': LaTeXToPNGClass
prepare

   local
      fun {Trim Is}
         case Is of nil then nil
         [] I|Ir then
            if {Char.isSpace I} then {Trim Ir} else Is end
         end
      end
   in
      fun {TrimVS V}
         S = {VirtualString.toString V}
      in
         {Reverse {Trim {Reverse {Trim S}}}}
      end
   end

define
   LATEX2PNG = 'latex2png'

   class LaTeXToPNGClass
      attr DB: unit Dict: unit Keys: unit DirName: unit N: unit
      meth init(Dir DBName)
         DB <- case DBName of unit then unit
               else
                  try
                     {Gdbm.new write(DBName)}
                  catch _ then
                     {Gdbm.new create(DBName)}
                  end
               end
         Dict <- {NewDictionary}
         Keys <- nil
         DirName <- Dir
         N <- 0
      end
      meth convertPicture(VS ?OutFileName)
         LaTeXToPNGClass, Enter(VS ?OutFileName)
      end
      meth convertMath(VS Display ?OutFileName)
         TVS = {TrimVS VS}
      in
         LaTeXToPNGClass, Enter(case Display of inline then
                                   '$'#TVS#'$\n'
                                [] display then
                                   '$$\n'#TVS#'\n$$\n'
                                end ?OutFileName)
      end
      meth Enter(VS OutFileName) A in
         A = {VirtualString.toAtom VS}
         case {Dictionary.condGet @Dict A unit} of unit then
            N <- @N + 1
            OutFileName = 'latex'#@N#'.png'
            {Dictionary.put @Dict A OutFileName}
            case @DB == unit orelse {Gdbm.condGet @DB A true} of true then
               Keys <- A#@N|@Keys
            elseof X then
               {WriteFile X @DirName#'/'#OutFileName}
            end
         elseof FileName then
            OutFileName = FileName
         end
      end
      meth process(Packages Inputs Reporter)
         case @Keys of nil then skip
         else FileName File Outs in
            {Reporter startSubPhase('converting LaTeX sections to PNG')}
            FileName = {OS.tmpnam}
            File = {New Open.file init(name: FileName
                                       flags: [write create truncate])}
            {File write(vs: '\\documentclass{report}\n')}
            {ForAll Packages
             proc {$ P}
                case P of X#Y then
                   {File write(vs: '\\usepackage['#Y#']{'#X#'}\n')}
                else
                   {File write(vs: '\\usepackage{'#P#'}\n')}
                end
             end}
            {File write(vs: ('\\usepackage{times}\n' #
                             '\\usepackage{mathptm}\n' #
                             '\\DeclareMathAlphabet{\\mathnormal}{OT1}{phv}{m}{sl}\n'))}
            {ForAll Inputs
             proc {$ I}
                {File write(vs: '\\input '#I#'\n')}
             end}
            {File write(vs: ('\\pagestyle{empty}\n'#
                             '\\begin{document}\n'#
                             '\\renewcommand{\\familydefault}{\\sfdefault}\n'#
                             '\\vsize=100cm\n\\textheight=\\vsize\n'))}
            Outs = {FoldR @Keys
                    fun {$ X#N In}
                       {File write(vs: X#'\n\\clearpage\n')}
                       In#' '#N
                    end ""}
            {File write(vs: '\\end{document}\n')}
            {File close()}
            try
               case {OS.system LATEX2PNG#' '#FileName#' '#@DirName#Outs}
               of 0 then skip
               elseof I then
                  {Exception.raiseError ozDoc(latexToPng I)}
               end
               if @DB \= unit then
                  {ForAll @Keys
                   proc {$ X#_}
                      {Gdbm.put @DB X
                       {ReadFile @DirName#'/'#{Dictionary.get @Dict X}}}
                   end}
               end
            finally
               try {OS.unlink FileName} catch _ then skip end
               if @DB \= unit then
                  {Gdbm.close @DB}
               end
            end
         end
      end
   end
end

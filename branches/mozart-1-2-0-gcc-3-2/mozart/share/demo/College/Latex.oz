%%%
%%% Authors:
%%%   Tobias Mueller <tmueller@ps.uni-sb.de>
%%%   Joerg Wuertz <wuertz@de.ibm.com>
%%%
%%% Copyright:
%%%   Tobias Mueller, 1998
%%%   Joerg Wuert, 1997
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%    http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor

export

   LaTeX

import

   Open
   OS
   
   Common(monday:          Monday
	  tuesday:         Tuesday
	  wednesday:       Wednesday
	  thursday:        Thursday
	  friday:          Friday
	  quartersPerDay:  QuartersPerDay
	  quartersPerHour: QuartersPerHour)

   
define
   
   %% maps starting time to LaTeX tabular entry in German
   fun {StartToVS Start}
      cond Start::Monday  then "Montag"
      [] Start::Tuesday   then "Dienstag"
      [] Start::Wednesday then "Mittwoch"
      [] Start::Thursday  then "Donnerstag"
      [] Start::Friday    then "Freitag"
      end
      #
      " & "
      #
      (((Start-1) mod (QuartersPerDay))+1) div QuartersPerHour + 8
      #
      " & Uhr & "
      #
      ((Start) mod QuartersPerHour) * 15
   end
   
   %% Ordering according to semesters
   local
      proc {TakeDigits String Front Back}
	 {List.takeDropWhile String Char.isDigit Front Back}
      end
      fun {Split S}
	 case S
	 of nil then nil
	 else Back in
	    case S.1
	    of &M then Back = S.2 10 
	    [] &F then Back = S.2 12
	    else {String.toInt {TakeDigits S $ Back}}
	    end
	    |{Split {List.dropWhile Back Char.isPunct}}
	 end
      end
      fun {Compare X1|X2|Xr Y1|Y2|Yr}
	 X1 < Y1 orelse
	 (X1 == Y1
	  andthen
	  (X2 < Y2 orelse (X2 == Y2 andthen (Xr == Yr orelse Xr.1 =< Yr.1))))
      end
   in
      fun {NameOrdering X Y}
	 {Compare {Split {AtomToString X.name}} {Split {AtomToString Y.name}}}
      end
   end

   %% Ordering according to start time
   fun {TimeOrdering X Y}
      X.start =< Y.start
   end

   %% Ordering according to professor
   fun {ProfessorOrdering X Y}
      X.professor =< Y.professor
   end

   %% Number of lines on page of document
   LinesOnPage = 30

   %% Begin of tabular environment
   BeginTab = "\\begin{tabular}{lllrlr}\n"

   %% End of tabular environment
   EndTab = "\\end{tabular}\n"

   %% LecturesToLaTeX transforms list of lectures to LaTeX format
   fun {LecturesToLaTeX Lectures}
      BeginTab
      #
      {FoldL Lectures
       fun {$ N#In Lecture}
	  N+1
	  #
	  (In
	   #if N mod LinesOnPage == 0
	    then EndTab # "\\newpage\n" # BeginTab
	    else ""
	    end
	   #Lecture.name
	   #" & "
	   #Lecture.professor
	   #" & "
	   #{StartToVS Lecture.start}
	   #"\\\\\n")
       end
       1#""}.2
      #
      EndTab
   end

   %% WriteSchedule writes a solution to file FileName
   proc {WriteSchedule Lectures FileName Ordering}
      OO = {New Open.file init(name:FileName
			       flags: [write 'create' truncate])}
   in
      {OO write(vs:{LecturesToLaTeX
		    {Sort
		     {List.flatten Lectures} Ordering}})}
      {OO close}
   end

   Name = {OS.tmpnam}
   SourceName = !Name#".tex"
   DVIName = !Name#".dvi"
   NameFileName = {OS.tmpnam}#".tex"
   ProfFileName = {OS.tmpnam}#".tex"
   TimeFileName = {OS.tmpnam}#".tex"

   LaTeXSource= '\\documentstyle[12pt]{article}\\begin{document}'
   #'\\section*{Vorlesungsplan der Katholischen Hochschule f\\"ur Soziale Arbeit, Saarbr\\"ucken}'
   #'\\subsection*{Nach Professoren alphabetisch geordnet}'
   #'\\input '#!ProfFileName
   #'\\newpage \\subsection*{Nach Vorlesungen geordnet}'
   #'\\input '#!NameFileName
   #'\\newpage \\subsection*{Nach Anfangszeiten geordnet}'
   #'\\input '#!TimeFileName
   #'\\end{document}' 

   local
      OO = {New Open.file init(name:SourceName
			       flags:[read write 'create'])}
   in
      {OO write(vs:LaTeXSource)}
   end
   
   %% Write computes the first solution
   %% and writes three different listings to files

   proc {LaTeX What Solution}
      {Wait Solution}
      {WriteSchedule Solution ProfFileName ProfessorOrdering}
      {WriteSchedule Solution TimeFileName TimeOrdering}
      {WriteSchedule Solution NameFileName NameOrdering}
      Flag={OS.system 'cd /tmp; latex '#SourceName}
   in 
      {Wait Flag}
      case What
      of show then {OS.system 'xdvi '#DVIName _}
      [] print then {OS.system 'dvips '#DVIName _}
      end
   end

end

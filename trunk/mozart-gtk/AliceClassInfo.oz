%%%
%%% Author:
%%%   Thorsten Brunklaus <brunklaus@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Thorsten Brunklaus, 2004
%%%
%%% Last Change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation of Oz 3:
%%%   http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%   http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%
functor $
import
   Application(exit)
   Open(file text)
   Pickle(load)
   Util at 'Util.ozf'
define
   class TextFile from Open.file Open.text end

   {List.forAll
    ["gdk"      #"GdkClassesFull.ozp"      #"GdkClasses.aml"
     "gtk"      #"GtkClassesFull.ozp"      #"GtkClasses.aml"
     "gtkCanvas"#"GtkCanvasClassesFull.ozp"#"GtkCanvasClasses.aml"]
    proc {$ Prefix#InFile#OutFile}
       OutObj    = {New TextFile init(name:  OutFile
				      flags: [write create truncate])}
       Classes   = {Pickle.load InFile}
       NbClasses = {Length Classes}
    in
       {OutObj putS("[")}
       {List.forAllInd
	Classes
	proc {$ I Class#Info}
	   NbMethods = {Length Info.methods}
	in
	   {OutObj putS(' {class="'#Class#'",')}
	   {OutObj putS('  anchestor="'#Info.anchestor#'",')}
	   case Info.methods
	   of nil then {OutObj putS('  methods=[]}')}
	   [] Methods then
	      {OutObj putS('  methods=[')}
	      {List.forAllInd Methods
	       proc {$ I RawName}
		  Name = {Util.firstLower
			  {Util.cutPrefix Prefix {Util.translateName RawName}}}
	       in
		  if I == NbMethods
		  then {OutObj putS('           "'#Name#'"]}')}
		  else {OutObj putS('           "'#Name#'", ')}
		  end
	       end}
	   end
	   if I == NbClasses
	   then {OutObj putS("]")}
	   else {OutObj putS(",")}
	   end
	end}
       {OutObj close}
    end}
   {Application.exit 0}
end

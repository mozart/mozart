%%%
%%% Author:
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%
%%% Contributors:
%%%   Tobias Mueller <tmueller@ps.uni-sb.de>
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
export
   empty: EMPTY
   seq: SEQ
   block: BLOCK
   common: COMMON
   pcdata: PCDATA
   verbatim: VERBATIM
   toVirtualString: HTMLToVirtualString
   Clean
prepare
   % names used for constructing the output tree:
   EMPTY    = '<EMPTY>'
   SEQ      = '<SEQ>'
   BLOCK    = '<BLOCK>'
   COMMON   = '<COMMON>'
   PCDATA   = '<PCDATA>'
   VERBATIM = '<VERBATIM>'
define
   fun {MakeCDATA S}
      '"'#S#'"'   %--** quotation marks etc.
   end

   fun {MakePCDATA S}
      case S of C|Cr then
	 case C of &< then &&|&l|&t|&;|{MakePCDATA Cr}
	 [] &> then &&|&g|&t|&;|{MakePCDATA Cr}
	 [] && then &&|&a|&m|&p|&;|{MakePCDATA Cr}
	 [] &" then &&|&q|&u|&o|&t|&;|{MakePCDATA Cr}
	 else C|{MakePCDATA Cr}
	 end
      [] nil then ""
      end
   end

   local
      fun {Difference Xs Ys}
	 case Xs of X1|Xr then
	    if {Member X1 Ys} then {Difference Xr Ys}
	    else X1|{Difference Xr Ys}
	    end
	 [] nil then Ys
	 end
      end
   in
      fun {NormalizeNode N} Common N0 N1 Cs1 Cs2 in
	 Common = {CondSelect N COMMON COMMON}
	 N0 = {Record.subtract N COMMON}
	 N1 = case {CondSelect Common id unit} of unit then N0
	      elseof ID then
		 if {HasFeature N0 id} then N0
		 else {AdjoinAt N0 id ID}
		 end
	      end
	 Cs1 = {CondSelect Common 'class' nil}
	 Cs2 = {CondSelect N1 'class' nil}
	 if Cs1 == nil andthen Cs2 == nil then N1
	 else {AdjoinAt N1 'class' {Append Cs2 {Difference Cs2 Cs1}}}
	 end
      end
   end

   IgnoreIfNoAttr = ignoreIfNoAttr('div': true span: true)
   NoEndTag = noEndTag(hr: true br: true link: true img: true)

   fun {ToVSSub HTML Start End} HTML1 in
      HTML1 = {NormalizeNode HTML}
      case HTML1 of PCDATA(VS) then {MakePCDATA {VirtualString.toString VS}}
      [] VERBATIM(VS) then VS
      [] SEQ(Xs) then
	 case Xs of X1|Xr then
	    {FoldL Xr
	     fun {$ VS X} VS#{ToVSSub X Start End} end
	     {ToVSSub X1 Start End}}
	 [] nil then ""
	 end
      [] !EMPTY then ""
      [] BLOCK(X) then End#{ToVSSub X Start End}#Start
      else Tag Attrs1 Attrs ThisStart ThisEnd NewStart NewEnd in
	 Tag = {Map {Atom.toString {Label HTML1}} Char.toUpper}
	 Attrs1 = {Record.foldLInd HTML1
		   fun {$ F In X}
		      if {IsInt F} orelse X == unit orelse F == id then In
		      else
			 case F of 'class' then
			    case X of X1|_ then
			       % The following does not work in all browsers:
			       % {FoldL Xr fun {$ In X} In#' '#X end X1}
			       In#' '#F#'='#{MakeCDATA X1}
			    [] nil then ""
			    end
			 else
			    In#' '#F#'='#{MakeCDATA X}
			 end
		      end
		   end ""}
	 Attrs = case {CondSelect HTML1 id unit} of unit then Attrs1
		 elseof X then Attrs1#' id='#{MakeCDATA X}
		 end
	 if Attrs1 == "" andthen {HasFeature IgnoreIfNoAttr {Label HTML1}} then
	    NewStart = ""
	    NewEnd = ""
	 else
	    NewStart = '<'#Tag#Attrs1#'>'
	    NewEnd = if {HasFeature NoEndTag {Label HTML1}} then ""
		     else '</'#Tag#'>'
		     end
	 end
	 if Attrs == "" andthen {HasFeature IgnoreIfNoAttr {Label HTML1}} then
	    ThisStart = ""
	    ThisEnd = ""
	 else
	    ThisStart = '<'#Tag#Attrs#'>'
	    ThisEnd = if {HasFeature NoEndTag {Label HTML1}} then ""
		      else '</'#Tag#'>'
		      end
	 end
	 {Record.foldLInd HTML1
	  fun {$ F In X}
	     if {IsInt F} then In#{ToVSSub X NewStart NewEnd}
	     else In
	     end
	  end ThisStart}#ThisEnd
      end
   end

   fun {HTMLToVirtualString HTML}
      {ToVSSub HTML unit unit}
   end

   fun {Clean HTML}
      case HTML of PCDATA(_) then HTML
      [] VERBATIM(_) then HTML
      [] SEQ(Xs) then SEQ({Map Xs Clean})
      [] !EMPTY then EMPTY
      [] BLOCK(X) then {Clean X}
      else
	 SEQ({Record.foldRInd HTML
	      fun {$ F X In}
		 if {IsInt F} then {Clean X}|In else In end
	      end nil})
      end
   end
end

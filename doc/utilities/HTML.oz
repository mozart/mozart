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
%%%   $MOZARTURL$
%%%
%%% See the file "LICENSE" or
%%%   $LICENSEURL$
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
define
   % names used for constructing the output tree:
   EMPTY    = {NewName}
   SEQ      = {NewName}
   BLOCK    = {NewName}
   COMMON   = {NewName}
   PCDATA   = {NewName}
   VERBATIM = {NewName}   %--** should be eliminated

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
      fun {Union Xs Ys}
         case Xs of X1|Xr then
            if {Member X1 Ys} then {Union Xr Ys}
            else {Union Xr X1|Ys}
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
         else {AdjoinAt N1 'class' {Union Cs1 Cs2}}
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
      [] BLOCK(X) then End#{HTMLToVirtualString X}#Start
      else Tag Attrs1 Attrs ThisStart ThisEnd NewStart NewEnd in
         Tag = {Map {Atom.toString {Label HTML1}} Char.toUpper}
         Attrs1 = {Record.foldLInd HTML1
                   fun {$ F In X}
                      if {IsInt F} orelse X == unit orelse F == id then In
                      else
                         In#' '#F#'='#
                         {MakeCDATA case F of 'class' then
                                       case X of X1|Xr then
                                          {FoldL Xr
                                           fun {$ In X} In#' '#X end X1}
                                       [] nil then X
                                       end
                                    else X
                                    end}
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
      {ToVSSub HTML unit unit}#'\n'
   end
end

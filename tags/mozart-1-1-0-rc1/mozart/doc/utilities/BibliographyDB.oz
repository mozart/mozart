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
   Property(get)
   OS(tmpnam putEnv system unlink)
   Open(text file)
export
   'class': BibliographyDBClass
prepare
   CommandsMap = f('AA': &Å 'aa': &å
		   'AE': &Æ 'ae': &æ
		   'O': &Ø 'o': &ø
		   'ss': &ß)

   local
      Accents = f(&À: "`A" &Á: "'A" &Â: "^A" &Ã: "~A" &Ä: "\"A"
		  &Ç: "cC"
		  &È: "`E" &É: "'E" &Ê: "^E" &Ë: "\"E"
		  &Ì: "`I" &Í: "'I" &Î: "^I" &Ï: "\"I"
		  &Ñ: "~N"
		  &Ò: "`O" &Ó: "'O" &Ô: "^O" &Õ: "~O" &Ö: "\"O"
		  &Ù: "`U" &Ú: "'U" &Û: "^U" &Ü: "\"U"
		  &Ý: "'Y"
		  &à: "`a" &á: "'a" &â: "^a" &ã: "~a" &ä: "\"a"
		  &ç: "cc"
		  &è: "`e" &é: "'e" &ê: "^e" &ë: "\"e"
		  &ì: "`i" &í: "'i" &î: "^i" &ï: "\"i"
		  &ñ: "~n"
		  &ò: "`o" &ó: "'o" &ô: "^o" &õ: "~o" &ö: "\"o"
		  &ù: "`u" &ú: "'u" &û: "^u" &ü: "\"u"
		  &ý: "'y" &ÿ: "\"y")

      D = {NewDictionary}
   in
      {Record.forAllInd Accents
       proc {$ C [C1 C2]} D1 in
	  if {Dictionary.member D C1} then
	     D1 = {Dictionary.get D C1}
	  else
	     D1 = {NewDictionary}
	     {Dictionary.put D C1 D1}
	  end
	  {Dictionary.put D1 C2 C}
       end}

      AccentsMap = {Record.mapInd {Dictionary.toRecord accentsMap D}
		    fun {$ C1 D1}
		       {Dictionary.toRecord {String.toAtom [C1]} D1}
		    end}
   end
define
   BIBTEX    = 'bibtex'

   class TextFile from Open.text Open.file
      prop final
   end

   fun {DotExpand S}
      case S of &.|&/|L then
	 {VirtualString.toString {Property.get 'ozdoc.src.dir'}#'/'#L}
      elseof &.|&.|&/|_ then
	 {VirtualString.toString {Property.get 'ozdoc.src.dir'}#'/'#S}
      else S end
   end

   fun {RemoveExtension S}
      case S of ".bib" then ""
      elsecase S of C|Cr then C|{RemoveExtension Cr}
      [] nil then ""
      end
   end

   fun {CopyBraceLevel S I}
      case S of C|Cr then
	 case C of &{ then C|{CopyBraceLevel Cr I + 1}
	 [] &} then
	    case I of 1 then {CleanLine Cr}
	    else C|{CopyBraceLevel Cr I - 1}
	    end
	 else C|{CopyBraceLevel Cr I}
	 end
      [] nil then ""   % do not worry about missing closing brace(s)
      end
   end

   fun {TranslateAccent C1 S}
      case {CondSelect AccentsMap C1 unit} of unit then
	 {Raise error} unit
      elseof CharMap then Rest in
	 Rest = {List.dropWhile S Char.isSpace}
	 case Rest of &\\|&i|Rest1 then
	    case {CondSelect CharMap &i unit} of unit then
	       {Raise error} unit
	    elseof NewC then NewC|{CopyBraceLevel Rest1 1}
	    end
	 elseof C2|Rest1 then
	    case {CondSelect CharMap C2 unit} of unit then
	       case Rest of &{|C3|&}|Rest2 then
		  case {CondSelect CharMap C3 unit} of unit then
		     {Raise error} unit
		  elseof NewC then NewC|{CopyBraceLevel Rest2 1}
		  end
	       elseof &{|&\\|&i|&}|Rest2 then
		  case {CondSelect CharMap &i unit} of unit then
		     {Raise error} unit
		  elseof NewC then NewC|{CopyBraceLevel Rest2 1}
		  end
	       else {Raise error} unit
	       end
	    elseof NewC then NewC|{CopyBraceLevel Rest1 1}
	    end
	 [] nil then {Raise error} unit
	 end
      end
   end

   fun {CleanLine S}
      case S of C|Cr then
	 case C of &{ then
	    case Cr of &\\|Rest then Command Rest1 in
	       {List.takeDropWhile Rest Char.isAlpha ?Command ?Rest1}
	       case Command of "" then
		  case Rest1 of C|Rest2 then
		     try {TranslateAccent C Rest2}
		     catch error then   %--** error: unrecognized accent
			{CopyBraceLevel Cr 1}
		     end
		  [] nil then S   % do not worry about missing closing brace
		  end
	       [] C|Rest2 then
		  case {CondSelect CommandsMap {String.toAtom Command} unit}
		  of unit then
		     case Rest2 of nil then
			try {TranslateAccent C Rest1}
			catch error then   %--** error: unrecognized accent
			   {CopyBraceLevel Rest 1}
			end
		     else
			{CopyBraceLevel Rest 1}
		     end
		  elseof NewC then NewC|{CopyBraceLevel Rest1 1}
		  end
	       end
	    else {CopyBraceLevel Cr 1}
	    end
	 [] &~ then &\240|{CleanLine Cr}
	 else C|{CleanLine Cr}
	 end
      [] nil then ""
      end
   end

   fun {ReadBib File Keys}
      case {File getS($)} of false then ""
      elseof S then
	 case S of &%|S1 then Key Text in
	    {List.takeDropWhile S1 fun {$ C} C \= &" end ?Key &"|?Text}
	    {Dictionary.get Keys {String.toAtom Key}} = {CleanLine Text}
	    {ReadBib File Keys}
	 else
	    {CleanLine S}#'\n'#{ReadBib File Keys}
	 end
      end
   end

   class BibliographyDBClass
      attr DirName: unit AuxFileName: unit AuxFile: unit Tos: unit Keys: unit
      meth init(Dir)
	 DirName <- Dir
      end
      meth get(To Key ?Text) AKey NTo in
	 AKey = {String.toAtom Key}
	 case @AuxFile of unit then
	    AuxFileName <- {OS.tmpnam}
	    AuxFile <- {New Open.file init(name: @AuxFileName#'.aux'
					   flags: [write create truncate])}
	    {@AuxFile write(vs: '\\bibstyle{html}\n')}
	    Tos <- nil
	    Keys <- {NewDictionary}
	 else skip
	 end
	 if {Dictionary.member @Keys AKey} then
	    {Dictionary.get @Keys AKey Text}
	 else
	    {Dictionary.put @Keys AKey Text}
	 end
	 NTo = {DotExpand {RemoveExtension To}}
	 if {Member NTo @Tos} then skip
	 else
	    Tos <- NTo|@Tos
	 end
	 {@AuxFile write(vs: '\\citation{'#Key#'}\n')}
      end
      meth process(Reporter ?VS)
	 case @AuxFile of unit then
	    VS = unit
	 else
	    {@AuxFile write(vs: {FoldLTail @Tos
				 fun {$ In To|Tor}
				    case Tor of nil then In#To
				    else In#To#','
				    end
				 end '\\bibdata{'}#'}\n')}
	    {@AuxFile close()}
	    {OS.putEnv 'BSTINPUTS' {Property.get 'ozdoc.bst.path'}}
	    {OS.putEnv 'BIBINPUTS' {Property.get 'ozdoc.bib.path'}}
	    case {OS.system BIBTEX#' '#@AuxFileName} of 0 then File in
	       File = {New TextFile init(name: @AuxFileName#'.bbl'
					 flags: [read])}
	       VS = {ReadBib File @Keys}
	       {File close()}
	       try {OS.unlink @AuxFileName#'.bbl'} catch _ then skip end
	       try {OS.unlink @AuxFileName#'.blg'} catch _ then skip end
	    elseof I then
	       {Reporter error(kind: 'bibliography database'
			       msg: 'bibtex failed'
			       items: [hint(l: 'Exit code' m: I)])}
	       VS = ""
	    end
	    try {OS.unlink @AuxFileName#'.aux'} catch _ then skip end
	    {ForAll {Dictionary.entries @Keys}
	     proc {$ Key#Text}
		if {IsFree Text} then   % compensate for unknown entries
		   Text = Key
		end
	     end}
	 end
      end
   end
end

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
%%%   $MOZARTURL$
%%%
%%% See the file "LICENSE" or
%%%   $LICENSEURL$
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor prop once
import
   Property(get)
   OS(tmpnam putEnv system unlink)
   Open(text file)
export
   'class': BibliographyDBClass
define
   BIBTEX = 'bibtex'
   BSTINPUTS = {Property.get 'oz.home'}#'/share/doc/'

   class TextFile from Open.text Open.file
      prop final
   end

   fun {RemoveExtension S}
      case S of ".bib" then ""
      elsecase S of C|Cr then C|{RemoveExtension Cr}
      [] nil then ""
      end
   end

   fun {ReadBib File Keys}
      case {File getS($)} of false then ""
      elseof S then
         case S of &%|S1 then Key Text in
            {List.takeDropWhile S1 fun {$ C} C \= &: end ?Key &:|?Text}
            {Dictionary.get Keys {String.toAtom Key}} = Text
            {ReadBib File Keys}
         else
            S#'\n'#{ReadBib File Keys}
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
         NTo = {RemoveExtension To}
         if {Member NTo @Tos} then skip
         else
            Tos <- NTo|@Tos
         end
         {@AuxFile write(vs: '\\citation{'#Key#'}\n')}
      end
      meth process(?VS)
         case @AuxFile of unit then
            VS = unit
         else File in
            {@AuxFile write(vs: {FoldLTail @Tos
                                 fun {$ In To|Tor}
                                    case Tor of nil then In#To
                                    else In#To#','
                                    end
                                 end '\\bibdata{'}#'}\n')}
            {@AuxFile close()}
            {OS.putEnv 'BSTINPUTS' BSTINPUTS}
            case {OS.system BIBTEX#' '#@AuxFileName} of 0 then skip
            elseof I then
               {Exception.raiseError ozDoc(bibtex I)}
            end
            File = {New TextFile init(name: @AuxFileName#'.bbl'
                                      flags: [read])}
            VS = {ReadBib File @Keys}
            {ForAll {Dictionary.entries @Keys}
             proc {$ Key#Text}
                if {IsFree Text} then   % compensate for unknown entries
                   Text = Key
                end
             end}
            {File close()}
            {OS.unlink @AuxFileName#'.aux'}
            {OS.unlink @AuxFileName#'.bbl'}
            {OS.unlink @AuxFileName#'.blg'}
         end
      end
   end
end

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
   OS(tmpnam system unlink)
   Open(file text)
export
   'class': FontifierClass
   NoProgLang
define
   NoProgLang = {NewName}

   local
      %% -- PATH and OZDOC_ELISP_PATH have already been augmented
      FONTIFY = 'fontify'

      fun {NotIsEOF C}
         C \= 4
      end

      class TextFile from Open.file Open.text
         prop final
         attr Buffered: unit
         meth readResult(NL ?Res) Res0 Rest in
            case @Buffered of unit then
               Buffered <- TextFile, getS($)
            else skip
            end
            {List.takeDropWhile @Buffered NotIsEOF ?Res0 ?Rest}
            case Rest of "" then Res1 in
               Buffered <- unit
               Res = Res0#NL#Res1
               TextFile, readResult(NL Res1)
            elseof 4|Rest2 then
               Buffered <- Rest2
               Res = Res0
            end
         end
      end

      fun {DoRead File}
         case {File getS($)} of false then ""
         elseof S then
            if {File atEnd($)} then S
            else S#'\n'#{DoRead File}
            end
         end
      end

      proc {ReadFile FileName ?Res}
         Dir = {Property.get 'ozdoc.src.dir'}
         File
      in
         File = {New TextFile init(name: Dir#'/'#FileName flags: [read])}
         Res = {DoRead File}
         {File close()}
      end

      fun {ProgLangToMode ProgLang}
         case ProgLang of oz then 'oz-mode'
         [] gump then 'oz-gump-mode'
         [] c then 'c-mode'
         [] cc then 'c++-mode'
         [] elisp then 'emacs-lisp-mode'
         [] !NoProgLang then 'fundamental-mode'
         else
            %--** warn about unsupported programming language
            'fundamental-mode'
         end
      end
   in
      class FontifierClass
         attr Hd: unit Tl: unit
         meth init() X in
            Hd <- X
            Tl <- X
         end
         meth enqueueFile(ProgLang FileName NL ?Res) NewTl in
            @Tl = file(ProgLang FileName NL Res)|NewTl
            Tl <- NewTl
         end
         meth enqueueVirtualString(ProgLang VS NL ?Res) NewTl in
            @Tl = virtualString(ProgLang VS NL Res)|NewTl
            Tl <- NewTl
         end
         meth process(OutputType)
            InFileName InFile OutFileName Command OutFile X
         in
            @Tl = nil
            InFileName = {OS.tmpnam}
            InFile = {New Open.file init(name: InFileName
                                         flags: [write create truncate])}
            {ForAll @Hd
             proc {$ Task}
                case Task of file(ProgLang FileName _ _) then Mode in
                   Mode = {ProgLangToMode ProgLang}
                   {InFile write(vs: Mode#[4]#{ReadFile FileName}#[4])}
                [] virtualString(ProgLang VS _ _) then Mode in
                   Mode = {ProgLangToMode ProgLang}
                   {InFile write(vs: Mode#[4]#VS#[4])}
                end
             end}
            {InFile close()}
            OutFileName = {OS.tmpnam}
            Command = FONTIFY#' '#' '#OutputType#' '#InFileName#' '#OutFileName
            case {OS.system Command} of 0 then skip
            elseof I then
               {Exception.raiseError ozDoc(fontifier I)}
            end
            {OS.unlink InFileName}
            OutFile = {New TextFile init(name: OutFileName flags: [read])}
            {ForAll @Hd
             proc {$ Task} {OutFile readResult(Task.3 ?Task.4)} end}
            {OutFile close()}
            {OS.unlink OutFileName}
            Hd <- X
            Tl <- X
         end
      end
   end
end

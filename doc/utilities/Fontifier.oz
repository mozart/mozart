%%%
%%% Authors:
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%   Denys Duchier <duchier@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Leif Kornstaedt, 1998
%%%   Denys Duchier, 1998
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
   Fontifier at 'x-oz://contrib/doc/code/Fontifier'
   HTML(seq: SEQ pcdata: PCDATA verbatim: VERBATIM)
   URL(toVirtualString)
   Resolve(localize pickle handler make)
   OS(unlink)
export
   'class': FontifierClass
   NoProgLang
define
   NoProgLang = {NewName}
   {Fontifier.loadpath.set
    {ByNeedFuture
     fun {$}
        {String.tokens
         {VirtualString.toString {Property.get 'ozdoc.elisp.path'}} &:}
     end}}

   FontifierBase = Fontifier.'class'

   class FontifierClass from FontifierBase
      attr q:nil meta:unit resolver:unit
      meth init(Meta)
         FontifierBase,init
         meta <- Meta
         resolver <- {Resolve.make ozdoc init({Resolve.pickle.getHandlers})}
         {@resolver.addHandler back({Resolve.handler.root {Property.get 'ozdoc.src.dir'}})}
         for D in {Property.get 'ozdoc.include'} do
            {@resolver.addHandler back({Resolve.handler.root D})}
         end
      end
      meth enqueueFile(ProgLang FileName Result)
         lock R
            FILE = {@resolver.localize FileName}
            case FILE of new(PATH) then
               thread {Wait Result} {OS.unlink PATH} end
            else skip end
            PATH = {URL.toVirtualString FILE.1}
         in
            q <- (R#Result)|@q
            FontifierBase,enqueueFile(
                             {self toMode(ProgLang $)} PATH R)
         end
      end
      meth enqueueVirtualString(ProgLang VS Result)
         lock R in
            q <- (R#Result)|@q
            FontifierBase,enqueueVirtualString(
                             {self toMode(ProgLang $)} VS R)
         end
      end
      meth enqueueRequest(ProgLang Result)
         Request = {MakeRequest Result}
      in
         lock
            q <- (Request#Result)|@q
            FontifierBase,enqueueRequest(
                             {self toMode(ProgLang $)} Request)
         end
      end
      meth process(Type)
         L Transform = case Type
                       of 'html-color'       then ToHtmlColor
                       [] 'html-mono'        then ToHtmlMono
                       [] 'html-stylesheets' then ToHtmlCSS
                       end
      in
         lock
            L1 = {Fontifier.requires.get}
            L2 = {Dictionary.condGet @meta 'emacs.package' nil}
         in
            try
               {Fontifier.requires.set {Append L1 L2}}
               L=@q q<-nil FontifierBase,synck
            finally
               {Fontifier.requires.set L1}
            end
         end
         {ForAll L proc {$ R#Result} {Transform R Result} end}
      end
      meth toMode(ProgLang $)
         case {Lookup ProgLang
               {Dictionary.condGet @meta 'proglang.mode' unit}}
         of unit then {ToMode ProgLang}
         elseof Mode then Mode end
      end
   end

   fun {MakeRequest Req}
      case Req
      of simple(In _) then simple(In _)
      [] complex(L)   then complex({Map L MakeRequest})
      end
   end

   fun {Lookup Key Alist}
      case Alist of H|T then
         case H of K#V then
            if K==Key then V else {Lookup Key T} end
         else {Lookup Key T} end
      else unit end
   end

   fun {ToMode ProgLang}
      case  ProgLang
      of oz          then 'oz-mode'
      [] gump        then 'oz-gump-mode'
      [] c           then 'c-mode'
      [] cc          then 'c++-mode'
      [] elisp       then 'emacs-lisp-mode'
      [] sh          then 'sh-mode'
      [] !NoProgLang then 'fundamental-mode'
      else                ProgLang
      end
   end

   local
      SPACE = VERBATIM('&nbsp;')
      TAB = VERBATIM('&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;')

      fun {HtmlEscapeSub S} S1 Rest in
         {List.takeDropWhile S fun {$ C}
                                  C \= &\n andthen C \= &  andthen C \= &\t
                               end ?S1 ?Rest}
         case Rest of C|Rest1 then
            case C of &\n then
               %% hack to overcome netscape's bug that
               %% <BR><BR> only causes one line break
               %% but <BR>&nbsp;<BR> works as expected
               if S1==nil then SPACE else PCDATA(S1) end
               |br()|{HtmlEscapeSub Rest1}
            [] &  then PCDATA(S1)|SPACE|{HtmlEscapeSub Rest1}
            [] &\t then PCDATA(S1)|TAB|{HtmlEscapeSub Rest1}
            end
         else
            [PCDATA(S1)]
         end
      end
   in
      fun {HtmlEscape S}
         SEQ({HtmlEscapeSub S})
      end
   end

   fun {Face2Color Face}
      case Face
      of comment   then '#B22222'
      [] keyword   then '#A020F0'
      [] string    then '#BC8F8F'
      [] function  then '#0000FF'
      [] type      then '#228B22'
      [] variable  then '#B8860B'
      [] reference then '#5F9EA0'
      else '#000000' end
   end

   fun {Face2Font Face}
      case Face
      of comment  then 'i'
      [] keyword  then 'b'
      [] function then 'u'
      [] type     then 'u'
      else unit end
   end

   fun {Face2Class Face}
      case Face
      of comment   then comment
      [] keyword   then keyword
      [] string    then string
      [] function  then functionname
      [] type      then type
      [] variable  then variablename
      [] reference then reference
      [] builtin   then builtin
      else unit end
   end

   fun {ToHtmlCSS1 Face#Text}
      C = {Face2Class Face}
   in
      if C==unit then {HtmlEscape Text}
      else span('class':[C] 1:{HtmlEscape Text}) end
   end

   fun {ToHtmlCSS L}
      case L
      of simple(S R) then simple(S {ToHtmlCSS R})
      [] complex(L)  then complex({Map L ToHtmlCSS})
      else SEQ({Map L ToHtmlCSS1}) end
   end

   fun {ToHtmlColor1 Face#Text}
      font(color:{Face2Color Face} 1:{HtmlEscape Text})
   end

   fun {ToHtmlColor L}
      case L
      of simple(S R) then simple(S {ToHtmlColor R})
      [] complex(L)  then complex({Map L ToHtmlColor})
      else SEQ({Map L ToHtmlColor1}) end
   end

   fun {ToHtmlMono1 Face#Text}
      F = {Face2Font Face}
   in if F==unit then {HtmlEscape Text}
      else F({HtmlEscape Text}) end
   end

   fun {ToHtmlMono L}
      case L
      of simple(S R) then simple(S {ToHtmlMono R})
      [] complex(L)  then complex({Map L ToHtmlMono})
      else SEQ({Map L ToHtmlMono1}) end
   end

end

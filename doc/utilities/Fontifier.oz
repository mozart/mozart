%%%
%%% Author:
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
%%%   $MOZARTURL$
%%%
%%% See the file "LICENSE" or
%%%   $LICENSEURL$
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor
import
   Property(get)
   Fontifier @ 'x-oz://contrib/doc/code/Fontifier'
export
   'class': FontifierClass
   NoProgLang
define
   NoProgLang = {NewName}
   {Fontifier.loadpath.set
    {ByNeed
     fun {$}
        {String.tokens
         {VirtualString.toString {Property.get 'ozdoc.elisp.path'}} &:}
     end}}

   FontifierBase = Fontifier.'class'

   class FontifierClass from FontifierBase
      attr q:nil
      meth enqueueFile(ProgLang FileName Result)
         lock R DIR={Property.get 'ozdoc.src.dir'} in
            q <- (R#Result)|@q
            FontifierBase,enqueueFile(
                             {ToMode ProgLang} DIR#'/'#FileName R)
         end
      end
      meth enqueueVirtualString(ProgLang VS Result)
         lock R in
            q <- (R#Result)|@q
            FontifierBase,enqueueVirtualString(
                             {ToMode ProgLang} VS R)
         end
      end
      meth process(Type)
         L Transform = case Type
                       of 'html-color'       then ToHtmlColor
                       [] 'html-mono'        then ToHtmlMono
                       [] 'html-stylesheets' then ToHtmlCSS
                       end
      in
         lock L=@q q<-nil FontifierBase,synck end
         {ForAll L proc {$ R#Result} {Transform R Result} end}
      end
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
      else
         %%--** warn about unsupported programming language
         'fundamental-mode'
      end
   end

   fun {HtmlEscape L}
      case L of nil then nil
      [] H|T then
         case H
         of &< then &&|&l|&t|&;|{HtmlEscape T}
         [] &> then &&|&g|&t|&;|{HtmlEscape T}
         [] && then &&|&a|&m|&p|&;|{HtmlEscape T}
         else H|{HtmlEscape T} end
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
      of comment  then 'I'
      [] keyword  then 'B'
      [] function then 'U'
      [] type     then 'U'
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
      else unit end
   end

   fun {ToHtmlCSS1 Face#Text L}
      C = {Face2Class Face}
   in
      if C==unit then {HtmlEscape Text}
      else '<SPAN CLASS="'#C#'">'#{HtmlEscape Text}#'</SPAN>' end
      #L
   end

   fun {ToHtmlCSS L} {FoldR L ToHtmlCSS1 nil} end

   fun {ToHtmlColor1 Face#Text L}
      '<FONT color="'#{Face2Color Face}#'">'#{HtmlEscape Text}#'</FONT>'
      #L
   end

   fun {ToHtmlColor L} {FoldR L ToHtmlColor1 nil} end

   fun {ToHtmlMono1 Face#Text L}
      F = {Face2Font Face}
   in if F==unit then {HtmlEscape Text}
      else '<'#F#'>'#{HtmlEscape Text}#'</'#F#'>' end
      #L
   end

   fun {ToHtmlMono L} {FoldR L ToHtmlMono1 nil} end

end

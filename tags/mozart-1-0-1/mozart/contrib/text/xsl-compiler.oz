%%% Copyright (c) by Denys Duchier, Sep 1998, Universität des Saarlandes
%%%
%%% given a parsed xsl stylesheet document, the compiler returns a
%%% record stylesheet(id:I space:S constants:C templates:T)
%%%
%%% I:	a list of pairs A#E where A is an atom naming an attribute and
%%% E is either unit or an atom naming an element type.  This alist
%%% indicates which attributes should be considered ID attributes.
%%%
%%% S:	a list where each element is either strip(E) or preserve(E),
%%% where E is an atom naming an element type.
%%%
%%% O:	a list of pairs C#V where C is an atom naming a constant and V
%%% is the string that is assigned as its value.
%%%
%%% T:	a list of template(M L) where M is a parsed pattern and L is a
%%% list of the content of the template.
%%%
%%% elements have one of the forms:
%%%
%%%	xsl(type:T attribute:A content:C)
%%%	elt(type:T attribute:A content:C)
%%%
%%% T is an atom, A is a record of attributes and C is a list of
%%% the element's content. For xsl elements, A maps attribute names
%%% to preprocessed attribute values; for elt elements, A maps
%%% attribute names to attribute nodes of the form
%%% attribute(name:NAME value:VALUE) where VALUE is either a string
%%% or a expr(L) where L is a list of elements data(S), where S is a
%%% string, and expr(E), where is a parsed string expression.
%%% 
functor
import
   XSL('class' : XSL_Attribute_Parser) at 'xsl-attribute.ozf'
export
   'class' : XSL_Compiler
define
   %% the preprocessor changes the label of xsl element to `xsl'
   %% and of other elements to `elt'
   local
      fun {DoAll L} {Map L DoOne} end
      fun {DoOne X}
	 case X of element(type:T attribute:A content:C) then
	    if {List.isPrefix "xsl:" {Atom.toString T}} then
	       xsl(type:T attribute:A content:{DoAll C})
	    else
	       elt(type:T attribute:A content:{DoAll C})
	    end
	 [] pi(_)    then X
	 [] cdata(_) then X end
      end
   in
      fun {Preprocess Root}
	 case Root of root(L) then root({DoAll L}) end
      end
   end
   fun {IsId A E L}
      case L of nil then false
      [] H|T then
	 (H.1==A andthen (H.2==unit orelse H.2==E)) orelse {IsId A E T}
      end
   end
   class XSL_Data
      attr id space constant template
      meth init
	 id<-nil space<-nil constant<-nil template<-nil
      end
      meth putId(Att Elt) %Elt can be unit if it applies to all elts
	 id<-(Att#Elt)|@id
      end
      meth getId($) @id end
      meth putStripSpace(E)    space<-strip(E)   |@space end
      meth putPreserveSpace(E) space<-preserve(E)|@space end
      meth getSpace($) {Reverse @space} end
      meth putConstant(N V) constant<-(N#V)|@constant end
      meth getConstant($) @constant end
      meth putTemplate(T) template<-T|@template end
      meth getTemplate($) @template end
   end
   IsTopLevel =
   o('xsl:import'               : true
     'xsl:include'              : true
     'xsl:id'                   : true
     'xsl:strip-space'          : true
     'xsl:preserve-space'       : true
     'xsl:define-macro'         : true
     'xsl:define-attribute-set' : true
     'xsl:define-constant'      : true
     'xsl:template'             : true)
   IsTemplate =
   o('xsl:process-children'     : true
     'xsl:process'              : true
     'xsl:text'                 : true
     'xsl:value-of'             : true
     'xsl:if'                   : true
     'xsl:choose'               : true
     'xsl:number'               : true
     'xsl:invoke'               : true)
   fun {IsPCDATA X}
      case X of cdata(_) then true
      [] pi(_) then true
      else false end
   end
   fun {IsSpaces X}
      case X of cdata(S) then {All S Char.isSpace}
      else false end
   end
   fun {DropSpaces L}
      case L of nil then nil
      [] H|T then
	 if {IsSpaces H} then {DropSpaces T}
	 else H|{DropSpaces T} end
      end
   end
   fun {AttributeCondGet Elt Att Def}
      A = {CondSelect Elt.attribute Att unit}
   in if A==unit then Def else A.value end end
   fun {AttributeGet Elt Att}
      Elt.attribute.Att.value
   end
   fun {SplitAttributeValue S}
      try {SplitValue S nil}
      catch bad then
	 raise xsl(illegalAttributeValueTemplate S) end
      end
   end
   fun {SplitValue S L}
      case S of nil then
	 if L==nil then nil else [data({Reverse L})] end
      [] H|T then
	 if H==&{ then
	    if L==nil then {SplitValueAux T nil}
	    else data({Reverse L})|{SplitValueAux T nil} end
	 else {SplitValue T H|L} end
      end
   end
   fun {SplitValueAux S L}
      case S of nil then raise bad end
      [] H|T then
	 if H==&} then expr({Reverse L})|{SplitValue T nil}
	 else {SplitValueAux T H|L} end
      end
   end
   class XSL_Compiler
      prop locking
      attr data parser
      meth init parser<-{New XSL_Attribute_Parser init} end
      meth process(Root $)
	 lock D = {New XSL_Data init} in
	    data <- D
	    {self doRoot({Preprocess Root})}
	    data <- unit
	    stylesheet(
	       id	: {D getId($)}
	       space	: {D getSpace($)}
	       constants: {D getConstant($)}
	       templates: {D getTemplate($)})
	 end
      end
      meth doRoot(Root)
	 case Root of root(L) then
	    {ForAll L proc {$ X} {self DoTop(X)} end}
	 end
      end
      meth DoTop(X)
	 if {IsSpaces X} then skip
	 elsecase X of xsl(type:'xsl:stylesheet' ...) then
	    {self 'xsl:stylesheet'(X)}
	 else raise xsl(illegalAtTopLevel X) end end
      end
      meth 'xsl:stylesheet'(X)
	 {ForAll X.content
	  proc {$ E} {self topSS(E)} end}
      end
      meth topSS(X)
	 if {IsSpaces X} then skip
	 elsecase X of xsl(type:T ...) then
	    if {CondSelect IsTopLevel T false} then
	       {self T(X)}
	    else
	       raise xsl(illegalInStylesheetElement X) end
	    end
	 else raise xsl(illegalInStylesheetElement X) end
	 end
      end
      meth 'xsl:import'(X)
	 raise xsl(notImplemented X) end
      end
      meth 'xsl:include'(X)
	 raise xsl(notImplemented X) end
      end
      meth noContent(X)
	 if {CondSelect X content nil}\=nil then
	    raise xsl(expectedNoContent X) end
	 end
      end
      meth 'xsl:id'(X)
	 {self noContent(X)}
	 A = {AttributeGet X attribute}
	 E = {AttributeCondGet X element unit}
      in
	 {self putId(A E)}
      end
      meth putId(A E) {@data putId(A E)} end
      meth 'xsl:strip-space'(X)
	 {self noContent(X)}
	 E = {AttributeGet X element}
      in
	 {self putStripSpace(E)}
      end
      meth putStripSpace(E) {@data putStripSpace(E)} end
      meth 'xsl:preserve-space'(X)
	 {self noContent(X)}
	 E = {AttributeGet X element}
      in
	 {self putPreserveSpace(E)}
      end
      meth putPreserveSpace(E) {@data putPreserveSpace(E)} end
      meth 'xsl:define-macro'(X)
	 raise xsl(notImplemented X) end
      end
      meth 'xsl:define-attribute-set'(X)
	 raise xsl(notImplemented X) end
      end
      meth 'xsl:define-constant'(X)
	 {self noContent(X)}
	 N = {String.toAtom {AttributeGet X name}}
	 V = {AttributeGet X value}
      in
	 {self putConstant(N V)}
      end
      meth putConstant(N V) {@data putConstant(N V)} end
      meth 'xsl:template'(X)
	 M = {self parsePattern({AttributeGet X match} $)}
	 L = {self doContent(X.content $)}
      in
	 {self putTemplate(template(M L))}
      end
      meth putTemplate(T) {@data putTemplate(T)} end
      meth parsePattern(VS P)
	 {@parser parsePattern(VS P)}
      end
      meth doContent(L $)
	 case L of nil then nil
	 [] H|T then
	    if {IsSpaces H} then {self doContent(T $)}
	    else {self doTemplate(H $)}|{self doContent(T $)} end
	 end
      end
      meth doTemplate(X $)
	 case X of pi(_) then X
	 [] cdata(_) then X
	 [] xsl(type:T attribute:_ content:_) then
	    if {CondSelect IsTemplate T false} then
	       {self T(X $)}
	    else
	       raise xsl(illegalInTemplate X) end
	    end
	 [] elt(type:_ attribute:_ content:_) then
	    {self doElement(X $)}
	 end
      end
      meth doElement(X $)
	 As = {Record.map X.attribute
	       fun {$ V} {self doAttributeValue(V $)} end}
      in
	 elt(type      : X.type
	     attribute : As
	     content   : {self doContent(X.content $)})
      end
      meth doAttributeValue(V $)
	 if {Member &{ V} then
	    template({Map {SplitAttributeValue V}
		      fun {$ X}
			 case X of data(_) then X
			 elseof expr(E) then
			    expr({self parseStringExpression(E $)})
			 end
		      end})
	 else V end
      end
      meth parseStringExpression(E $)
	 {@parser parseStringExpression(E $)}
      end
      meth 'xsl:process-children'(X $)
	 {self noContent(X)}
      in X end
      meth 'xsl:process'(X $)
	 {self noContent(X)}
	 S = {self parsePattern({AttributeGet X select} $)}
      in xsl(type:'xsl:process' attribute:o(select:S) content:nil) end
      meth 'xsl:text'(X $)
	 if {All X.content IsPCDATA} then X else
	    raise xsl(hasIllegalContent X) end
	 end
      end
      meth 'xsl:value-of'(X $)
	 {self noContent(X)}
	 E = {self parseStringExpression({AttributeGet X expr} $)}
      in xsl(type:'xsl:value-of' attribute:o(expr:E) content:nil) end
      meth 'xsl:if'(X $)
	 T = {self parsePattern({AttributeGet X test} $)}
      in
	 xsl(type:'xsl:if' attribute:o(test:T)
	     content:{self doContent(X.content $)})
      end
      meth 'xsl:choose'(X $)
	 xsl(type:'xsl:choose' attribute:o
	     content : {self doChoose({DropSpaces X.content} $)})
      end
      meth doChoose(L $)
	 case L of nil then nil
	 [] H|T then
	    case H of xsl(type:'xsl:when'    ...) then
	       {self doWhen(H $)}|{self doChoose(T $)}
	    elseof    xsl(type:'xsl:otherwise' ...) then
	       if T==nil then
		  [xsl(type:'xsl:otherwise' attribute:o
		       content:{self doContent(H.content)})]
	       else raise xsl(otherwiseNotLast H) end end
	    else raise xsl(illegalInChoose H) end end
	 end
      end
      meth doWhen(X $)
	 T = {self parsePattern({AttributeGet X test} $)}
      in
	 xsl(type:'xsl:when' attribute:o(test:T)
	     content:{self doContent(X.content $)})
      end
      meth 'xsl:number'(X $)
	 {self noContent(X)}
	 L = {String.toAtom {AttributeCondGet X level "single"}}
	 if L==single orelse L==multi orelse L==any then skip
	 else raise xsl(illegalLevel X) end end
	 C0 = {AttributeCondGet X count unit}
	 C = if C0==unit then unit else {self parsePattern(C0 $)} end
	 F0 = {AttributeCondGet X 'from' unit}
	 F = if F0==unit then unit else {self parsePattern(F0 $)} end
      in xsl(type:'xsl:number'
	     attribute:o(level:L count:C 'from':F) content:nil)
      end
      meth 'xsl:invoke'(X $)
	 raise xsl(notImplemented X) end
      end
   end
end
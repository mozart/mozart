%%% reads a stylesheet from a file and returns a representation for it:
%%%
%%% stylesheet(id	: ID
%%%            space	: SPACE
%%%            constants: CONSTANTS
%%%            templates: TEMPLATES)
%%%
%%% ID is a list of pairs A#E where A is an atom naming an attribute and
%%% E is either unit or an atom naming an element type.  This list
%%% indicates which attributes should be considered ID attributes.
%%%
%%% SPACE is a list where each element is either strip(E) or preserve(E),
%%% where E is an atom naming an element type.  It indicates what to do
%%% with chunks of whitespace.
%%%
%%% CONSTANTS is a list of pairs C#V where C is an atom naming a constant
%%% and V is the string that is assigned as its value.
%%%
%%% TEMPLATES is a list of template elements.
%%%
functor
import
   XML('class')
   Parse(pattern expression) at 'XSL-ATTRIBUTE.ozf'
export
   ParseFile
define
   fun {ParseFile File}
      {Internalize {PreProcess {FromFile File}}}
   end
   %%
   Parser = {ByNeed fun {$} {New XML.'class' init} end}
   fun {FromFile File} {Parser parseFile(File $)} end
   %%
   %% {PreProcess XmlTree XslTree}
   %%
   %% convert an XML tree to an XSL tree, where XSL elements
   %% are labeled with `xsl' and other elements are labeled
   %% with `elt'.  All elements have features `type',
   %% `attribute' and `content'.
   %%
   fun {PreProcess Node}
      case Node of element(type:T attribute:A content:C) then
	 if {List.isPrefix "xsl:" {Atom.toString T}} then
	    xsl(type	  : T
		attribute : {Record.map A PreProcess}
		content   : {Map C PreProcess})
	 else
	    elt(type	  : T
		attribute : {Record.map A PreProcess}
		content   : {Map C PreProcess})
	 end
      elseof root(L) then root({Map L PreProcess})
      elseof attribute(name:_ value:V) then V
      else Node end
   end
   %%
   %% {Internalize Root Record}
   %%
   %% converts an XSL tree stylesheet to an internal representation
   %%
   fun {Internalize Root}
      AccId    = {NewCell nil}
      AccSpace = {NewCell nil}
      AccConst = {NewCell nil}
      AccTempl = {NewCell nil}
      proc {DoRoot Root}
	 case Root of root(L) then {ForAll L DoTop} end
      end
      proc {DoTop Top}
	 if {IsSpaces Top} then skip
	 elsecase Top of xsl(type:'xsl:stylesheet' ...) then
	    {DoStyleSheet Top}
	 else raise xsl(illegalAtTopLevel Top) end end
      end
      proc {DoStyleSheet Top}
	 {ForAll Top.content DoSSTop}
      end
      proc {DoSSTop E}
	 if {IsSpaces E} then skip
	 elsecase E of xsl(type:T ...) then
	    P = {CondSelect StyleSheetTopLevel T unit}
	 in
	    if P==unit then
	       raise xsl(illegalAtSSTopLevel E) end
	    else {P E} end
	 else  raise xsl(illegalAtSSTopLevel E) end end
      end
      StyleSheetTopLevel =
      o(
	 'xsl:import'			: DoImport
	 'xsl:include'			: DoInclude
	 'xsl:id'			: DoId
	 'xsl:strip-space'		: DoStripSpace
	 'xsl:preserve-space'		: DoPreserveSpace
	 'xsl:define-macro'		: DoDefineMacro
	 'xsl:define-attribute-set'	: DoDefineAttributeSet
	 'xsl:define-constant'		: DoDefineConstant
	 'xsl:template'			: DoTemplate
	 )
      proc {DoImport  E} raise xsl(notImplemented E) end end
      proc {DoInclude E} raise xsl(notImplemented E) end end
      proc {NoContent E}
	 if {CondSelect E 'content' nil}\=nil then
	    raise xsl(expectedNoContent E) end
	 end
      end
      proc {DoId E}
	 {NoContent E}
	 Att = {AttributeGet E 'attribute'}
	 Elt = {AttributeCondGet E 'element' unit}
	 L
      in
	 {Exchange AccId L (Att#Elt)|L}
      end
      proc {DoStripSpace E}
	 {NoContent E}
	 Elt = {AttributeGet E 'element'}
	 L
      in
	 {Exchange AccSpace L strip(Elt)|L}
      end
      proc {DoPreserveSpace E}
	 {NoContent E}
	 Elt = {AttributeGet E 'element'}
	 L
      in
	 {Exchange AccSpace L preserve(Elt)|L}
      end
      proc {DoDefineMacro E} raise xsl(notImplemented E) end end
      proc {DoDefineAttributeSet E} raise xsl(notImplemented E) end end
      proc {DoDefineConstant E}
	 {NoContent E}
	 N = {String.toAtom {AttributeGet E 'name'}}
	 V = {AttributeGet E 'value'}
	 L
      in
	 {Exchange AccConst L (N#V)|L}
      end
      proc {DoTemplate E} L in
	 {Exchange AccTempl L
	  xsl(type      : 'xsl:template'
	      attribute : o(match:{Parse.pattern {AttributeGet E 'match'}})
	      content   : {DoTemplateBatch E.content})
	  |L}
      end
      fun {DoTemplateBatch L}
	 case L of nil then nil
	 [] H|T then
	    if {IsSpaces H} then {DoTemplateBatch T}
	    else
	       case H of xsl(type:T ...) then
		  case T of
		     'xsl:number'           then {DoNumber          H}
		  [] 'xsl:text'             then {DoText            H}
		  [] 'xsl:process-children' then {DoProcessChildren H}
		  [] 'xsl:process'          then {DoProcess         H}
		  [] 'xsl:for-each'         then {DoForEach         H}
		  [] 'xsl:if'               then {DoIf              H}
		  [] 'xsl:choose'           then {DoChoose          H}
		  [] 'xsl:value-of'         then {DoValueOf         H}
		  [] 'xsl:invoke'           then {DoInvoke          H}
		  else raise xsl(unknown H) end end
	       elseof elt(type:T attribute:A content:C) then
		  elt(type      : T
		      attribute : {Record.mapInd A DoTemplateAttribute}
		      content   : {DoTemplateBatch C})
	       else H end
	       |{DoTemplateBatch T}
	    end
	 end
      end
      fun {DoNumber E}
	 {NoContent E}
	 L = {String.toAtom {AttributeCondGet E 'level' "single"}}
	 if L=='single' orelse L=='multi' orelse L=='any' then skip
	 else raise xsl(illegalAttribute E 'level') end end
	 C = case {AttributeCondGet E 'count' unit} of unit then unit
	     elseof S then {Parse.pattern S} end
	 F = case {AttributeCondGet E 'from'  unit} of unit then unit
	     elseof S then {Parse.pattern S} end
      in
	 xsl(type	: 'xsl:number'
	     attribute	: o(level:L count:C 'from':F)
	     content	: nil)
      end
      fun {DoText E}
	 xsl(type	: 'xsl:text'
	     attribute	: o
	     content	: E.content)
      end
      fun {DoProcessChildren E}
	 {NoContent E}
      in
	 xsl(type	: 'xsl:process-children'
	     attribute	: o
	     content	: nil)
      end
      fun {DoProcess E}
	 {NoContent E}
      in
	 xsl(type	: 'xsl:process'
	     attribute	: o(select:{Parse.pattern {AttributeGet E select}})
	     content	: nil)
      end
      fun {DoForEach E}
	 xsl(type	: 'xsl:for-each'
	     attribute	: o(select:{Parse.pattern {AttributeGet E select}})
	     content	: {DoTemplateBatch E.content})
      end
      fun {DoIf E}
	 xsl(type	: 'xsl:if'
	     attribute	: o(test:{Parse.pattern {AttributeGet E select}})
	     content	: {DoTemplateBatch E.content})
      end
      fun {DoChoose E}
	 xsl(type	: 'xsl:choose'
	     attribute	: o
	     content	: {DoChooseBatch E.content})
      end
      fun {DoChooseBatch L}
	 case L of nil then nil
	 [] H|T then
	    case H.type
	    of 'xsl:otherwise' then
	       if T\=nil then raise xsl(illegalChoose L) end else
		  [xsl(type	: 'xsl:otherwise'
		       attribute: o
		       content	: {DoTemplateBatch H.content})]
	       end
	    [] 'xsl:when'      then
	       xsl(type		: 'xsl:when'
		   attribute	: o(test:{Parse.pattern
					  {AttributeGet H select}})
		   content	: {DoTemplateBatch H.content})
	       |{DoChooseBatch T}
	    end
	 end
      end
      fun {DoValueOf E}
	 {NoContent E}
	 xsl(type	: 'xsl:value-of'
	     attribute	: o(expr:{Parse.expression {AttributeGet E expr}})
	     content	: nil)
      end
      fun {DoInvoke E} raise xsl(notImplemented E) end end
      fun {DoTemplateAttribute A Value}
	 try {DoAttributeValue Value}
	 catch bad then raise xsl(attributeValueTemplate A#Value) end end
      end
   in
      {DoRoot Root}
      stylesheet(id		: {Reverse {Access AccId}}
		 space		: {Reverse {Access AccSpace}}
		 constants	: {Reverse {Access AccConst}}
		 templates	: {Reverse {Access AccTempl}})
   end
   CharIsSpace = Char.isSpace
   fun {IsSpaces X}
      case X of cdata(S) then {All S CharIsSpace} else false end
   end
   fun {AttributeGet E A}
      V = {CondSelect E.attribute A unit}
   in
      if V==unit then raise xsl(attributeGet E A) end else V end
   end
   fun {AttributeCondGet E A D}
      {CondSelect E.attribute A D}
   end
   TakeDropWhile = List.takeDropWhile
   fun {NotLBrace C} C==&{ end
   fun {NotRBrace C} C==&} end
   fun {DoAttributeValue S}
      if S==nil then nil else S1 S2 in
	 {TakeDropWhile S NotLBrace S1 S2}
	 if S1==nil then {DoAttributeExpr S2}
	 else string(S1)|{DoAttributeExpr S2} end
      end
   end
   fun {DoAttributeExpr S}
      case S of nil then nil
      elseof &{|S then S1 S2 in
	 {TakeDropWhile S NotRBrace S1 S2}
	 case S2 of &}|S2 then
	    eval({Parse.expression S1})|{DoAttributeValue S2}
	 else raise bad end end
      else raise bad end end
   end
end

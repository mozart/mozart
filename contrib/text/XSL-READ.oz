%%% read a stylesheet and build a representation for it
functor
import
   XML
   Parse(pattern expression) at 'XSL-ATTRIBUTE.ozf'
define
   Parser = {ByNeed fun {$} {New XML init} end}
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
            xsl(type:T attribute:A content:{DoAll C})
         else
            elt(type:T attribute:A content:{DoAll C})
         end
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
         else raise(illegalAtTopLevel Top) end end
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
         'xsl:import'                   : DoImport
         'xsl:include'                  : DoInclude
         'xsl:id'                       : DoId
         'xsl:strip-space'              : DoStripSpace
         'xsl:preserve-space'           : DoPreserveSpace
         'xsl:define-macro'             : DoDefineMacro
         'xsl:define-attribute-set'     : DoDefineAttributeSet
         'xsl:define-constant'          : DoDefineConstant
         'xsl:template'                 : DoTemplate
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
         A = {AttributeGet E 'attribute'}
         E = {AttributeCondGet E 'element' unit}
         L
      in
         {Exchange AccId L (A#E)|L}
      end
      proc {DoStripSpace E}
         {NoContent E}
         E = {AttributeGet E 'element'}
         L
      in
         {Exchange AccSpace L strip(E)|L}
      end
      proc {DoPreserveSpace E}
         {NoContent E}
         E = {AttributeGet E 'element'}
         L
      in
         {Exchange AccSpace L preserve(E)|L}
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
      proc {DoTemplate E}
         xsl(type      : 'xsl:template'
             attribute : o(match:{Parse.pattern {AttributeGet X 'match'}})
             content   : {DoTemplateBatch E.contents})
      end
      proc {DoTemplateBatch L}
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
                      attribute : {Record.map A DoTemplateAttribute}
                      content   : {DoTemplateBatch C})
               else H end
               |{DoTemplateBatch T}
            end
         end
      end
      proc {DoNumber E}
         {NoContent E}
         L = {String.toAtom {AttributeCondGet X 'level' "single"}}
         if L=='single' orelse L=='multi' orelse L=='any' then skip
         else raise xsl(illegalAttribute E 'level') end end
         C = case {AttributeCondGet E 'count' unit} of unit then unit
             elseof S then {Parse.pattern S} end
         F = case {AttributeCondGet E 'from'  unit} of unit then unit
             elseof S then {Parse.pattern S} end
      in
         xsl(type      : 'xsl:number'
             attribute : o(level:L count:C 'from':F)
             content   : nil)
      end
   in

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
end

functor
import
   SGML(isOfClass) at 'x-oz://contrib/text/SGML'
define
   fun {Match Spec Stack}
      case Spec of nil then true
         %% -- tag name
      elseof tag(T)|Spec then
         case Stack of Node|_ then
            T==Node.tag andthen
            {Match Spec Stack}
         else false end
         %% -- existence of attribute
      elseof attribute(A)|Spec then
         case Stack of Node|_ then
            {HasFeature Node.attributes A} andthen
            {Match Spec Stack}
         else false end
         %% -- attribute value
      elseof attribute(A V)|Spec then
         case Stack of Node|_ then
            {HasFeature Node.attributes A} andthen
            V==Node.attributes.A andthen
            {Match Spec Stack}
         else false end
         %% -- element class
      elseof 'class'(...)=X|Spec then
         case Stack of Node|_ then
            {Record.all X
             fun {$ C} {SGML.isOfClass Node C} end}
            andthen {Match Spec Stack}
         else false end
         %% -- proceed matching on parent
      elseof '/'|Spec then
         case Stack of _|Stack then
            {Match Spec Stack}
         else false end
         %% -- proceed matching on some ancestor+
      elseof '//'|Spec then
         try
            {List.forAllTail Stack
             proc {$ _|Stack}
                if {Match Spec Stack} then
                   raise ok end
                end
             end}
            false
         catch ok then true end
      elseof
         %% -- Tag is treated like tag(Tag)
      elseof Tag|Spec andthen {IsAtom Tag} then
         case Stack of Node|_ then
            Tag==Node.tag andthen
            {Match Spec Stack}
         else false end
      end
   end
end

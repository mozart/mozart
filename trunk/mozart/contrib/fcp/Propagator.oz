functor

export
   Is
   IsDiscarded
   Discard
   GetName
   GetParameter
   IdentifyParameter

import
   Reflect

define

   Is                = Reflect.isPropagator
   IsDiscarded       = Reflect.isDiscardedPropagator
   Discard           = Reflect.discardPropagator
   GetName           = Reflect.propName
   GetParameter      = fun {$ P} {Reflect.propReflect P}.params end
   IdentifyParameter = Reflect.identifyParameter
end

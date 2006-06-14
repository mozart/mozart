functor

export

   Is
   IsDiscarded
   Discard
   IsActive
   Activate
   Deactivate
   GetName
   GetParameter
   IdentifyParameter

import

   Reflect

define

   Is                = Reflect.isPropagator
   IsDiscarded       = Reflect.isDiscardedPropagator
   Discard           = Reflect.discardPropagator
   IsActive          = Reflect.isActivePropagator
   Activate          = Reflect.activatePropagator
   Deactivate        = Reflect.deactivatePropagator
   GetName           = Reflect.propName
   GetParameter      = fun {$ P} {Reflect.propReflect P}.params end
   IdentifyParameter = Reflect.identifyParameter

end

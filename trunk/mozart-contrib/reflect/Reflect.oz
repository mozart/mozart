functor

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
export
   VarReflect
   VarEq     
   PropReflect
   PropEq     
   PropName
   PropLocation

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
import
   ReflectExport at 'reflect.so{native}'
   System
   
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
define
   VarReflect   = ReflectExport.variableReflect
   VarEq        = System.eq
   PropReflect  = ReflectExport.propagatorReflect
   PropEq       = ReflectExport.propagatorEq
   PropName     = ReflectExport.propagatorName
   PropLocation = ReflectExport.propagatorCoordinates

end

functor
import
   Celloid(new:New is:Is access:Access assign:Assign)
   at 'native-celloid.so{native}'
   Error ErrorRegistry
export
   New Is Access Assign
define
   fun {CelloidFormatter Exc}
      E = {Error.dispatch Exc}
      T = 'Celloid Error'
   in
      case E of celloid(nonLocal C V) then
         {Error.format T
          'Attempted assign on non local celloid'
          [hint(l:'Operation' m:'Celloid.assign')
           hint(l:'Celloid'   m:oz(C))
           hint(l:'Value'     m:oz(V))]
          Exc}
      else
         {Error.formatGeneric T Exc}
      end
   end
   {ErrorRegistry.put celloid CelloidFormatter}
end

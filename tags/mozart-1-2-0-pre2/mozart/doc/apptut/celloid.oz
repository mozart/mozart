functor
import
   Celloid(new:New is:Is access:Access assign:Assign)
   at 'native-celloid.so{native}'
   Error(registerFormatter)
export
   New Is Access Assign
define
   fun {CelloidFormatter E}
      T = 'Celloid Error'
   in
      case E of celloid(nonLocal C V) then
	 error(kind: T
	       msg: 'Attempted assign on non local celloid'
	       items: [hint(l:'Operation' m:'Celloid.assign')
		       hint(l:'Celloid'   m:oz(C))
		       hint(l:'Value'     m:oz(V))])
      else
	 error(kind: T
	       items: [line(oz(E))])
      end
   end
   {Error.registerFormatter celloid CelloidFormatter}
end

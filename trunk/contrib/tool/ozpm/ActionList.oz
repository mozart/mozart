functor
import
   Global(ozpmInfo)
   System(showInfo:Print)
   Application(exit)
export
   Run
define
   proc {Run}
      for I in {Map Global.ozpmInfo fun {$ R} R.id end} do
	 {Print I}
      end
      {Application.exit 0}
   end
end

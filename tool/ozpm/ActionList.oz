functor
import
   Global(localDB)
   System(showInfo:Print)
   Application(exit)
export
   Run
define
   proc {Run}
      for I in {Map Global.localDB fun {$ R} R.id end} do
	 {Print I}
      end
      {Application.exit 0}
   end
end

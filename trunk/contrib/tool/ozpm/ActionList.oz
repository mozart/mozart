functor
import
   Global(localDB)
   System(showInfo:Print)
   Application(exit)
export
   Run
define
   proc {Run}
      for I in {Global.localDB keys($)} do
	 {Print I}
      end
      {Application.exit 0}
   end
end

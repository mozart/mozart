functor
import
   Global(readDB)
   System(showInfo:Print)
   Application(exit)
export
   Run
define
   proc {Run}
      for I in {Map {Global.readDB} fun {$ R} R.id end} do
	 {Print I}
      end
      {Application.exit 0}
   end
end

functor
import Application
define
   Args = {Application.getCmdArgs
	   record(size( single type:int optional:false)
		  times(single type:int optional:false))}
   proc {FirstPiggy}
      {List.make Args.size _}
      {For 1 Args.times 1 SecondPiggy}
   end
   proc {SecondPiggy _}
      {List.make Args.size _}
      {For 1 Args.times 1 ThirdPiggy}
   end
   proc {ThirdPiggy _}
      {List.make Args.size _}
   end
   {FirstPiggy}
   {Application.exit 0}
end

functor
import
   CNT(new:NEW is:Is get:Get set:Set next:Next free:Free)
   at 'counter-obj.so{native}'
   Finalize(register)
export
   New Is Get Set Next
define
   proc {New I C}
      {NEW I C}
      {Finalize.register C Free}
   end
end
functor
import
   Admin(manager:Manager)
export
   Raise
define
   proc {Raise E}
      {Manager trace('Error: '#{Value.toVirtualString E 100 100})}
      raise E end
   end
end
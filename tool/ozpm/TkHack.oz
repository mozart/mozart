functor
require
   BootObject at 'x-oz://boot/Object'
   BootName   at 'x-oz://boot/Name'
   Tk(frame)
prepare
   TkClass =
   {List.last
    {Arity
     {BootObject.getClass
      {New class $ from Tk.frame meth init skip end end init}}
     . {BootName.newUnique 'ooFeat'}}}
export
   TkClass
end

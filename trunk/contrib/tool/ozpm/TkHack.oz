functor
require
   BootObject at 'x-oz://boot/Object'
   BootName   at 'x-oz://boot/Name'
prepare
   GetClass = BootObject.getClass
   OoFeat   = {BootName.newUnique 'ooFeat'}
import
   Tk(frame)
export
   TkClass
define
   TkClass =
   {List.last
    {Arity
     {GetClass
      {New class $ from Tk.frame meth init skip end end init}}
     . OoFeat}}
end

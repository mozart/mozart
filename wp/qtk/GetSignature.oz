functor

import
   BootName at 'x-oz://boot/Name'
   BootObject at 'x-oz://boot/Object'

export
   GetObjInfo
   GetClassInfo
   
define

   NewUniqueName=BootName.newUnique
   GetClass=BootObject.getClass

   `ooMeth`={NewUniqueName 'ooMeth'}
   `ooFeat`={NewUniqueName 'ooFeat'}

   fun{GetObjInfo Obj}
      C={GetClass Obj}
   in
      r('meth':{Dictionary.keys C.`ooMeth`}
	'feat':{Arity C.`ooFeat`})
   end

   Dummy={NewName}
   
   fun{GetClassInfo Class}
      Obj={New
	   class $
	      from Class
	      meth !Dummy
		 skip
	      end
	   end
	   Dummy}
      OI={GetObjInfo Obj}
   in
      r('meth':{List.filter OI.'meth' fun{$ V} V\=Dummy end}
	'feat':{List.toRecord 'feat'
		{List.map OI.'feat'
		 fun{$ F} F#Obj.F end}}
       )
   end
end
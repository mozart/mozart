functor
export
   CondGet
import
   Property
define
   local
      fun {Loop LGot LWant}
	 case LWant of HWant|TWant then
	    case LGot of HGot|TGot then
	       if     HGot > HWant then true
	       elseif HGot < HWant then false
	       else {Loop TGot TWant} end
	    else false end
	 else true end
      end
   in
      fun {VersionIsAtLeast S}
	 LGot  = {Map {String.tokens {VirtualString.toString {Property.get 'oz.version'}} &.} StringToInt}
	 LWant = {Map {String.tokens {VirtualString.toString S} &.} StringToInt}
      in
	 {Loop LGot LWant}
      end
   end
   Table = {NewDictionary}
   fun {CondGet K D} {CondSelect Table K D} end
   Table.gumpdir := false%{VersionIsAtLeast '1.2.6'}
end

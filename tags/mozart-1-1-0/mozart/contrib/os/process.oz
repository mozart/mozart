functor
import
   PROC(
      is	: IS
      make	: MAKE
      dropDead	: DROPDEAD
      status	: STATUS
      kill	: KILL
      ) at 'process.so{native}'
   Finalize
export
   is		: IS
   make		: Make
   status	: Status
   kill		: KILL
define

   fun {IsIntPair X}
      case X of I#J then
	 {IsInt I} andthen {IsInt J} andthen I>=0 andthen J>=0
      else false end
   end

   fun {Make CMD ARGS IOMAP}
      if {IsVirtualString CMD} andthen
	 {IsList ARGS} andthen
	 {All ARGS IsVirtualString} andthen
	 {IsList IOMAP} andthen
	 {All IOMAP IsIntPair}
      then {MAKE CMD ARGS IOMAP}
      else raise process(make CMD ARGS IOMAP) end end
   end

   fun {Status P} !!{STATUS P} end

   {Finalize.everyGC DROPDEAD}

end

functor
import
   PROC(
      is        : IS
      make      : MAKE
      dropDead  : DROPDEAD
      ) @ 'process.so{native}'
   Finalize
export
   is   : IS
   make : Make
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

   proc {AfterGC _}
      {DROPDEAD}
      {Finalize.register AfterGC AfterGC}
   end

   {Finalize.register AfterGC AfterGC}

end

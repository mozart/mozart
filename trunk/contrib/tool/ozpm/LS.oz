%%% {FileTree +DIR ?R}
%%%	returns a record R representing the file structure root at DIR
%%% R has one of the following forms
%%%	stat(type:file size:N ...)
%%%	stat(type:dir  size:N entries:[R1 ... Rk] ...)
%%% where Ri is again of the same form
%%%
functor
import
   Resolve(expand) URL(toVirtualStringExtended) OS(stat getDir)
export
   FileTree
define
   fun {Encode F}
      case F
      of nil  then nil
      [] &#|T then &%|&2|&3|{Encode T}
      [] &{|T then &%|&7|&b|{Encode T}
      []  H|T then        H|{Encode T}
      end
   end
   fun {Expand F}
      {URL.toVirtualStringExtended
       {Resolve.expand {Encode {VirtualString.toString F}}}
       o(full:true raw:true)}
   end
   fun {IsNotDot F}
      F\="." andthen F\=".."
   end
   fun {DirEntries F}
      {Map {Filter {OS.getDir F} IsNotDot}
       fun {$ FF}
	  {AdjoinAt {FileTree F#'/'#FF} name FF}
       end}
   end
   fun {FileTree F}
      F2 = {Expand F}
      R1 = try {OS.stat F2} catch _ then notFound(type:unknown size:0) end
      R2 = {AdjoinAt R1 path F2}
   in
      if R2.type==dir then
	 {AdjoinAt R2 entries {DirEntries F2}}
      else
	 R2
      end
   end
end

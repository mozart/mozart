functor
export
   'class' : URLClass
   Is Make
prepare
   IS_URL = {NewName}
   VS2S = VirtualString.toString
   VS2A = VirtualString.toAtom
import
   URL
   Path at 'x-oz://system/os/Path.ozf'
define
   fun {Is X} {HasFeature X IS_URL} end
   
   fun {Make X}
      if {Is X} then X else
	 {New URL init(X)}
      end
   end

   class URLClass
      feat !IS_URL:unit
      attr info
      meth INIT(R)
	 info <- R
      end
      meth GetInfo($)
	 @info
      end
      meth init(S)
	 URLClass,INIT({URL.make S})
      end
      meth toVS($ full:FULL<=false cache:CACHE<=false raw:RAW<=false)
	 {URL.toVirtualStringExtended @info
	  unit(full:FULL cache:CACHE raw:RAW)}
      end
      meth toString($ full:FULL<=false cache:CACHE<=false raw:RAW<=false)
	 {VS2S URLClass,toVS($ full:FULL cache:CACHE raw:RAW)}
      end
      meth toAtom($ full:FULL<=false cache:CACHE<=false raw:RAW<=false)
	 {VS2A URLClass,toVS($ full:FULL cache:CACHE raw:RAW)}
      end
      meth isAbsolute($)
	 {URL.isAbsolute @info}
      end
      meth isRelative($)
	 {URL.isRelative @info}
      end
      meth resolve(U $)
	 {New URLClass INIT({URL.resolve
			     {URL.toBase @info}
			     {{Make U} GetInfo($)}})}
      end
      meth isRoot($)
	 @info.path == nil
      end
      meth dirname($)
	 case {Reverse @info.path}
	 of nil then self
	 [] _|T then
	    {New URLClass INIT({AdjoinAt @info path {Reverse T}})}
	 end
      end
      meth basename($)
	 {New URLClass
	  INIT(
	     case {Reverse @info.path}
	     of nil then {URL.make nil}
	     [] H|_ then {AdjoinAt {URL.make nil} path [H]}
	     end)}
      end
      meth getScheme($)    @info.scheme end
      meth getAuthority($) @info.authority end
      meth getDevice($)    @info.device end
      meth getAbsolute($)  @info.absolute end
      meth getPath($)      @info.path end
      meth getQuery($)     @info.query end
      meth getFragment($)  @info.fragment end
      meth getInfo($)      @info.info end

      meth toPath($ windows:WIN<=false)
	 S1 = {FoldL @info.path
	       fun {$ Accu Comp}
		  if Accu==nil then Comp else
		     Accu#'/'#Comp
		  end
	       end nil}
	 S2 = if URLClass,getAbsolute($)
	      then '/'#S1 else S1 end
	 WIN2
	 S3 = case URLClass,getDevice($)
	      of unit then WIN2=WIN S2
	      [] C then WIN2=true [C]#S2 end
      in
	 {New Path.'class' init(S3 windows:WIN2)}
      end
   end
end

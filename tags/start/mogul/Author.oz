functor
import
   Admin(manager:Manager)
   Text(
      strip:Strip
      )
   ContactName('class':CName)
export
   toHref:AuthorToHref
define
   fun {AuthorToHref S}
      ID = {VirtualString.toAtom {Strip S}}
      Entry = {Manager condGetId(ID unit $)}
   in
      if Entry==unit then
	 {VirtualString.toString
	  {{New CName init(S)} toVS($)}}
      else
	 a(href:{Manager id_to_href(ID $)}
	   {VirtualString.toString
	    {Entry getSlot('name' $)}})
      end
   end
end
functor
import
   Text(toLower)
   Except('raise':Raise)
export
   'class' : Entry
define
   class Entry
      attr content_type body
      meth init(Msg)
	 content_type <- {Msg condGet1('content-type' unit $)}
	 if @content_type==unit then
	    content_type<-'text/plain'
	 elsecase {VirtualString.toAtom {Text.toLower @content_type}}
	 of 'text/plain' then content_type<-'text/plain'
	 [] 'text/html'  then content_type<-'text/html'
	 [] V then {Raise mogul(invalid_content_type(V))} end
	 body <- {Msg getBody($)}
      end
   end
end

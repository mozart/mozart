functor
   
import
   QHTMLDevel(defineWidget:DefineWidget
	      setTooltips:SetTooltips
	      baseClass:BaseClass
	      send:Send)

define
   
   {DefineWidget
    r(desc:checkbox(action(proc{$ O A} {O bind(event:onclick action:A)} end)
		    tooltips(proc{$ O V} {O SetTooltips(V)} end))
      tag:input
      params:[type#r(default:checkbox
		     init:outer(false)
		     set:outer(false)
		     get:outer(false))
% 	       status#r(type:bool
% 			init:html(true)
% 			set:html(true)
% 			get:html(true))
	      accessKey#r(type:vs
			  init:outer(true)
			  set:outer(true)
			  get:outer(true))
	      disabled#r(type:bool
			 default:false
			 init:html(true)
			 set:html(true)
			 get:internal(true))
	      tabIndex#r(type:int
			 init:outer(true)
			 set:outer(true)
			 get:outer(true))
	      checked#r(type:bool
			default:false
			init:html(true)
			set:html(true)
			get:html(true))
	      indeterminate#r(type:bool
			      default:false
			      init:html(true)
			      set:html(true)
			      get:internal(true))
	     ]
      events:nil
     )
    class $ from BaseClass
       meth blur
	  {self Send({self get(refId:$)}#".blur()")}
       end
       meth focus
	  {self Send({self get(refId:$)}#".focus()")}
       end
    end}

end
   
      

   

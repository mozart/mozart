functor
   
import
   QHTMLDevel(defineWidget:DefineWidget
	      setTooltips:SetTooltips
	      baseClass:BaseClass
	      send:Send)

define
   
   {DefineWidget
    r(desc:button(action(proc{$ O A} {O bind(event:onclick action:A)} end)
		  tooltips(proc{$ O V} {O SetTooltips(V)} end))
      tag:input
      params:[type#r(default:button
		     init:outer(false)
		     set:outer(false)
		     get:outer(false))
	      value#r(type:html
		      init:outer(true)
		      set:outer(true)
		      get:internal(true))
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
   
      

   

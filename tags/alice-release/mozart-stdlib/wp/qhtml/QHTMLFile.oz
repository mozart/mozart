functor
   
import
   QHTMLDevel(defineWidget:DefineWidget
	      setTooltips:SetTooltips
	      send:Send
	      baseClass:BaseClass)

define
   
   {DefineWidget
    r(desc:file(action(proc{$ O A} {O bind(event:onchange action:A)} end)
		tooltips(proc{$ O V} {O SetTooltips(V)} end))
      tag:input
      params:[type#r(default:file
		     init:outer(false)
		     set:outer(false)
		     get:outer(false))
	      value#r(type:html
		      init:outer(true)
		      set:outer(true)
		      get:html(true))
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
	      size#r(type:int
		     init:html(true)
		     set:html(true)
		     get:internal(true))
	     ]
      events:nil
     )
    class $ from BaseClass
       meth select
	  {self Send({self get(refId:$)}#".select()")}
       end
       meth blur
	  {self Send({self get(refId:$)}#".blur()")}
       end
       meth focus
	  {self Send({self get(refId:$)}#".focus()")}
       end
    end}

end
   
      

   

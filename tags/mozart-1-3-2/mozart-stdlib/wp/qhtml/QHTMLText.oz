functor
   
import
   QHTMLDevel(defineWidget:DefineWidget
	      setTooltips:SetTooltips
	      baseClass:BaseClass
	      send:Send)

define
   
   {DefineWidget
    r(desc:text(action(proc{$ O A} {O bind(event:onchange action:A)} end)
		tooltips(proc{$ O V} {O SetTooltips(V)} end))
      tag:input
      params:[type#r(default:text
		     init:outer(false)
		     set:outer(false)
		     get:outer(false))
	      value#r(default:""
		      type:html
		      init:outer(true)
		      set:outer(true)
		       get:html(true))
	      accessKey#r(type:vs
			  init:outer(true)
			  set:outer(true)
			  get:internal(true))
	      disabled#r(type:bool
			 default:false
			 init:html(true)
			 set:html(true)
			 get:internal(true))
	      tabIndex#r(type:int
			 init:outer(true)
			 set:outer(true)
			 get:internal(true))
	      maxLength#r(type:int
			  init:outer(true)
			  set:outer(true)
			  get:internal(true))
	      readOnly#r(type:bool
			 init:html(true)
			 set:html(true)
			 get:internal(true))
	      size#r(type:int
		     init:outer(true)
		     set:outer(true)
		     get:internal(true))
	     ]
      events:nil)
    class $ from BaseClass
%       meth createTextRange
%	  skip
%       end
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
   
      

   

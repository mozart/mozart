functor
   
import
   QHTMLDevel(defineWidget:DefineWidget
	      setTooltips:SetTooltips
	      set:Set
	      send:Send
	      baseClass:BaseClass
	      qInit:QInit)

define
   
   {DefineWidget
    r(desc:radio(action(proc{$ O A} {O bind(event:onclick action:A)} end)
		 tooltips(proc{$ O V} {O SetTooltips(V)} end)
		 group(proc{$ _ _} skip end))
      tag:input
      params:[%value#r(init:custom(false)
		%      set:custom(false)
		%      get:custom(false))
	      type#r(default:radio
		     init:outer(false)
		     set:outer(false)
		     get:outer(false))
	      name#r(init:outer(false)
		     set:outer(false)
		     get:outer(false))
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
% 	       status#r(type:bool
% 			init:html(true)
% 			set:html(true)
% 			get:html(true))
	      checked#r(type:bool
			default:false
			init:html(true)
			set:html(true)
			get:html(true))
	     ]
      events:nil
     )
    class $ from BaseClass
       meth !QInit
	  {self Set(name {self get(group:$)})}
       end
       meth blur
	  {self Send({self get(refId:$)}#".blur()")}
       end
       meth focus
	  {self Send({self get(refId:$)}#".focus()")}
       end
    end}

end
   
      

   

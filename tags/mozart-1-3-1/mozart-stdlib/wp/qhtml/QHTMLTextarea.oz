functor
   
import
   QHTMLDevel(defineWidget:DefineWidget
	      get:Get
	      undefined:Undefined
	      quoteDouble:QuoteDouble
	      cget:Cget
	      send:Send
	      baseClass:BaseClass
	      qInit:QInit
	      buildInnerHTML:BuildInnerHTML)
   HTMLmarshaller(hTML2VS:Html2vs
		  vS2HTML:Vs2html)

define
   fun{Parse S}
      case S
      of &\n|Cs then &&|&#|&1|&3|&;|{Parse Cs}
      [] &\r|Cs then {Parse Cs}
      [] &&|Cs then &&|&&|{Parse Cs}
      [] C|Cs then C|{Parse Cs}
      else nil end
   end
   
   {DefineWidget
    r(desc:textarea(action(proc{$ O A} {O bind(event:onchange action:A)} end))
      tag:textarea
      params:[accessKey#r(type:vs
			  init:outer(true)
			  set:outer(true)
			  get:outer(true))
	      disabled#r(type:bool
			 default:false
			 init:html(true)
			 set:html(true)
			 get:internal(true))
	      cols#r(type:int
		     init:outer(true)
		     set:outer(true)
		     get:internal(true))
	      rows#r(type:int
		     init:outer(true)
		     set:outer(true)
		     get:internal(true))
	      readonly#r(type:bool
			 default:true
			 init:html(true)
			 set:html(true)
			 get:internal(true))
	      tabIndex#r(type:int
			 init:outer(true)
			 set:outer(true)
			 get:outer(true))
	      wrap#r(type:[hard off physical virtual]
		     init:outer(true)
		     set:outer(true)
		     get:outer(true))
	      value#r(type:vs
		      init:custom(true)
		      set:custom(true)
		      get:custom(true))
	     ]
      events:nil
     )
    class $ from BaseClass
       attr list
       meth !QInit(...)=M
	  {self {Record.adjoin M set}}
       end
       meth set(...)=M
	  {Record.forAllInd M
	   proc{$ I V}
	      case I of value then
		 {self Send("top.setTA("#{self get(refId:$)}#',"'#{QuoteDouble {Parse {VirtualString.toString V}}}#'")')}
%		 {self Configure(innerHTML '"'#{QuoteDouble {Vs2html {VirtualString.toString V}}}#'"')}
	      end
	   end}
       end
       meth get(...)=M
	  {Record.forAllInd M
	   proc{$ I V}
	      case I of value then
		 V={Html2vs {self Cget(innerHTML $)}}
	      end
	   end}
       end
       meth blur
	  {self Send({self get(refId:$)}#".blur()")}
       end
       meth focus
	  {self Send({self get(refId:$)}#".focus()")}
       end
       meth select
	  {self Send({self get(refId:$)}#".select()")}
       end
       meth !BuildInnerHTML($)
	  V={self Get(value $)}
       in
	  if V==Undefined then "" else {Vs2html {VirtualString.toString V}} end
       end
    end}
   
end
   
      

   

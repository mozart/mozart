functor
   
import
   QHTMLDevel(defineWidget:DefineWidget
	      baseClass:BaseClass
	      quote:Quote
	      get:Get
	      configure:Configure
	      qInit:QInit
	      buildInnerHTML:BuildInnerHTML)
   HTMLmarshaller(vS2HTML:Vs2html)
   
define
   fun{ToHTML O}
      fun{Loop S}
	 case S
	 of &\\|Xs then &\\|&\\|{Loop Xs}
	 [] &&|&#|&0|&1|&3|Xs then &<|&B|&R|&>|{Loop Xs}
	 [] &&|&#|&0|&1|&0|Xs then {Loop Xs}
%	 [] &\n|Xs then &<|&B|&R|&>|{Loop Xs}
	 [] & |Xs then &&|&n|&b|&s|&p|&;|{Loop Xs}
	 [] X|Xs then X|{Loop Xs}
	 else nil end
      end
      V={O Get(value $)}
   in
      {Loop if {VirtualString.is V} then {Vs2html V} else "" end}
   end
      
   {DefineWidget
    r(desc:label
      tag:span
      params:[value#r(type:vs
		      init:custom(true)
		      set:custom(true)
		      get:internal(true))]
      events:nil
     )
    class $ from BaseClass
       meth !QInit(...)=M
	  {self {Record.adjoin M set}}
       end
       meth set(...)=M
	  {Record.forAllInd M
	   proc{$ I V}
	      case I of value then
		 {self Configure(innerHTML {Quote '"'#{ToHTML self}#'"'})}
	      end
	   end}
       end
       meth !BuildInnerHTML($)
	  {ToHTML self}
       end
    end
   }   

end
   
      

   

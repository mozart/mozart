functor
   
import
   QHTMLDevel(defineWidget:DefineWidget
	      baseClass:BaseClass
	      quoteDouble:QuoteDouble
	      set:Set
	      get:Get
	      send:Send
	      undefined:Undefined
	      sync:Sync
	      onConnect:OnConnect
	      onDisconnect:OnDisconnect
	      join:Join
	      qInit:QInit
	      buildHTML:BuildHTML
	      buildInnerHTML:BuildInnerHTML)
   QUI
   
define

   MBuildChild=QUI.mBuildChild
   MGetChildren=QUI.mGetChildren
   
   {DefineWidget
    r(desc:frameset(container)
      tag:frameset
      params:[border#r(type:int
		       init:outer(true)
		       set:outer(true)
		       get:internal(true))
	      borderColor#r(type:color
			    init:outer(true)
			    set:outer(true)
			    get:internal(true))
	      cols#r(type:framewidth
		     init:outer(true)
		     set:outer(true)
		     get:internal(true))
	      frameBorder#r(type:bool
			    init:html(true)
			    set:html(true)
			    get:internal(true))
% 	      frameSpacing#r(type:int
% 			     init:outer(true)
% 			     set:outer(true)
% 			     get:internal(true))
	      rows#r(type:framewidth
		     init:outer(true)
		     set:outer(true)
		     get:internal(true))
	     ]
      events:nil
     )
    class $ from BaseClass
       meth !QInit(...)=M
	  case {self get(parent:$)}.widgetId
	  of toplevel then skip
	  [] frame then skip
	  [] tdframe then skip
	  [] lrframe then skip
	  else
	     {QUI.raiseError self.widgetId "Error : frameset can be placed inside toplevel, frame, tdframe or lrframe widgets only" M}
	  end
	  {self {Record.adjoin
		 {Record.filterInd M
		  fun{$ I V}
		     if {Int.is I} then
			{self Add(V)}
			false
		     else
			true
		     end
		  end}
		 set}}
       end
       meth Add(C)=M
	  if {Label C}\=frame then
	     {QUI.raiseError self.widgetId "Error : a frameset can contain only frame widgets" C}
	  end
	  {self MBuildChild(C _)}
       end
       meth Remove(C)=M
	  skip
       end
       meth !BuildInnerHTML($)
	  {Join
	   {List.map {Reverse {self MGetChildren($)}}
	    fun{$ C}
	       {C BuildHTML($)}
	    end}
	   ""}
       end
       meth Broadcast(M)
	  {ForAll {self MGetChildren($)}
	   proc{$ C} {C M} end}
       end
       meth close=M
	  {self Broadcast(M)}
       end
       meth !Sync=M
	  {self Broadcast(M)}
       end
       meth !OnConnect=M
	  {self Broadcast(M)}
       end
       meth !OnDisconnect=M
	  {self Broadcast(M)}
       end
    end
   }

   {DefineWidget
    r(desc:frame(container)
      tag:frame
      params:[borderColor#r(type:color
			    init:outer(true)
			    set:outer(false)
			    get:internal(true))
	      frameBorder#r(type:bool
			    init:outer(true)
			    set:outer(false)
			    get:internal(true))
	      height#r(type:int
		       init:outer(true)
		       set:outer(false)
		       get:outer(true))
	      marginHeight#r(type:int
			     init:outer(true)
			     set:outer(false)
			     get:outer(true))
	      marginWidth#r(type:int
			    init:outer(true)
			    set:outer(false)
			    get:outer(true))
	      name#r(type:atom
		     init:outer(false)
		     set:outer(false)
		     get:outer(false))
	      noResize#r(type:bool
			 init:outer(true)
			 set:outer(false)
			 get:internal(true))
	      scrolling#r(type:[auto no yes]
			  init:outer(true)
			  set:outer(false)
			  get:internal(true))
	      src#r(type:html
		    init:outer(true)
		    set:outer(false)
		    get:outer(true))
	     ]
      events:nil
     )
    class $ from BaseClass
       meth !QInit(...)=M
	  Parent={self get(parent:$)}
	  {self Set(name "f"#{self Get(id $)})}
	  Ref={Parent Get(childPrefixId $)}#"."#{self Get(name $)}
       in
	  if Parent.widgetId\=frameset then
	     {QUI.raiseError self.widgetId "Error : frame can be placed inside frameset widgets only" M}
	  end
	  {self Set(childPrefixId Ref)}
%	  {self Set(refId Ref#"."#{self Get(id $)})}
	  if {HasFeature M 1} then
	     {self MBuildChild(M.1 _)}
	  end
       end
       meth !OnConnect
	  CL={self MGetChildren($)}
       in
	  if CL==nil then skip
	  else
	     Child={List.last CL}
	  in
	     {self Send({self Get(childPrefixId $)}#'.document.write("'#
			if {List.member Child.widgetId [frameset tdframe lrframe]} then
			   {QuoteDouble {Child BuildHTML($)}}
			else
			   {QuoteDouble "<body style='width:100%; height:100%' topMargin=0 bottomMargin=0 leftMargin=0 rightMargin=0>"#{Child BuildHTML($)}#"</body>"}
			end#'")')}
	     {self Send({self Get(childPrefixId $)}#'.document.close()')}
	     {Child OnConnect}
	  end
       end
    end}

   proc{Build Dir}
      {DefineWidget
       r(desc:{Record.adjoin frameset(container) {VirtualString.toAtom Dir#"frame"}}
	 tag:frameset
	 params:{List.append
		 [border#r(type:int
			   init:outer(true)
			   set:outer(true)
			   get:internal(true))
		  borderColor#r(type:color
				init:outer(true)
				set:outer(true)
				get:internal(true))
		  frameBorder#r(type:bool
				init:html(true)
				set:html(true)
				get:internal(true))
% 		  frameSpacing#r(type:int
% 				 init:outer(true)
% 				 set:outer(true)
% 				 get:internal(true))
		 ]
		 if Dir==td then 
		    [rows#r(type:framewidth
			    init:outer(true)
			    set:outer(true)
			    get:internal(true))
		     cols#r(type:framewidth
			    default:[100#'%']
			    init:outer(false)
			    set:outer(false)
			    get:internal(false))
		    ]
		 else
		    [rows#r(type:framewidth
			    default:[100#'%']
			    init:outer(false)
			    set:outer(false)
			    get:internal(false))
		     cols#r(type:framewidth
			    init:outer(true)
			    set:outer(true)
			    get:internal(true))]
		 end}
	 events:nil
	)
       class $ from BaseClass
	  meth !QInit(...)=M
	     case {self get(parent:$)}.widgetId
	     of toplevel then skip
	     [] frame then skip
	     [] tdframe then skip
	     [] lrframe then skip
	     else
		{QUI.raiseError self.widgetId "Error : tdframe or lrframe can be placed inside toplevel, frame, tdframe or lrframe widgets only" M}
	     end
	     {self {Record.adjoin
		    {Record.filterInd M
		     fun{$ I V}
			if {Int.is I} then
			   {self Add(V)}
			   false
			else
			   true
			end
		     end}
		    set}}
	     local
		T={Length {self MGetChildren($)}}
		proc{SetD D}
		   if {self Get(D $)}==Undefined then
		      I=100 div T
		   in
		      {self Set(D {List.mapInd
				   {List.make T}
				    fun{$ J C}
				       C=if J==T then '*' else I#'%' end
				    end})}
		   end
		end
	     in
		{SetD if Dir==td then rows else cols end}
	     end
	  end
	  meth Add(C)=M
	     Ref={self Get(childPrefixId $)}#".f"#({Length {self MGetChildren($)}}+1)
	     CP={self Get(childPrefixId $)}
	     {self Set(childPrefixId Ref)}
	     {self MBuildChild(C _)}
	  in
	     {self Set(childPrefixId CP)}
	  end
	  meth Remove(C)=M
	     skip
	  end
	  meth !BuildInnerHTML($)
	     {Join
	      {List.mapInd {Reverse {self MGetChildren($)}}
	       fun{$ I _}
		  "<frame name='f"#I#"'>"
	       end}
	      ""}
	  end
	  meth Broadcast(M)
	     {ForAll {self MGetChildren($)}
	      proc{$ C} {C M} end}
	  end
	  meth close=M
	     {self Broadcast(M)}
	  end
	  meth !Sync=M
	     {self Broadcast(M)}
	  end
	  meth !OnConnect=M
	     {ForAll {Reverse {self MGetChildren($)}}
	      proc{$ C}
		 Ref={C Get(childPrefixId $)}
	      in
		 {self Send(Ref#'.document.write("'#
			    if {List.member C.widgetId [frameset tdframe lrframe]} then
			       {QuoteDouble {C BuildHTML($)}}
			    else
			       {QuoteDouble "<body style='width:100%; height:100%' topMargin=0 bottomMargin=0 leftMargin=0 rightMargin=0>"#{C BuildHTML($)}#"</body>"}
			    end#'")')}
		 {self Send(Ref#'.document.close()')}
	      end}
	     {self Broadcast(M)}
	  end
	  meth !OnDisconnect=M
	     {self Broadcast(M)}
	  end
       end
      }
   end
   
   {Build td}
   {Build lr}
   
end
   
      

   

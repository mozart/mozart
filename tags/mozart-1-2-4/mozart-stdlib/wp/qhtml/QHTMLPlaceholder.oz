functor
   
import
   QHTMLDevel(defineWidget:DefineWidget
	      baseClass:BaseClass
	      quoteDouble:QuoteDouble
	      get:Get
	      configure:Configure
	      sync:Sync
	      onConnect:OnConnect
	      onDisconnect:OnDisconnect
	      qInit:QInit
	      buildHTML:BuildHTML
	      buildInnerHTML:BuildInnerHTML)
   QUI

define

   {DefineWidget
    r(desc:placeholder(container)
      tag:'div'
      params:[1#r(type:no
		  init:custom(true)
		  set:custom(true)
		  get:custom(true))]
      events:nil
     )
    class $ from BaseClass
       attr childList child
       meth !QInit(...)=M
	  childList<-nil
	  child<-unit
	  {self {Record.adjoin M set}}
       end
       meth set(...)=M
	  {Record.forAllInd M
	   proc{$ I C}
	      case I of 1 then
		 if {Object.is C} then
		    if {List.member C @childList} then
		       if @child\=unit andthen {@child Get(isdisplayed $)} then
			  {@child Sync}
			  {@child OnDisconnect}
		       end
		       child<-C
		       if {self Get(isdisplayed $)} then
			  {self Configure(innerHTML '"'#{QuoteDouble {C BuildHTML($)}}#'"')}
			  {C OnConnect}
		       end
		    else
		       {QUI.raiseError self.widgetId "Invalid child : "#C M}
		    end
		 elseif C==unit then
		    if @child\=unit andthen {@child Get(isdisplayed $)} then
		       {@child Sync}
		       {@child OnDisconnect}
		    end
		    child<-C
		    {self Configure(innerHTML '""')}
		 elseif {Record.is C} then
		    O={QUI.build C self}
		 in
		    childList<-O|@childList
		    {self set(O)}
		 else
		    {QUI.raiseError self.widgetId "Invalid child : "#C M}
		 end
	      end
	   end}
       end
       meth get(...)=M
	  {Record.forAllInd M
	   proc{$ I V}
	      case I of 1 then V=@child end
	   end}
       end
       meth remove(C)=M
	  if {List.member C @childList} then
	     if @child==C then {self set(unit)} end
	     {C close}
	     childList<-{List.filter @childList fun{$ L} C\=C end}
	  else
	     {QUI.raiseError self.widgetId "Invalid child : "#C M}
	  end
       end
       meth close
	  {ForAll @childList proc{$ O} {O close} end}
       end
       meth !Sync
	  if @child\=unit then {@child Sync} end
       end
       meth !OnConnect
	  if @child\=unit then {@child OnConnect} end
       end
       meth !OnDisconnect
	  if @child\=unit then {@child OnDisconnect} end
       end
       meth !BuildInnerHTML($)
	  if @child==unit then "" else
	     {@child BuildHTML($)}
	  end
       end
    end
   }   

end
   
      

   

functor
   
import
   QUI
   QHTMLDevel(defineWidget:DefineWidget
	      onConnect:OnConnect
	      setTooltips:SetTooltips
	      baseClass:BaseClass
	      set:Set
	      get:Get
	      dataInfo:DataInfo
	      configure:Configure
	      quoteDouble:QuoteDouble
	      send:Send
	      qInit:QInit
	      buildInnerHTML:BuildInnerHTML)
   QHTMLType(marshall:Marshall)
   HTMLmarshaller(vS2HTML:Vs2html)

define
   
   {DefineWidget
    r(desc:hyperlink(tooltips(proc{$ O V} {O SetTooltips(V)} end))
      tag:a
      params:[value#r(type:html
		      default:""
		      init:custom(true)
		      set:custom(true)
		      get:internal(true))
	      accessKey#r(type:vs
			 init:outer(true)
			 set:outer(true)
			 get:internal(true))
	      charset#r(type:html
			init:outer(true)
			set:outer(true)
			get:internal(true))
	      href#r(type:fun{$ T}
			     {IsFree T} orelse
			     (({Object.is T} andthen (T.widgetId==anchor)) orelse {VirtualString.is T})
			  end
		     marshall:fun{$ V}
				 if {IsFree V} then ""
				 elseif {Object.is V} then
				    '"#'#{V Get(name $)}#'"'
				 else
				    '"'#{Vs2html V}#'"'
				 end
			      end
		     init:outer(true)
		     set:outer(true)
		     get:internal(true))
	      hreflang#r(type:html
			 init:outer(true)
			 set:outer(true)
			 get:internal(true))
	      methods#r(type:html
			init:outer(true)
			set:outer(true)
			get:internal(true))
	      tabIndex#r(type:int
			 init:outer(true)
			 set:outer(true)
			 get:internal(true))
	      target#r(type:['_blank' '_parent' '_self' '_top']
		       init:outer(true)
		       set:outer(true)
		       get:internal(true))
	      type#r(type:html
		     init:outer(true)
		     set:outer(true)
		     get:internal(true))
	      urn#r(type:html
		    init:outer(true)
		    set:outer(true)
		    get:internal(true))
	     ]
      events:nil
     )
    class $ from BaseClass
       meth !QInit(...)=M
	  {self {Record.adjoin M set}}
       end
       meth set(...)=M
	  {Record.forAllInd M
	   proc{$ I V}
	      case I
	      of value then
		 {self Configure(innerHTML
				 {QuoteDouble '"'#{self BuildInnerHTML($)}#'"'}
				)}
	      end
	   end}
       end
       meth !BuildInnerHTML($)
	  {Vs2html {self Get(value $)}}
       end
       meth !OnConnect
	  {self Configure(href {Marshall self {Dictionary.get self.DataInfo href} {self Get(href $)}})}
       end
       meth blur
	  {self Send({self get(refId:$)}#".blur()")}
       end
       meth focus
	  {self Send({self get(refId:$)}#".focus()")}
       end
    end}

   {DefineWidget
    r(desc:anchor
      tag:a
      params:[name#r(type:html
		     init:outer(false)
		     set:outer(false)
		     get:internal(false))]
      events:nil
     )
    class $ from BaseClass
       meth !QInit(...)=M
	  {self Set(name {self Get(id $)})}
       end
    end}

   {QUI.setAlias a hyperlink}

end
   
      

   

functor
   
import
   QUI
   QHTMLDevel(defineWidget:DefineWidget
	      onConnect:OnConnect
	      baseClass:BaseClass
	      set:Set
	      configure:Configure
	      quoteDouble:QuoteDouble
	      removeQuote:RemoveQuote
	      undefined:Undefined
	      events:Events
	      execEvent:ExecEvent
	      get:Get
	      send:Send
	      join:Join
	      qInit:QInit
	      buildHTML:BuildHTML
	      buildInnerHTML:BuildInnerHTML)
   HTMLmarshaller(vS2HTML:Vs2html)
   QHTMLType(record2str:Record2str)

define

   OnSelect={NewName}
   Rebuild={NewName}
   SetDeep={NewName}

   {DefineWidget
    r(desc:menubutton(container)
      tag:span
      params:[component#r(type:vs
			  default:menulabel
			  init:outer(false)
			  set:outer(false)
			  get:internal(false))
	      position#r(type:[belowright
			       belowleft
			       aboveright
			       aboveleft]
			 default:belowright
			 init:outer(true)
			 set:outer(false)
			 get:internal(true))
	      onclick#r(type:vs
			default:"top.menubtnclick(event)"
			init:outer(false)
			set:outer(false)
			get:internal(false))
	      onmouseover#r(type:vs
			    default:"top.menubtnenter(event)"
			    init:outer(false)
			    set:outer(false)
			    get:internal(false))
	      onmouseout#r(type:vs
			   default:"top.menubtnexit(event)"
			   init:outer(false)
			   set:outer(false)
			   get:internal(false))
	      cursor#r(type:atom
		       default:hand
		       init:style(false)
		       set:style(false)
		       get:internal(false))
	      border#r(type:vs
		       default:"'2 solid'"
		       init:style(false)
		       set:style(false)
		       get:internal(false))
	      padding#r(type:int
			default:2
			init:style(false)
			set:style(false)
			get:internal(false))
	      'height'#r(type:int
			 default:20
			 init:style(false)
			 set:style(false)
			 get:internal(false))
	      'background-color'#r(type:atom
				   init:style(false)
				   set:style(false)
				   get:internal(false))
	      color#r(type:atom
		      init:style(false)
		      set:style(false)
		      get:internal(false))
	      value#r(type:html
		      default:""
		      init:custom(true)
		      set:custom(false)
		      get:internal(true))]
      events:nil)
    class $ from BaseClass
       feat GetColor
       attr children
       meth !QInit(...)=M
	  children<-nil
	  self.GetColor=fun{$ C}
			   {RemoveQuote {Record2str {{self get(toplevel:$)} Get(C $)}}}
			end
	  {self Set('background-color' {self.GetColor menuBgColor})}
	  {self Set(color {self.GetColor menuFgColor})}
	  {Record.forAllInd M
	   proc{$ I V}
	      if {Int.is I} then
		 {self add(V)}
	      end
	   end}
       end
       meth add(Child)=M
	  if {List.member {Label Child} [menuitem menuradio menucheck menuhr submenu]} then
	     O={QUI.build Child self}
	     {O SetDeep(1)}
	  in
	     children<-O|@children
	     if {self get(isdisplayed:$)} then
		{self Configure(innerHTML '"'#{QuoteDouble {self BuildInnerHTML($)}}#'"')}
	     end
	  else
	     {QUI.raiseError self.widgetId "Invalid widget to place inside menubutton widget :"#Child M}
	  end
       end
       meth !Rebuild
	  if {self get(isdisplayed:$)} then
	     {self Configure(innerHTML '"'#{QuoteDouble {self BuildInnerHTML($)}}#'"')}
	  end
       end
       meth !BuildInnerHTML($)
	  " "#{Vs2html {self Get(value $)}}#" <div "#
	  "style=\"display:none; cursor:hand; width:0; position:absolute; top:0; left:0; border:'2 solid'; "#
	  "background-color:"#{self.GetColor menuBgColor}#";"#
	  "border-bottom-color:"#{self.GetColor menuButtonBottomColor}#";"#
	  "border-right-color:"#{self.GetColor menuButtonRightColor}#";"#
	  "border-top-color:"#{self.GetColor menuButtonTopColor}#";"#
	  "border-left-color:"#{self.GetColor menuButtonLeftColor}#";\">"#
	  "<table border=0 cellpadding=0 cellspacing=0 bgColor="#{self.GetColor menuBgColor}#">"#
	  {Join
	   {List.map {Reverse @children}
	    fun{$ C} {C BuildHTML($)} end}
	   ""}#"</table></div>"
       end
       meth !OnConnect
	  {self Send("top.menuhighlight("#{self get(refId:$)}#",0)")}
       end
    end}

   fun{IsSC R}
      if R==unit then true
      elseif {Atom.is R} then
	 true
      elsecase R
      of ctrl(X) then {IsSC X}
      [] alt(X) then {IsSC X}
      [] shift(X) then {IsSC X}
      else false end
   end

   fun{Capitalize S}
      fun{Loop S}
	 case S
	 of C|Cs then
	    if C>=&A andthen C=<&Z then
	       (C+&a-&A)|{Loop Cs}
	    else
	       C|{Loop Cs}
	    end	 	    
	 else S end
      end
   in
      case S
      of C|Cs then
	 if C>=&a andthen C=<&z then
	    (C+&A-&a)|{Loop Cs}
	 else
	    C|{Loop Cs}
	 end	 
      else S end
   end
   
   fun{SC2STR R}
      if R==unit then ""
      elseif {Atom.is R} then
	 {Capitalize {Atom.toString R}}
      elsecase R
      of ctrl(X) then "Ctrl+"#{SC2STR X}
      [] alt(X) then "Alt+"#{SC2STR X}
      [] shift(X) then "Shift+"#{SC2STR X}
      else "" end
   end

   fun{Nbsp V}
      fun{Loop R}
	 case R
	 of & |Cs then &&|&n|&b|&s|&p|&;|{Loop Cs}
	 [] C|Cs then C|{Loop Cs}
	 else nil end
      end
   in
      {Loop {VirtualString.toString V}}
   end

   fun{BuildClass BaseClass}
      class $ from BaseClass
	 feat GetColor Deep
	 attr children
	 meth !QInit(...)=M
	    self.GetColor=fun{$ C}
			     {RemoveQuote {Record2str {{self get(toplevel:$)} Get(C $)}}}
			  end
	    if {Not {List.member {self get(parent:$)}.widgetId [menubutton submenu]}} then
	       {QUI.raiseError self.widgetId "menu widgets can be placed inside menubutton or submenu widgets only" M}
	    end
	    Ref={self Get(id $)}
	 in
	    children<-nil
	    {self Set(onmouseout "top.rowexit("#Ref#")")}
	    {self Set(onclick "top.rowclick("#Ref#")")}
	    local
	       Port#Msg={{self get(actionManager:$)} getAction(self#OnSelect $)}
	    in
	       self.Events.onselectmenu:=Port#Msg#nil
	    end
	    if {self Get(group $)}\=Undefined then
	       {self Set(checked {self getState($)})}
	    end
	    local
	       Remain={Record.filterInd M
		       fun{$ I V}
			  if {Int.is I} then
			     {self add(V)}
			     false
			  else
			     true
			  end
		       end}
	    in
	       {self {Record.adjoin Remain set}}
	    end
	 end
	 meth !SetDeep(I)
	    self.Deep=I
	    {self Set(onmouseover "top.rowenter(event,"#{self Get(id $)}#","#self.Deep#")")}
	 end
	 meth set(...)=M
	    if (M\=set) then
	       Group={self Get(group $)}
	    in
	       if {HasFeature M checked} andthen ({self Get(group $)}\=Undefined) andthen M.checked then
		  GM={self get(groupManager:$)}
	       in
		  try
		     {{GM getActive(Group $)} set(checked:false)}
		  catch _ then skip end
		  {GM setActive(Group self)}
	       end
	       {{self get(parent:$)} Rebuild}
	    end
	 end
	 meth !BuildInnerHTML($)
	    Checked={self Get(checked $)}
	    Group={self Get(group $)}
	    Icon={self Get(icon $)}
	 in
	    "<td><span style='width:100%; cursor:default; margin-left:4; margin-right:4;'>"#
	    if Checked==Undefined then
	       if Icon=="" then ""
	       else
		  "<img align='left' src='"#{Vs2html Icon}#"'>"
	       end
	    else
	       if Group==Undefined then
		  if Checked then "<b>&radic;</b>"
		  else "" end
		else
		  if Checked then "<b>&bull;</b>"
		  else "" end
	       end
	    end#"</span>"#
	    "<td><span style='width:100%; cursor:default; margin-left:4; margin-right:4;'>"#
	    {Nbsp {self Get(value $)}}#"</span>"#
	    "<td><span style='width:100%; cursor:default; margin-left:4; margin-right:4;'>"#
	    {SC2STR {self Get(shortcut $)}}#"</span>"#
	    "<td><span style='width:100%; cursor:default; margin-left:4; margin-right:4;'>"#
	    if self.widgetId==submenu then "<b>&raquo;</b><td><div "#
	       "style=\"display:none; cursor:hand; width:0; position:absolute; top:0; left:0; border:'2 solid'; "#
	       "background-color:"#{self.GetColor menuBgColor}#"; "#
	       "border-bottom-color:"#{self.GetColor menuButtonBottomColor}#"; "#
	       "border-right-color:"#{self.GetColor menuButtonRightColor}#"; "#
	       "border-top-color:"#{self.GetColor menuButtonTopColor}#"; "#
	       "border-left-color:"#{self.GetColor menuButtonLeftColor}#"; \">"#
	       "<table border=0 cellpadding=0 cellspacing=0 bgColor="#{self.GetColor menuBgColor}#">"#
	       {Join
		{List.map {Reverse @children}
		 fun{$ C} {C BuildHTML($)} end}
		""}#"</table></div>"
	    else "" end#"</span>"
	 end
	 meth bind(...)=M skip end
	 meth !OnSelect
	    Checked={self Get(checked $)}
	    Group={self Get(group $)}
	 in
	    if Group\=Undefined then
	       {self set(checked:true)}
	    elseif Checked\=Undefined then
	       {self set(checked:{Not {self get(checked:$)}})}
	    end
	    {self ExecEvent(onselect)}
	 end
	 meth add(...)=M
	    if self.widgetId\=submenu then
	       try
		  {self inexistingadd}
	       catch X then
		  raise {Record.adjoinAt X 1
			 {Record.adjoinAt X.1 3 M}} end
	       end
	    end
	    if {Arity M}\=[1] then {QUI.raiseError self.widgetId "Invalid call to add method" M} end
	    Child=M.1
	 in
	    if {List.member {Label Child} [menuitem menuradio menucheck menuhr submenu]} then
	       O={QUI.build Child self}
	       thread
		  {O SetDeep(self.Deep+1)}
	       end
	    in
	       children<-O|@children
	       if {self get(isdisplayed:$)} then
		  {{self get(parent:$)} Rebuild}
	       end
	    else
	       {QUI.raiseError self.widgetId "Invalid widget to place inside menubutton widget :"#Child M}
	    end
	 end
	 meth !Rebuild
	    if {self get(isdisplayed:$)} then
	       {{self get(parent:$)} Rebuild}
	    end
	 end
      end
   end

   
   {DefineWidget
    r(desc:menuitem(action(proc{$ O A} {O bind(event:onselect action:A)} end))
      tag:tr
      params:[onmouseover#r(type:no
			    init:outer(false)
			    set:outer(false)
			    get:internal(false))
	      onmouseout#r(type:no
			    init:outer(false)
			    set:outer(false)
			   get:internal(false))
	      onclick#r(type:no
			init:outer(false)
			set:outer(false)
			get:internal(false))
	      icon#r(type:no
		     default:""
		     init:custom(true)
		     set:custom(true)
		     get:internal(true))
	      value#r(type:html
		      default:""
		      init:custom(true)
		      set:custom(true)
		      get:internal(true))
	      shortcut#r(type:IsSC
			 default:unit
			 init:custom(true)
			 set:custom(true)
			 get:internal(true))]
      events:[onselect#r(bind:custom(true))
	      onselectmenu#r(bind:custom(false))])
    {BuildClass BaseClass}}
   
   {DefineWidget
    r(desc:menucheck(action(proc{$ O A} {O bind(event:onselect action:A)} end))
      tag:tr
      params:[onmouseover#r(type:no
			    init:outer(false)
			    set:outer(false)
			    get:internal(false))
	      onmouseout#r(type:no
			    init:outer(false)
			    set:outer(false)
			   get:internal(false))
	      onclick#r(type:no
			init:outer(false)
			set:outer(false)
			get:internal(false))
	      checked#r(type:bool
			default:false
			init:custom(true)
			set:custom(true)
			get:internal(true))
	      value#r(type:html
		      default:""
		      init:custom(true)
		      set:custom(true)
		      get:internal(true))
	      shortcut#r(type:IsSC
			 default:unit
			 init:custom(true)
			 set:custom(true)
			 get:internal(true))]
      events:[onselect#r(bind:custom(true))
	      onselectmenu#r(bind:custom(false))])
    {BuildClass BaseClass}}
   
   {DefineWidget
    r(desc:menuradio(group(proc{$ _ _} skip end)
		     action(proc{$ O A} {O bind(event:onselect action:A)} end))
      tag:tr
      params:[onmouseover#r(type:no
			    init:outer(false)
			    set:outer(false)
			    get:internal(false))
	      onmouseout#r(type:no
			    init:outer(false)
			    set:outer(false)
			   get:internal(false))
	      onclick#r(type:no
			init:outer(false)
			set:outer(false)
			get:internal(false))
	      checked#r(type:bool
			default:false
			init:custom(true)
			set:custom(true)
			get:internal(true))
	      value#r(type:html
		      default:""
		      init:custom(true)
		      set:custom(true)
		      get:internal(true))
	      shortcut#r(type:IsSC
			 default:unit
			 init:custom(true)
			 set:custom(true)
			 get:internal(true))]
      events:[onselect#r(bind:custom(true))
	      onselectmenu#r(bind:custom(false))])
    {BuildClass BaseClass}}

   {DefineWidget
    r(desc:submenu(container)
      tag:tr
      params:[component#r(type:vs
			  default:submenulabel
			  init:outer(false)
			  set:outer(false)
			  get:internal(false))
	      position#r(type:atom
			 default:right
			 init:outer(false)
			 set:outer(false)
			 get:internal(false))
	      onmouseover#r(type:no
			    init:outer(false)
			    set:outer(false)
			    get:internal(false))
	      onmouseout#r(type:no
			    init:outer(false)
			    set:outer(false)
			   get:internal(false))
	      onclick#r(type:no
			init:outer(false)
			set:outer(false)
			get:internal(false))
	      cursor#r(type:atom
		       default:hand
		       init:style(false)
		       set:style(false)
		       get:internal(false))
	      'height'#r(type:int
			 default:20
			 init:style(false)
			 set:style(false)
			 get:internal(false))
	      value#r(type:html
		      default:""
		      init:custom(true)
		      set:custom(false)
		      get:internal(true))
	      icon#r(type:no
		     default:""
		     init:custom(true)
		     set:custom(true)
		     get:internal(true))
	      value#r(type:html
		      default:""
		      init:custom(true)
		      set:custom(true)
		      get:internal(true))
	      shortcut#r(type:IsSC
			 default:unit
			 init:custom(true)
			 set:custom(true)
			 get:internal(true))]
      events:nil)
    {BuildClass BaseClass}}

   
   {DefineWidget
    r(desc:menuhr
      tag:tr
      params:nil
      events:nil)
    class $ from BaseClass
       meth !QInit(...)=M skip end
       meth set(...)=M skip end
       meth !SetDeep(I) skip end
       meth !BuildInnerHTML($)
	  "<td><span style='width:100%; cursor:default; margin-left:0; margin-right:0;'><hr></span>"#
	  "<td><span style='width:100%; cursor:default; margin-left:0; margin-right:0;'><hr></span>"#
	  "<td><span style='width:100%; cursor:default; margin-left:0; margin-right:0;'><hr></span>"#
	  "<td><span style='width:100%; cursor:default; margin-left:0; margin-right:0;'><hr></span>"
       end
    end
   }
   
			    
end
   
      

   


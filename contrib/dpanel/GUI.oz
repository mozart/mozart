functor 
import
   Tk
   Widgets(cardFrame toplevel)
   Graph(graph:GraphClass)
export
   Open
   
   ssites:SSites
   sactivity:SActive
   snumber:SNumber
   
   osites:OSites
   oactive:OActive
   onumber:ONumber
   
   bsites:BSites
   bactive:BActive
   bnumber:BNumber

   nilist:NIList
   ninumber:NINumber
   nibyte:NIByte
define
   SSites SActive SNumber
   OSites OActive ONumber
   BSites BActive BNumber
   NIList NINumber NIByte

   class TitleGraph from Tk.frame
      feat graph 

      meth tkInit(parent:P title:T ...)=M G L Init in
	 Tk.frame, tkInit(parent:P)
	 Init={Record.adjoin {Record.subtract M title} init(parent:self)}
	 self.graph=G={New GraphClass Init}
	 L={New Tk.label tkInit(parent:self text:T)}
	 {Tk.batch [grid(L row:0 column:0 sticky:we)
		    grid(G row:1 column:0 sticky:news)]}
      end

      meth otherwise(X)
	 {self.graph X}
      end
   end
   
   class SiteList from Tk.frame
      prop
	 final locking
      feat
	 listbox
	 InternalHelpFun1
      attr
	 cy:1
	 width:18*8
	 height:17*8
	 sites:nil
	 action:proc{$ _} skip end

      meth tkInit(...)=M
	 LB SY
      in
	 Tk.frame, M
	 self.listbox=LB={New Tk.canvas tkInit(parent:self
					       bd:2
					       relief:sunken
					       bg:white
					       width:@width
					       height:@height)}
	 SY={New Tk.scrollbar tkInit(parent:self
				     width:8)}
	 {Tk.addYScrollbar LB SY}
	 {Tk.batch [grid(LB row:0 column:0 sticky:news)
		    grid(SY row:0 column:1 sticky:ns)
		    grid(columnconfigure self 0 weight:1)
		    grid(rowconfigure self 0 weight:1)]}
	 
	 self.InternalHelpFun1 = proc{$ X} Y0 Y1
				    DC=self.listbox
				 in
				    
				    {X.bgtag tk(delete)}
				    {X.fgtag tk(delete)}
				    {DC tk(crea text 5 @cy text:X.text anchor:nw fill:X.fg tags:X.fgtag)}
				    [_ Y0 _ Y1]={DC tkReturnListInt(bbox X.fgtag $)}
				    cy<-Y1+1
				    {DC tk(crea rect 0 Y0 1000 Y1+1 fill:X.bg tags:X.bgtag outline:X.bg)}
				    {DC tk('raise' X.fgtag)}
				 end
	 
	 
      end
   
      meth setAction(P)
	 action<-P
      end

      meth Action(A)
	 {@action A}
      end
   
      meth addSite(Ks)=M
	 DC=self.listbox
      in
	 if {IsList Ks}==false then
	    {Exception.'raise' giveMeAFuckingListPleaze(M)}
	 else
	    skip
	 end
	 sites <- {Append @sites {Map Ks proc{$ K O}
					    T1={New Tk.canvasTag tkInit(parent:DC)}
					    T2={New Tk.canvasTag tkInit(parent:DC)}
					    S=site(text:K.text
						   key:K.key
						   fg:{CondSelect K fg black}
						   bg:{CondSelect K bg white}
						   bgtag:T1
						   fgtag:T2)
					 in
					    O = S
					    {DC tk(crea line 0 0 0 1 tags:q(T1 T2))}
					    {T1 tkBind(event:'<1>'
						       action:self#Action(K.key))}
					    {T2 tkBind(event:'<1>'
						       action:self#Action(K.key))}
					 end}}
	 {self Redraw(@sites)}
      end
      
      meth deleteSite(Ks)=M
	 if {IsList Ks}==false then
	    {Exception.'raise' giveMeAFuckingListPleaze(M)}
	 else
	    skip
	 end
	 sites<-{List.filterInd @sites fun{$ I X}
					  if {Member X.key Ks} then
					     {X.bgtag tk(delete)}
					     {X.fgtag tk(delete)}
					     false
					  else
					     true
					  end
				       end}
	 {self Redraw(@sites)}
      end

      
      meth Redraw(Ss)
	 cy<-1
	 {ForAll Ss self.InternalHelpFun1}
	 {self.listbox tk(configure scrollregion:q(0 1 1000 @cy))}
      end
   
		    
      meth setColour(key:K bg:BG fg:FG)
	 sites<-{Map @sites fun{$ X}
			       if K==X.key then
				  {X.fgtag tk(itemconfig fill:FG)}
				  {X.bgtag tk(itemconfig fill:BG outline:BG)}
				  site(key:X.key
				       text:X.text
				       bg:BG
				       fg:FG
				       bgtag:X.bgtag
				       fgtag:X.fgtag)
			       else X end
			    end}
      end
   end

   Toplevel
   proc{Open Resume TThread}
      if Resume==true then
	 {Thread.resume TThread}
	 {Toplevel tkShow}
      else
	 T=Toplevel={New Widgets.toplevel tkInit(title:"Distribution Panel"
						 delete:proc{$}
							   {T tkHide}
							   {Thread.suspend TThread}
							end)}
	 CardF={New Widgets.cardFrame tkInit(parent:T padx:10 pady:10 width:900 height:190)}
	 SiteF OwnerF BorrowF NetInfoF
      in
	 %% Site frame
	 SiteF={New Tk.frame tkInit(parent:CardF)}
	 SSites={New SiteList tkInit(parent:SiteF)}
	 SActive={New TitleGraph tkInit(parent:SiteF
					title:"#Activity/s"
					miny:1.0
					maxy:11.0
					dim:''
					fill:false)}
	 SNumber={New TitleGraph tkInit(parent:SiteF
					title:"#Sites in store/s"
					miny:1.0
					maxy:11.0
					dim:''
					fill:true)}
   
	 %% Owner frame
	 OwnerF={New Tk.frame tkInit(parent:CardF)}
	 OSites={New SiteList tkInit(parent:OwnerF)}
	 OActive={New TitleGraph tkInit(parent:OwnerF
					title:"#Active/s"
					miny:1.0
					maxy:11.0
					dim:''
					fill:true)}
	 ONumber={New TitleGraph tkInit(parent:OwnerF
					title:"#Entities/s"
					miny:1.0
					maxy:11.0
					dim:''
					fill:true)}
   

	 %% Borrow frame
	 BorrowF={New Tk.frame tkInit(parent:CardF)}
	 BSites={New SiteList tkInit(parent:BorrowF)}
	 BActive={New TitleGraph tkInit(parent:BorrowF
					title:"#Active/s"
					miny:1.0
					maxy:11.0
					dim:''
					fill:true)}
	 BNumber={New TitleGraph tkInit(parent:BorrowF
					title:"#Entities/s"
					miny:1.0
					maxy:11.0
					dim:''
					fill:true)}
   
	 %% Net info frame
	 NetInfoF={New Tk.frame tkInit(parent:CardF)}
	 NIList={New SiteList tkInit(parent:NetInfoF)}
	 NINumber={New TitleGraph tkInit(parent:NetInfoF
					 title:"Number"
					 miny:1.0
					 maxy:11.0
					 dim:''
					 fill:false)}
	 NIByte={New TitleGraph tkInit(parent:NetInfoF
				       title:"Byte"
				       miny:1.0
				       maxy:11.0
				       dim:''
				       fill:true)}
   

	 {Tk.batch [grid(SSites	row:0 column:0 sticky:news)
		    grid(SActive	row:0 column:1 sticky:news)
		    grid(SNumber	row:0 column:2 sticky:news) 

		    grid(OSites	row:0 column:0 sticky:news)
		    grid(OActive	 row:0 column:1 sticky:news)
		    grid(ONumber	row:0 column:2 sticky:news) 

		    grid(BSites	row:0 column:0 sticky:news)
		    grid(BActive	row:0 column:1 sticky:news)
		    grid(BNumber	row:0 column:2 sticky:news) 

		    grid(NIList	row:0 column:0 sticky:news)
		    grid(NINumber	row:0 column:1 sticky:news)
		    grid(NIByte	row:0 column:2 sticky:news) 
		 
		    grid(columnconfigure SiteF 0 weight:1)
		    grid(columnconfigure OwnerF 0 weight:1)
		    grid(columnconfigure BorrowF 0 weight:1)
		    grid(columnconfigure NetInfoF 0 weight:1)

		    grid(rowconfigure SiteF 0 weight:1)
		    grid(rowconfigure OwnerF 0 weight:1)
		    grid(rowconfigure BorrowF 0 weight:1)
		    grid(rowconfigure NetInfoF 0 weight:1)
		   ]}

	 {CardF addCard(id:1 title:" Site Info " frame:SiteF)}
	 {CardF addCard(id:2 title:" Owner Info " frame:OwnerF)}
	 {CardF addCard(id:3 title:" Borrow Info " frame:BorrowF)}
	 {CardF addCard(id:4 title:" Net Info " frame:NetInfoF)}
      
	 {Tk.batch [grid(columnconfigure T 0 weight:1)
		    grid(rowconfigure T 0 weight:1)
		    grid(CardF row:0 column:0 sticky:news)]}
      end
   end
end





%%%
%%% Authors:
%%%   Nils Franzen <nilsf@sics.se>
%%%   Erik Klintskog <erik@sic.se>
%%%   Andreas Sundstroem 
%%%
%%% Contributors:
%%%   Anna Neiderud <annan@sics.se>
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%    http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor 
import
   Tk
   Widgets(cardFrame toplevel)
   FieldDisplay
   TitleGraph
   AdvancedListBox(advancedListBox:SiteList)
export
   Open
   ReOpen
   OpenNetInfo
   ssites:SSites
   sactivity:SActive
   srtt:SRTT
   
   osites:OSites
   oactive:OActive
   onumber:ONumber
   
   bsites:BSites
   bactive:BActive
   bnumber:BNumber

   nilist:NIList
   ninumber:NINumber
   nibyte:NIByte
   messageSent: MsentStat
   messageReceived: MreceivedStat
   diffSent:DsentStat
   diffReceived:DreceivedStat


   RemoteSiteWin
define
   SSites SActive SRTT
   OSites OActive ONumber
   BSites BActive BNumber
   NIList NINumber NIByte
   MsentStat
   MreceivedStat
   SMLabel
   RMLabel

   DsentStat
   DreceivedStat
   SDLabel
   RDLabel

   Toplevel
   proc{ReOpen}
      {Toplevel tkShow}
   end

   CardF NetInfoF
   proc{Open  RunSync}
      T=Toplevel={New Widgets.toplevel tkInit(title:"Distribution Panel"
					      delete:proc{$}
							{T tkHide}
							{Exchange RunSync unit _}
						     end)}
      !CardF={New Widgets.cardFrame tkInit(parent:T padx:10 pady:10 width:600 height:300)}
      SiteF OwnerF BorrowF MessageF DiffTypeF
   in
      %% Site frame
      SiteF={New Tk.frame tkInit(parent:CardF)}
      SSites={New SiteList tkInit(parent:SiteF)}
      SActive={New TitleGraph.titleGraph tkInit(parent:SiteF
				     title:"Message exchange per sample period"
				     miny:1.0
				     maxy:11.0
				     dim:''
				     fill:false)}
      SRTT={New TitleGraph.titleGraph tkInit(parent:SiteF
				  title:"Last RTT (ms)"
				  miny:1.0
				  maxy:11.0
				  dim:''
				  fill:false)}
      
      %% Owner frame
      OwnerF={New Tk.frame tkInit(parent:CardF)}
      OSites={New SiteList tkInit(parent:OwnerF)}
      OActive={New TitleGraph.titleGraph tkInit(parent:OwnerF
				     title:"Number of imported/exported Entities"
				     miny:1.0
				     maxy:11.0
				     dim:''
				     fill:true)}
      ONumber={New TitleGraph.titleGraph tkInit(parent:OwnerF
				     title:"Number of Entities"
				     miny:1.0
				     maxy:11.0
				     dim:''
				     fill:true)}
   

      %% Borrow frame
      BorrowF={New Tk.frame tkInit(parent:CardF)}
      BSites={New SiteList tkInit(parent:BorrowF)}
      BActive={New TitleGraph.titleGraph tkInit(parent:BorrowF
				     title:"Number of imported/exported Entities"
				     miny:1.0
				     maxy:11.0
				     dim:''
				     fill:true)}
      BNumber={New TitleGraph.titleGraph tkInit(parent:BorrowF
				     title:"Number of Entities"
				     miny:1.0
				     maxy:11.0
				     dim:''
				     fill:true)}
   
      %% Net info frame
      NetInfoF={New Tk.frame tkInit(parent:CardF)}
      NIList={New SiteList tkInit(parent:NetInfoF)}
      NINumber={New TitleGraph.titleGraph tkInit(parent:NetInfoF
				      title:"Number"
				      miny:1.0
				      maxy:11.0
				      dim:''
				      fill:false)}
      NIByte={New TitleGraph.titleGraph tkInit(parent:NetInfoF
				    title:"Byte"
				    miny:1.0
				    maxy:11.0
				    dim:''
				    fill:true)}

      %% MessageDiffStatistics
      MessageF={New Tk.frame tkInit(parent:CardF)}
      SMLabel ={New Tk.label tkInit(parent:MessageF
				    text:"Sent message counted by type")}
      
      RMLabel ={New Tk.label tkInit(parent:MessageF
				    text:"Received message counted by type")}
      
      MsentStat = {New FieldDisplay.fieldDisplayClass
		   open(parent:MessageF width:550 height:14*8)}
      
      MreceivedStat = {New FieldDisplay.fieldDisplayClass
		       open(parent:MessageF width:550 height:14*8)}

      %% DiffTypeStatistics
      DiffTypeF={New Tk.frame tkInit(parent:CardF)}
      SDLabel ={New Tk.label tkInit(parent:DiffTypeF
				    text:"Sent diff types counted by type")}
      
      RDLabel ={New Tk.label tkInit(parent:DiffTypeF
				    text:"Received diff types counted by type")}
      
      DsentStat = {New FieldDisplay.fieldDisplayClass
		   open(parent:DiffTypeF width:550 height:14*8)}
      
      DreceivedStat = {New FieldDisplay.fieldDisplayClass
		       open(parent:DiffTypeF width:550 height:14*8)}
      
      {Tk.batch [grid(SSites	row:0 column:0 rowspan:2 sticky:news)
		 grid(SActive	row:0 column:1 sticky:news)
		 grid(SRTT   	row:1 column:1 sticky:news) 

		 grid(OSites	row:0 column:0 rowspan:2 sticky:news)
		 grid(OActive	 row:0 column:1 sticky:news)
		 grid(ONumber	row:1 column:1 sticky:news) 

		 grid(BSites	row:0 column:0 rowspan:2 sticky:news)
		 grid(BActive	row:0 column:1 sticky:news)
		 grid(BNumber	row:1 column:1 sticky:news) 

		 grid(NIList	row:0 column:0 rowspan:2 sticky:news)
		 grid(NINumber	row:0 column:1 sticky:news)
		 grid(NIByte	row:1 column:1 sticky:news) 

		 grid(SMLabel row:0 column:0 sticky:news)
		 grid(MsentStat row:1 column:0 sticky:news)
		 grid(RMLabel row:2 column:0 sticky:news)
		 grid(MreceivedStat row:3 column:0 sticky:news)

		 grid(SDLabel row:0 column:0 sticky:news)
		 grid(DsentStat row:1 column:0 sticky:news)
		 grid(RDLabel row:2 column:0 sticky:news)
		 grid(DreceivedStat row:3 column:0 sticky:news)
		 
		 grid(columnconfigure SiteF 0 weight:1)
		 grid(columnconfigure OwnerF 0 weight:1)
		 grid(columnconfigure BorrowF 0 weight:1)
		 grid(columnconfigure MessageF 0 weight:1)
		 
		 grid(columnconfigure NetInfoF 0 weight:1)

		 grid(rowconfigure SiteF 0 weight:1)
		 grid(rowconfigure OwnerF 0 weight:1)
		 grid(rowconfigure BorrowF 0 weight:1)
		 grid(rowconfigure MessageF 0 weight:1)
		 grid(rowconfigure DiffTypeF 0 weight:1)
		 grid(rowconfigure NetInfoF 0 weight:1)
		 grid(rowconfigure SiteF 1 weight:1)
		 grid(rowconfigure OwnerF 1 weight:1)
		 grid(rowconfigure BorrowF 1 weight:1)
		 grid(rowconfigure MessageF 1 weight:1)
		 grid(rowconfigure DiffTypeF 1 weight:1)
		 grid(rowconfigure NetInfoF 1 weight:1)
		]}

      {CardF addCard(id:1 title:" Communication " frame:SiteF)}
      {CardF addCard(id:2 title:" Exported entities " frame:OwnerF)}
      {CardF addCard(id:3 title:" Imported entities " frame:BorrowF)}
%	 {CardF addCard(id:4 title:" Net Info " frame:NetInfoF)}
      {CardF addCard(id:4 title:" Messages " frame:MessageF)}
      {CardF addCard(id:5 title:" Diff Types " frame:DiffTypeF)}
      {Tk.batch [grid(columnconfigure T 0 weight:1)
		 grid(rowconfigure T 0 weight:1)
		 grid(CardF row:0 column:0 sticky:news)]}
   end
   proc{OpenNetInfo}
      {CardF addCard(id:6 title:" Net Info " frame:NetInfoF)}     
   end

   
   fun{RemoteSiteWin Site Delete}
      Top    = {New Tk.toplevel tkInit(title:Site.ip#":"#Site.port#" pid:"#Site.pid
				       delete:Delete
				      )}
      SSites = {New SiteList tkInit(parent:Top)}
      SActive= {New TitleGraph.titleGraph tkInit(parent:Top
						title:"Message exchange per sample period"
						miny:1.0
						maxy:11.0
						dim:''
						fill:false)}
      SRTT   = {New TitleGraph.titleGraph tkInit(parent:Top
					     title:"Last RTT (ms)"
					     miny:1.0
					     maxy:11.0
					     dim:''
					     fill:false)}
   in
      {Tk.batch [grid(SSites	row:0 column:0 rowspan:2 sticky:news)
		 grid(SActive	row:0 column:1 sticky:news)
		 grid(SRTT   	row:1 column:1 sticky:news)]}
      gui(ssites:SSites sactivity:SActive srtt:SRTT top:Top)
   end
end





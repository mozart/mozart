%%%
%%% Author:
%%%
%%%   Nils Franzen   (nilsf@sics.se)
%%%
%%% Copyright:
%%%
%%%   Nils Franzen,   1998
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    http://mozart.ps.uni-sb.de
%%%
%%% See the file "LICENSE" or
%%%    http://mozart.ps.uni-sb.de/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor 
export
   Start
   Stop
import
   Tk
   Connection(take offerUnlimited)
   Open(file)
   OS
   Browser(browse:Show)
define

   %% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   %% Progress bar
   %% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   class ProgressBar from Tk.canvas
      prop locking
      feat max fg tag
      attr current:0
      meth tkInit(max:M fg:FG<=red ...)=Q
	 Tk.canvas, {Record.subtract {Record.subtract Q fg} max}
	 self.max={IntToFloat M}
	 {self tkBind(event:'<Configure>' action:proc{$} {self progress(@current)} end)}
	 self.tag={New Tk.canvasTag tkInit(parent:self)}
	 {self tk(crea rect 0 0 0 0 fill:FG outline:FG tags:self.tag)}
      end
   
      meth progress(X)
	 lock
	    A=({IntToFloat X}/self.max)*{Tk.returnFloat winfo(width(self))}
	    B={Tk.returnFloat winfo(height(self))}
	 in
	    {self tk(coords self.tag 0 0 A B)} 
	    current<-X
	 end
      end
   end


   %% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   %% File operations
   %% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   proc{OpenFile File Size GetData CloseFile} F in
      Size={OS.stat File}.size
      F={New Open.file init(name:File flags:[read])}
      fun{GetData Bytes} {F read(list:$ size:Bytes)} end
      proc{CloseFile} {F close} end
   end

   proc{SaveFile File SaveData CloseFile} F in
      F={New Open.file init(name:File flags:[write create truncate])}
      proc{SaveData Ls} {F write(vs:Ls)} end
      proc{CloseFile} {F close} end
   end


   %% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   %% Sending
   %% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   proc{SendFile File P Meta}
      GetData Size CloseFile CloseWindow
      Progress PC={NewCell 0} 
      PacketsCount={NewCell 0}
      proc{Transfer} O N in
	 {Exchange PacketsCount O N}
	 if O>1000 then
	    N=O
	    {Delay 1000}
	    {Transfer}
	 else  A Ds={GetData 1024} in
	    if Ds\=nil then
	       N=O+1
	       {Send P {VirtualString.toByteString Ds}#A}
	       thread L={Length Ds} O N in
		  {Wait A}
		  {Progress L}
		  {Exchange PacketsCount O N}
		  N=O-1
	       end
	       {Transfer}
	    else
	       proc{W}
		  if {Access PacketsCount}>0 then
		     {Delay 1000}
		     {W}
		  end
	       end
	    in
	       O=N
	       {W}
	    end
	 end
      end
   in
      {OpenFile File Size GetData CloseFile}
      local
	 T={New Tk.toplevel tkInit(title:"Sending File")}
	 FL={New Tk.label tkInit(parent:T text:"File Name:")}
	 FE={New Tk.entry tkInit(parent:T textvariable:{New Tk.variable tkInit(File)})}
	 SL={New Tk.label tkInit(parent:T text:"File Size:")}
	 SE={New Tk.entry tkInit(parent:T textvariable:{New Tk.variable tkInit(Size)})}
	 PB={New ProgressBar tkInit(parent:T max:Size bd:2 relief:sunken width:50 height:20 fg:red)}
      in
	 {Tk.batch [grid(FL row:3 column:0 sticky:e)
		    grid(FE row:3 column:1 sticky:we)
		    grid(SL row:5 column:0 sticky:e)
		    grid(SE row:5 column:1 sticky:we)
		    grid(PB row:8 column:0 columnspan:2 sticky:we)
		    grid(columnconfigure T 1 weight:1)
		   ]}
	 CloseWindow=proc{$} {Delay 1000} {T tkClose} end
	 Progress=proc{$ X} O N in
		     {Exchange PC O N}
		     N=O+X
		     {PB progress(N)}
		  end
      end
      {Send P Meta#size(Size)}
      {Transfer}
      {CloseFile}
      {Send P Meta#done}
      {CloseWindow}
   end


   %% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   %% Receiving
   %% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   proc{ReceiveFile File P Meta} SaveData CloseFile in
      Meta={NewName}
      thread
	 proc{OpenWindow SetSize CloseWindow Progress}
	    T={New Tk.toplevel tkInit(title:"Saving File")}
	    FL={New Tk.label tkInit(parent:T text:"Saving File:")}
	    FE={New Tk.entry tkInit(parent:T textvariable:{New Tk.variable tkInit(File)})}
	    SV={New Tk.variable tkInit("UNKNOWN")}
	    SL={New Tk.label tkInit(parent:T text:"File Size:")}
	    SE={New Tk.entry tkInit(parent:T textvariable:SV)}
	    PB PC={NewCell 0}
	 in
	    {Tk.batch [grid(FL row:3 column:0 sticky:e)
		       grid(FE row:3 column:1 sticky:we)
		       grid(SL row:5 column:0 sticky:e)
		       grid(SE row:5 column:1 sticky:we)
		       grid(columnconfigure T 1 weight:1)
		      ]}
	    CloseWindow=proc{$} {Delay 1000} {T tkClose} end
	    Progress=proc{$ X} O N in
			{Exchange PC O N}
			N=O+X
			{PB progress(N)}
		     end
	    SetSize=proc{$ S}
		       {SV tkSet(S)}
		       PB={New ProgressBar tkInit(parent:T max:S bd:2 relief:sunken width:50 height:20 fg:red)}
		       {Tk.send grid(PB row:8 column:0 columnspan:2 sticky:we)}
		    end
	 end
	 SetSize CloseWindow Progress
      in
	 {SaveFile File SaveData CloseFile}
	 {OpenWindow SetSize CloseWindow Progress}
	 try	    
	    {ForAll {NewPort $ P} proc{$ X}
				     if X.1==Meta then
					if X.2==done then
					   raise done end
					elseif {Label X.2}==size then
					   {SetSize X.2.1}
					end
				     else %Ss={ByteString.toString X.1} in
					{SaveData X.1} %Ss}
					thread X.2=unit end
					{Progress {ByteString.length X.1}} %{Length Ss}}
				     end
				  end}
	 catch done then skip end
	 {CloseWindow}
	 {CloseFile}
      end
   end

   %% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   %% ServerSide Class
   %% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   class ServerSide
      feat cont user
      attr cs:nil
      meth init(user:U)
	 T={New Tk.toplevel tkInit(title:"File Transfer ("#U.name#")")}
	 F1={New Tk.frame tkInit(parent:T)}
	 Q={New Tk.button tkInit(parent:T text:"Halt File Transfer"
				 action:proc{$}
					   {ForAll @cs proc{$ P}
							  {Send P quit}
						       end}
					   {T tkClose}
					end)}
      in
	 {Tk.send pack(F1 Q fill:x)}
	 self.cont=F1
	 self.user=U
      end

      meth addUser(user:U port:P)
	 Str="Send File to "#U.name#"..."
	 B={New Tk.button tkInit(parent:self.cont text:Str
				 action:proc{$}
					   {B tk(config state:disabled)}
					   SrcF={Tk.return tk_getOpenFile(title:"Send File to "#U.name
									  filetypes:q(q('All Files' '*')
										      q('Text Files' q('.txt'))
										      q('Oz Files' q('.oz'))
										     )
									 )}
					in
					   if SrcF\=nil then
					      Sync P1 Done
					      F={Reverse {List.takeWhile {Reverse SrcF}
							  fun{$ C} C\=&/ andthen C\=&\\ end}}
					      N=self.user.name
					   in
					      {Send P filetransfer(Sync port:P1 done:Done file:F sender:N)}
					      {B tk(configure text:"Waiting for client response... ("#U.name#")")}
					      {Wait Sync}
					      if Sync==true then
						 thread
						    {SendFile SrcF P1 Done}
						 end
					      end					      
					      {B tk(configure text:Str)}
					   end
					   {B tk(config state:normal)}
					end)}
	 O N
      in
	 O=cs<-N
	 N=P|O
	 {Tk.send pack(B fill:x)}
      end
   end

   proc {Start User Ticket}
      if {IsDet Ticket}==false then
	 {StartServer User Ticket}
      else
	 {StartClient User Ticket}
      end
   end

   proc {Stop}
      skip
   end
	
   proc {StartServer User Ticket}
      S P A={New ServerSide init(user:User)}
   in
      {NewPort S P}
      %_={New Connection.gate init(P Ticket)}
      {Connection.offerUnlimited P Ticket}
   
      thread
	 {ForAll S proc{$ X} {A X} end}
      end
   end
   
   proc {StartClient User Ticket}
      S P RP={Connection.take Ticket}
   in
      {NewPort S P}
      {Send RP addUser(user:User port:P)}
      thread
	 {ForAll S proc{$ X}
		      case {Label X} of filetransfer then
			 DestF={Tk.return tk_getSaveFile(title:"Save File from "#X.sender#"..."
							 initialfile:X.file
							 filetypes:q(q('All Files' '*')
								     q('Text Files' q('.txt'))
								     q('Oz Files' q('.oz'))
								    )
							)}
		      in
			 if DestF==nil then
			    X.1=false
			 else
			    X.1=true
			    thread
			       {ReceiveFile DestF X.port X.done}
			    end
			 end
		      elseof quit then
			 skip
			 {Show quit}
		      else
			 skip
			 {Show unknownLabel(X)}
		      end
		   end}
      end
   end

in
   skip
end

/*
local
   Ticket
   U1=user(id:nilsf name:"Nils Franzen")
   U2=user(id:seif name:"Seif Haridi")
   U3=user(id:erik name:"Erik Klintskog")
in
   {Start U1 Ticket}

   {Delay 2000}
   {Start U2 Ticket}

   {Delay 2000}
   {Start U3 Ticket}
end
*/

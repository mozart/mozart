%%%
%%% Authors:
%%%   Christian Schulte <schulte@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Christian Schulte, 1998
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
   OS
   Pickle
   Connection
   Application
   Abstractions

prepare
   
   BaseDir = '/tmp/'

   IDB = [t(fr:'Saarbrücken' to:'London'        price:576)
	  t(fr:'Saarbrücken' to:'Miami'         price:345)
	  t(fr:'Saarbrücken' to:'San Francisco' price:812)
	  t(fr:'Saarbrücken' to:'Hong Kong'     price:640)
	  t(fr:'Saarbrücken' to:'Hamburg'       price:245)
	  t(fr:'Saarbrücken' to:'Pittsburgh'    price:560)
	  t(fr:'Saarbrücken' to:'Munich'        price:135)
	  t(fr:'Saarbrücken' to:'Stockholm'     price:420)]

   
   LogWidth  = 50
   LogHeight = 8
   LogFont   = '-adobe-helvetica-medium-r-normal-*-*-140*'
   LogBg     = '#fffff0'

   fun {FindJourney Js JID}
      case Js of nil then false
      [] J|Jr then
	 J.id==JID orelse {FindJourney Jr JID}
      end
   end
   
   fun {RemoveJourney Js JID}
      case Js of nil then nil
      [] J|Jr then
	 if J.id==JID then Jr else J|{RemoveJourney Jr JID} end
      end
   end

   ArgSpec = record(url(single
			type:    string
			default: BaseDir # 'ticket.ozp')
		    db(single
		       type:     string
		       default:  BaseDir # 'tdb.ozp')
		    title(single
			  type:  string
			  default: 'Last Minute Flights Server'))


define
   
   class LogWindow from Tk.text
		      
      meth init(title:Title delete:Delete)
	 W = {New Tk.toplevel tkInit(title:  Title
				     delete: Delete)}
	 S = {New Tk.scrollbar tkInit(parent:W width:10)}
      in
	 LogWindow, tkInit(parent:             W
			   width:              LogWidth
			   height:             LogHeight
			   bg:                 LogBg
			   font:               LogFont
			   highlightthickness: 0)
	 {Tk.addYScrollbar self S}
	 {Tk.send pack(self S side:left fill:y)}
      end
      meth print(V)
	 LogWindow,tk(insert 'end' V)
	 LogWindow,tk(yview pickplace:'end')
      end
      meth show(V)
	 LogWindow,print(V#'\n')
      end
      meth clear
	 LogWindow,tk(delete '1.0' 'end')	    
      end
   end
   
   class TravelServer
      
      feat
	 DbFile
	 Text
	 
      attr
	 Id: 0
	 
	 
      meth init(ThisDbFile Delete)
	 self.DbFile = ThisDbFile
	 self.Text   = {New LogWindow init(title:    'Travel Server'
					   delete:   Delete)}
      end
      
      
      meth request(From $)
	 {self.Text show('Request: '#if From==false then anonymous
				     else From
				     end)}
	 try
	    {Pickle.load self.DbFile}
	 catch _ then nil
	 end
      end
      
      meth book(By JID $)
	 try
	    DB = {Pickle.load self.DbFile}
	 in
	    if {FindJourney DB JID} then
	       NDB = {RemoveJourney DB JID}
	    in
	       {OS.unlink self.DbFile}
	       {Pickle.save NDB self.DbFile}
	       {self.Text show('Booked: '#JID#' by: '#
			       By.firstname#' '#
			       By.lastname#' <'#By.email#'>')}
	       true
	    else
	       false
	    end
	 catch _ then false
	 end
      end
      
      meth add(J $)
	 try
	    DB    = {Pickle.load self.DbFile}
	    I     = @Id
	    NewDB = {AdjoinAt J id I}|DB
	 in
	    Id <- I + 1
	    {Pickle.save NewDB self.DbFile}
	    {self.Text show('Added: '#I)}
	    true
	 catch _ then false
	 end
      end
      
   end
   

   Argv = {Application.getCmdArgs ArgSpec}
   
   %% Create empty data base
   {Pickle.save nil Argv.db}

   %% Start server
   Server = {Abstractions.newServer
	     TravelServer init(Argv.db
			       proc {$}
				  {OS.unlink Argv.db}
				  {OS.unlink Argv.url}
				  {Application.exit 0}
			       end)}

   %% Add initial flights
   
   {ForAll IDB proc {$ J}
		  {Server add(J _)}
	       end}
   
   Gate   = {New Connection.gate init(Server)}
   Ticket = {Gate getTicket($)}
   
   %% Write ticket to url
   {Pickle.save Ticket Argv.url}

end

%%%
%%% Authors:
%%%   Nils Franzén (nilsf@sics.se)
%%%   Simon Lindblom (simon@sics.se)
%%%
%%% Copyright:
%%%   Nils Franzén, 1998
%%%   Simon Lindblom, 1998
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
   Application(getCmdArgs exit)
   OS(system)
   System(showInfo)
define
   Spec=record('ticket'(single char:&t type:string default:"")
	       'dir'(single char:&d type:string default:""))

   proc {Start Args No}
      {System.showInfo "Server is started. Try: "#No#"\n"}
      try
	 Status =  {OS.system "./micq -q "#Args}
      in
	 {System.showInfo "Server stopped. Status: "#Status#"\n"}
	 if Status \= 0 then {Start Args No+1} end
      catch _ then
	 {System.showInfo "Supervisor Exception\n"}
	 {Start Args No+1}
      end
   end   
in
   try
      Args = {Application.getCmdArgs Spec}
      Ticket Dir
   in
      if Args.ticket \= "" then Ticket=" -t "#Args.ticket
      else Ticket = "" end
      if Args.dir \= "" then Dir=" -d "#Args.dir
      else Dir = "" end
	 
      {Start Ticket#Dir 1}
   catch _ then
      {System.showInfo "Supervisor Exception. Supervisor Halted.\n"}
   end
   {Application.exit 0}
end

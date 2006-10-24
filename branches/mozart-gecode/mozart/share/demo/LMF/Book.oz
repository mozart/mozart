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
   Abstractions
   Application
   OS
   HTML
   System

prepare

   Url = "/tmp/ticket.ozp"
   
   ArgSpec = record(url(single type:string default:"/tmp/ticket.ozp"))

   
define

   Argv = {Application.getCgiArgs ArgSpec}

%   {System.show url(Argv)}
   
   {HTML.out
    'Last Minute Flights'
    try
       form(action: 'lmf-book.cgi'
	    method: 'GET'
	    h2('Personal Data')
	    table(tr(td(label(for:firstname 'First name: '))
		     td(input(type:text name:firstname)))
		  tr(td(label(for:lastname  'Last name: '))
		     td(input(type:text name:lastname)))
		  tr(td(label(for:email     'E-Mail: '))
		     td(input(type:text name:email))))
	    h2('Available Flights')
	    local
	       SE = {Abstractions.connect Url}
	       
	       Fs = {SE request({OS.getEnv 'REMOTE_HOST'} $)}
	       N  = {Length Fs} div 2
	       As = {List.drop Fs N}
	       Bs = {List.take Fs N}
	       
	       fun {MkTable Fs}
		  table(cellpadding:2 border:1 bgcolor:'#ffe0e0'
			tr(th th('From') th('To') th('Price'))
			{List.toTuple '#'
			 {Map Fs
			  fun {$ J}
			     tr(td(input(type:  radio
					 name:  journey
					 value: J.id))
				td(J.fr) td(J.to) td(J.price))
			  end}})
	       end
	       
	    in
	       table(width:'70%'
		     tr(valign:top
			td({MkTable As})
			td({MkTable Bs})))
	    end
	    h2('Process')
	    table(tr(td(input(type:submit value:'Book'))
		     td(input(type:reset  value:'Reset'))))
	    h2('Server')
	    table(tr(th('URL')
		     td(input(type:hidden name:url value:Argv.url)
			Argv.url))))
    catch E then
       font(size:'+2' color:red
	    {Value.toVirtualString E 100 100})
%	    'Sorry, could not connect to server')
    end}
   
   {Application.exit 0}
	 
end


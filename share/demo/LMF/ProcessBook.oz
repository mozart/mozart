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
   HTML
   Abstractions
   Application
   
prepare

   ArgSpec = record(journey(single type:string default:"0")
		    firstname(single type:string)
		    lastname(single type:string)
		    email(single type:string)
		    url(single type:string))

define

   Argv = {Application.getCgiArgs ArgSpec}

   SE      = {Abstractions.connect Argv.url}

   WhoAmI  = customer(firstname: Argv.firstname
		      lastname:  Argv.lastname
		      email:     Argv.email)

   Journey = {String.toInt Argv.journey}

   {HTML.out
    'Flight Information'
    if {SE book(WhoAmI Journey $)} then
       font(size:'+2' color:green
	    'Flight Successfully Booked')
    else
       font(size:'+2' color:red
	    'Sorry, Could not Book Flight')
    end}
	 
   {Application.exit 0}
	 
end

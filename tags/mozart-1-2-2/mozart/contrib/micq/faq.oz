%%%
%%% Authors:
%%%   Nils Franzén (nilsf@sics.se)
%%%
%%% Copyright:
%%%   Nils Franzén, 1998
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
require
   DefSettings(url:URL) at 'defaultsettings.ozf'
   Meths(getFAQ:S_getFAQ) at 'methods.ozf'
import
   Application(exit:Exit getCgiArgs:GetArgs)
   HTML(out:Out)
   Pickle
   Connection
   OS
define
   /*
   [store(data:faq(answer:[79 107 44 32 100 111 32 108 105 107 101 32 116 104 105 115 46 46 46 46 10 10]
		   date:date(date:17#[47]#([48]#3) time:15#[58]#19 year:1999)
		   poster:nilsf
		   question:[72 101 108 108 111 44 32 104 101 108 112 32 109 101 33 10 80 108 101 97 115 101 33 10 10])
	  id:48)]
   */
   
   proc{SendQuestion _ Body}
      Host=local A={OS.getEnv 'REMOTE_HOST'} in
	      if A==false then "<Unknown Host>" else A end
	   end
      B Fs1={{Connection.take {Pickle.load URL}} S_getFAQ($ host:Host)}
      Fs={Sort Fs1 fun{$ N O} N.id > O.id end}
   in
      B=body("\n" h1('class':title "Posted FAQ's") "\n" hr br
	     {List.toRecord '#' {List.mapInd Fs fun{$ I X}
						   try
						      I#'#'(table(width:"80%"
								  "\n"
								  tr(th(align:left
									"Posted by " X.data.poster " "
									X.data.date.date "-"
									X.data.date.year ", " X.data.date.time))
								  "\n"
								  tr(td("Question:" br X.data.question))
								  "\n"
								  tr(td("Answer:" br X.data.answer))
								 ) "\n"
							    br hr br
							   )
						   catch _ then
						      I#p("Error in Data" "\n")
						   end
						end}}
	     "\n"
	     em("Generated on the fly by Mozart Instant Messenger") "\n"
	    )
      Body=html(head("\n" title("Frequently Asked Questions")
		     '\n<LINK href="http://www.sics.se/mozart/stylesheets/doc.css" rel="stylesheet" type="text/css">\n'
		    )
		"\n" B "\n"
	       )
   end
   
   proc {ExecCgi}
      Args Body
   in
      try
	 Args={GetArgs record(login(single type:string default:"all"))}
	 {SendQuestion Args Body}
      catch XX then
	 Body = {Value.toVirtualString XX 20 20}
      end
      
      try
	 {HTML.out Body}
      catch _ then 
	 skip
      end
   end
in
   {ExecCgi}
   {Exit 0}
end      




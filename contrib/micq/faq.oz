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
%   System
   Pickle
   Connection
%   Browser
define
   /*
   [store(data:faq(answer:[79 107 44 32 100 111 32 108 105 107 101 32 116 104 105 115 46 46 46 46 10 10]
                   date:date(date:17#[47]#([48]#3) time:15#[58]#19 year:1999)
                   poster:nilsf
                   question:[72 101 108 108 111 44 32 104 101 108 112 32 109 101 33 10 80 108 101 97 115 101 33 10 10])
          id:48)]
   */

   proc{SendQuestion _ Body}
      B Fs1={{Connection.take {Pickle.load URL}} S_getFAQ($)}
      Fs={Sort Fs1 fun{$ N O} N.id < O.id end}
   in
      B=body(%text:"#000000" bgcolor:"#FFFFFF" link:"#0000EE" vlink:"#551A8B" alink:"#FF0000"
             p("Posted FAQ's")
%            {Value.toVirtualString Fs 1000 1000}
             {List.toRecord p {List.mapInd Fs fun{$ I X}
                                                 try
                                                    I#p(table(width:"100%" border:2 %bgcolor:"#ffffff"
                                                              tr(th(align:left %colspan:2
%                                                                   "Posted by " X.data.poster " "
                                                                    X.data.date.date "-"
                                                                    X.data.date.year ", " X.data.date.time))
                                                              tr(%bgcolor:"#ffff00"
                                                                 td("Question:" br X.data.question))
                                                              tr(%bgcolor:"#ffcc00"
                                                                 td("Answer:" br X.data.answer))
                                                             )
                                                       )
                                                 catch _ then
                                                    I#p("Error in Data")
                                                 end
                                              end}}
             hr
             em("Generated on the fly by Mozart Instant Messenger")
            )
      Body=B
   end

   proc {ExecCgi}
      Args Title Body
   in
      try
         Args={GetArgs record(login(single type:string default:"all"))}
         {SendQuestion Args Body}
         Title = "Frequently Asked Questions"
      catch XX then
         Title = "Error"
         Body = {Value.toVirtualString XX 20 20}
      end

      try
         {HTML.out Title Body}
      catch _ then
         skip
      end
   end
in
   {ExecCgi}
   {Exit 0}
end

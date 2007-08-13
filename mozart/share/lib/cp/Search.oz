%%
%%% Authors:
%%%   Christian Schulte <schulte@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Christian Schulte, 1997
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
   ParSearch(engine: ParallelEngine) at 'x-oz://system/ParSearch.ozf'
   Space
   System
export
   one:      OneModule
   all:      All
   allS:     AllS
   allP:     AllP
   best:     BestModule
   object:   SearchObject
   base:     SearchBase
   parallel: ParallelEngine

prepare

   %% General help routines
   proc {NewKiller ?Killer ?KillFlag}
      proc {Killer}
	 KillFlag=kill
      end
   end

   %%
   %% Different wrappers for creation of output
   %%
   fun {WrapS S}
      S
   end

define
   
   fun {WrapP S}
      proc {$ X}
	 {Space.merge {Space.clone S} X}
      end
   end

   %%
   %% Make copy of space and recompute choices
   %%
   local
      proc {ReDo Is C}
	 case Is of nil then skip
	 [] I|Ir then
	    {ReDo Ir C}
	    {Space.commitB C I}
	 end
      end
   in
      proc {Recompute S Is C}
	 C={Space.clone S} {ReDo Is C}
      end
   end

   %%
   %% A Space.commit2 replacement
   %%
   proc {SPCommit2 S BL}
      {Space.inject S proc {$ _}
			 {Space.branch BL}
		      end}
   end

   
   %%
   %% Injection of solution constraints for best solution search
   %%
   proc {Better S O SS}
      CS
   in
      CS={Space.clone SS}
      
      {Space.inject S proc {$ X} {O {Space.merge CS} X} end}
   end
   
   %%
   %% The one solution search module
   %%

   fun {OneDepthNR KF S}
      if {IsFree KF} then
	 case {Space.ask S}
	 of failed then nil
	 [] succeeded then S
	 [] branch([B]) then
	    {Space.commitB S B}
	    {OneDepthNR KF S}
	 [] branch(B|Br) then C={Space.clone S} in
	    {Space.commitB S B}
	    case {OneDepthNR KF S}
	    of nil then {SPCommit2 C Br} {OneDepthNR KF C}
	    elseof O then O
	    end
	 end
      else nil
      end
   end
      
   local

      %% KF : killer function. All will work until this var gets det.
      %% I|M: List of branchings (alternatives).
      %% S  : Current space.
      %% MRD: Maximum recomputation distance.
      fun {AltCopy KF I|M S MRD}
	 if M==nil then
	    {Space.commitB S I}
	    {OneDepthR KF S S nil MRD MRD}
	 else C={Space.clone S} in
	    {Space.commitB C I}
	    case {OneDepthR KF C S [I] 1 MRD}
	    of nil then {AltCopy KF M S MRD}
	    elseof O then O
	    end
	 end
      end

      %% KF : killer function. All will work until this var gets det.
      %% I|M: List of branchings (alternatives).
      %% S  : Current space.
      %% C  : Clone Space.
      %% As : A reversed list with the alternatives taken to reach S.
      %% RD : Current recomputation distance.
      %% MRD: Maximum recomputation distance.
      fun {Alt KF I|M S C As RD MRD}
	 {Space.commitB S I}
	 if M==nil then {OneDepthR KF S C I|As RD MRD}
	 else
	    case {OneDepthR KF S C I|As RD MRD}
	    of nil then S={Recompute C As} in
	       {Alt KF M S C As RD MRD}
	    elseof O then O
	    end
	 end
      end

   in
      fun {OneDepthR KF S C As RD MRD}
	 if {IsFree KF} then
	    case {Space.ask S}
	    of failed    then nil
	    [] succeeded then S
	    [] branch(M) then
	       if RD==MRD then
		  {AltCopy KF M S MRD}
	       else
		  {Alt KF M S C As RD+1 MRD}
	       end
	    end
	 else nil
	 end
      end
   end

   local
      fun {OneDepth P MRD ?KP}
	 KF={NewKiller ?KP} S={Space.new P}
      in
	 if MRD==1 then {OneDepthNR KF S}
	 else {OneDepthR KF S S nil MRD MRD}
	 end
      end
   
      local
	 %% KF : killer function. All will work until this var gets det.
	 %% I|M: List of branchings (alternatives).
	 %% S  : Current space.
	 %% CD : Go as deep as CD spaces looking for a solution.
	 %% MRD: Maximum recomputation distance.
	 fun {AltCopy KF I|M S CD MRD CO}
	    if M==nil then
	       {Space.commitB S I}
	       {OneBoundR KF S S nil CD MRD MRD CO}
	    else
	       C={Space.clone S}
	       {Space.commitB C I}
	       O={OneBoundR KF C S [I] CD 1 MRD CO}
	    in
	       if {Space.is O} then O
	       else {AltCopy KF M S CD MRD O}
	       end
	    end
	 end
	 
	 %% KF   : Killer funtion. All will work until this var gets det.
	 %% I|M: List of branchings (alternatives).
	 %% S  : Current space.
	 %% C  : Clone Space.
	 %% As : A reversed list with the alternatives taken to reach S.
	 %% CD : Go as deep as CD spaces looking for a solution.
	 %% RD : Current recomputation distance.
	 %% MRD: Maximum recomputation distance.
	 fun {Alt KF I|M S C As CD RD MRD CO}
	    {Space.commitB S I}
	    if M==nil then {OneBoundR KF S C I|As CD RD MRD CO}
	    else O={OneBoundR KF S C I|As CD RD MRD CO} in
	       if {Space.is O} then O
	       else S={Recompute C As} in
		  {Alt KF M S C As CD RD MRD O}
	       end
	    end
	 end

	 fun {OneBoundR KF S C As CD RD MRD CO}
	    if {IsFree KF} then
	       case {Space.ask S}
	       of failed    then CO
	       [] succeeded then S
	       [] branch(M) then
		  if CD=<0 then cut
		  elseif RD==MRD then {AltCopy KF M S CD-1 MRD CO}
		  else {Alt KF M S C As CD-1 RD+1 MRD CO}
		  end
	       end
	    else nil
	    end
	 end

      	 fun {OneIterR KF S CD MRD}
	    if {IsFree KF} then C={Space.clone S} in
	       case {OneBoundR KF C C nil CD MRD MRD nil}
	       of cut then {OneIterR KF S CD+1 MRD}
	       elseof O then O
	       end
	    else nil
	    end
	 end
      in

	 fun {OneBound P MD MRD ?KP}
	    S={Space.new P}
	 in
	    {OneBoundR {NewKiller ?KP} S S nil MD MRD MRD nil}
	 end

	 fun {OneIter P MRD ?KP}
	    {OneIterR {NewKiller ?KP} {Space.new P} 1 MRD}
	 end
      end      

      local
	 proc {Probe S D KF}
	    if {IsDet KF} then
	       raise killed end
	    else
	       case {Space.ask S}
	       of failed then skip
	       [] succeeded then
		  raise succeeded(S) end
	       [] branch(B|Bs) then
		  if D==0 then
		     {Space.commitB S B} {Probe S 0 KF}
		  else C={Space.clone S} in
		     {SPCommit2 S Bs} {Probe S D-1 KF}
		     {Space.commitB C B} {Probe C D KF}
		  end
	       end
	    end
	 end

	 proc {Iterate S D M KF}
	    if M==D then {Probe S D KF} else
	       {Probe {Space.clone S} D KF} {Iterate S D+1 M KF}
	    end
	 end
      in
	 proc {LDS P D ?KP}
	    {Iterate {Space.new P} 0 D {NewKiller ?KP}} 
	 end
      end

   in

      OneModule = one(depth:    fun {$ P MRD ?KP}
				   case {OneDepth P MRD ?KP}
				   of nil then nil
				   elseof S then [{Space.merge S}]
				   end
				end
		      depthP:   fun {$ P MRD ?KP}
				   case {OneDepth P MRD ?KP}
				   of nil then nil
				   elseof S then [{WrapP S}]
				   end
			        end
		      depthS:   fun {$ P MRD ?KP}
				   case {OneDepth P MRD ?KP}
				   of nil then nil
				   elseof S then [S]
				   end
			        end
      
		      bound:    fun {$ P MD MRD ?KP}
				   case {OneBound P MD MRD ?KP}
				   of nil then nil
				   [] cut then cut
				   elseof S then [{Space.merge S}]
				   end
				end
		      boundP:   fun {$ P MD MRD ?KP}
				   case {OneBound P MD MRD ?KP}
				   of nil then nil
				   [] cut then cut
				   elseof S then [{WrapP S}]
				   end
				end
		      boundS:   fun {$ P MD MRD ?KP}
				   case {OneBound P MD MRD ?KP}
				   of nil then nil
				   [] cut then cut
				   elseof S then [S]
				   end
				end
      
		      iter:     fun {$ P MRD ?KP}
				   case {OneIter P MRD ?KP}
				   of nil then nil
				   elseof S then [{Space.merge S}]
				   end
				end
		      iterP:    fun {$ P MRD ?KP}
				   case {OneIter P MRD ?KP}
				   of nil then nil
				   elseof S then [{WrapP S}]
				   end
				end
		      iterS:    fun {$ P MRD ?KP}
				   case {OneIter P MRD ?KP}
				   of nil then nil
				   elseof S then [S]
				   end
				end
		      
		      lds:      fun {$ P D ?KP}
				   try {LDS P D ?KP} nil
				   catch killed then nil
				   [] succeeded(S) then [{Space.merge S}]
				   end
				end
		      ldsP:     fun {$ P D ?KP}
				   try {LDS P D ?KP} nil
				   catch killed then nil
				   [] succeeded(S) then [{WrapP S}]
				   end
				end
		      ldsS:     fun {$ P D ?KP}
				   try {LDS P D ?KP} nil
				   catch killed then nil
				   [] succeeded(S) then [S]
				   end
				end
		     )

   end

   %%
   %% The all solution search module
   %%
   local
      %% KF   :  killer function. All will work until this var gets det.
      %% S    :  Current space for search.
      %% W    :  Function to apply on each soultion when it is found.
      %% Or   :  Current list of found solutions.
      %% Os   :  Return param. (list of solutions).
      proc {AllNR KF S W Or Os}
	 if {IsFree KF} then
	    case {Space.ask S}
	    of failed then Os=Or
	    [] succeeded then Os={W S}|Or
	    [] branch([B]) then
	       {Space.commitB S B}
	       Os = {AllNR KF S W Or}
	    [] branch(B|Br) then C={Space.clone S} Ot in
	       {Space.commitB S B} {SPCommit2 C Br}
	       Os={AllNR KF S W Ot}
	       Ot={AllNR KF C W Or}
	    end
	 else Os=Or
	 end
      end

      local
	 %% KF   :  killer function. All will work until this var gets det.
	 %% I|M  :  List of branchings (alternatives).
	 %% S    :  Current space.
	 %% W    :  Function to apply on each soultion when it is found.
	 %% Or   :  Current list of found solutions.
	 %% Os   :  Return param. (list of solutions).
	 proc {AltCopy KF I|M S MRD W Or Os}
	    if M==nil then
	       {Space.commitB S I}
	       {AllR KF S S nil MRD MRD W Or Os}
	    else C={Space.clone S} Ot in
	       {Space.commitB C I}
	       Os={AllR KF C S [I] 1 MRD W Ot}
	       Ot={AltCopy KF M S MRD W Or}
	    end
	 end

	 %% KF   :  killer function. All will work until this var gets det.
	 %% I|M  :  List of branchings (alternatives).
	 %% S    :  Current space.
	 %% C    :  Clone
	 %% As   :  Recomputation list.
	 %% RD   :  Recomputation distance.
	 %% MRD  :  Max. recomputation distance
	 %% W    :  Function to apply on each soultion when it is found.
	 %% Or   :  Current list of found solutions.
	 %% Os   :  Return param. (list of solutions).
	 proc {Alt KF I|M S C As RD MRD W Or Os}
	    {Space.commitB S I}
	    if M==nil then
	       {AllR KF S C I|As RD MRD W Or Os}
	    else Ot NewS={Recompute C As} in
	       Os={AllR KF S C I|As RD MRD W Ot}
	       Ot={Alt KF M NewS C As RD MRD W Or}
	    end
	 end
      in
	 fun {AllR KF S C As RD MRD W Or}
	    if {IsFree KF} then
	       case {Space.ask S}
	       of failed    then Or
	       [] succeeded then {W S}|Or
	       [] branch(M) then
		  if RD==MRD then {AltCopy KF M S MRD W Or}
		  else {Alt KF M S C As RD+1 MRD W Or}
		  end
	       end
	    else Or
	    end
	 end
      end
   in
      fun {All P MRD ?KP}
	 KF={NewKiller ?KP} S={Space.new P}
      in
	 if MRD==1 then {AllNR KF S Space.merge nil}
	 else {AllR KF S S nil MRD MRD Space.merge nil}
	 end
      end
      
      fun {AllS P MRD ?KP}
	 KF={NewKiller ?KP} S={Space.new P}
      in
	 if MRD==1 then {AllNR KF S WrapS nil}
	 else {AllR KF S S nil MRD MRD WrapS nil}
	 end
      end

      fun {AllP P MRD ?KP}
	 KF={NewKiller ?KP} S={Space.new P}
      in
	 if MRD==1 then {AllNR KF S WrapP nil}
	 else {AllR KF S S nil MRD MRD WrapP nil}
	 end
      end
   end
   

   %%
   %% The best solution search module
   %%

   local

      local
	 %% KF : killer function. All will work until this var gets det.
	 %% S  : Current search space
	 %% O  : Optimization procedure
	 %% SS : Best solution found so far
	 fun {BABNR KF S O SS}
	    if {IsFree KF} then
	       case {Space.ask S}
	       of failed then SS
	       [] succeeded then S
	       [] branch([B]) then
		  {Space.commitB S B}
		  {BABNR KF S O SS}
	       [] branch(B|Bs) then C={Space.clone S} NewSS in
		  {Space.commitB S B} {SPCommit2 C Bs}
		  NewSS={BABNR KF S O SS}
		  if SS==NewSS then {BABNR KF C O SS}
		  elseif NewSS==nil then nil
		  else {Better C O NewSS} {BABNR KF C O NewSS}
		  end
	       end
	    else nil
	    end
	 end
	 local
	    fun {AltCopy KF I|M S MRD O SS}
	       if M==nil then
		  {Space.commitB S I}
		  {BABR KF S S nil MRD MRD O SS}
	       else C={Space.clone S} NewSS in
		  {Space.commitB C I}
		  NewSS = {BABR KF C S [I] 1 MRD O SS}
		  if NewSS==SS then
		     {AltCopy KF M S MRD O SS}
		  elseif NewSS==nil then nil
		  else
		     {SPCommit2 S M}% {Space.commit2 S I+1 M}
		     {Better S O NewSS}
		     {BABR KF S S nil MRD MRD O NewSS}
		  end
	       end
	    end
	 
	    fun {Alt KF I|M S C As RD MRD O SS}
	       {Space.commitB S I}
	       if M==nil then
		  {BABR KF S C I|As RD MRD O SS}
	       else
		  NewSS = {BABR KF S C I|As RD MRD O SS}
	       in
		  if NewSS==SS then
		     {Alt KF M {Recompute C As} C As RD MRD O SS}
		  elseif NewSS==nil then nil
		  else NewS={Recompute C As} in
		     {SPCommit2 NewS M}%{Space.commit2 NewS I+1 M}
		     {Better NewS O NewSS}
		     {BABR KF NewS NewS nil MRD MRD O NewSS}
		  end
	       end
	    end
	 in
	    fun {BABR KF S C As RD MRD O SS}
	       if {IsFree KF} then
		  case {Space.ask S}
		  of failed    then SS
		  [] succeeded then S
		  [] branch(M) then
		     if RD==MRD then {AltCopy KF M S MRD O SS}
		     else {Alt KF M S C As RD+1 MRD O SS}
		     end
		  end
	       else nil
	       end
	    end
	 end
	 
      in
	 fun {BestBAB P O MRD ?KP}
	    KF={NewKiller ?KP} S={Space.new P}
	 in
	    if MRD==1 then {BABNR KF S O nil}
	    else {BABR KF S S nil MRD MRD O nil}
	    end
	 end
      end

      local
	 fun {RestartNR KF S O PS}
	    if {IsFree KF} then C={Space.clone S} in
	       case {OneDepthNR KF S}
	       of nil then PS
	       elseof S then {Better C O S} {RestartNR KF C O S}
	       end
	    else nil
	    end
	 end

	 fun {RestartR KF S O PS MRD}
	    if {IsFree KF} then C={Space.clone S} in
	       case {OneDepthR KF S S nil MRD MRD}
	       of nil then PS
	       elseof S then {Better C O S} {RestartR KF C O S MRD}
	       end
	    else nil
	    end
	 end
      in
	 fun {BestRestart P O MRD ?KP}
	    KF={NewKiller ?KP} S={Space.new P}
	 in
	    if MRD==1 then {RestartNR KF S O nil}
	    else {RestartR KF S O nil MRD}
	    end
	 end
      end
      

   in

      BestModule = best(bab:      fun {$ P O MRD ?KP}
				     case {BestBAB P O MRD ?KP}
				     of nil then nil
				     elseof S then [{Space.merge S}]
				     end
			          end
			babP:     fun {$ P O MRD ?KP}
				     case {BestBAB P O MRD ?KP}
				     of nil then nil
				     elseof S then [{WrapP S}]
				     end
			          end
			babS:     fun {$ P O MRD ?KP}
				     case {BestBAB P O MRD ?KP}
				     of nil then nil
				     elseof S then [S]
				     end
			          end

			restart:  fun {$ P O MRD ?KP}
				     case {BestRestart P O MRD ?KP}
				     of nil then nil
				     elseof S then [{Space.merge S}]
				     end
				  end
			restartP: fun {$ P O MRD ?KP}
				     case {BestRestart P O MRD ?KP}
				     of nil then nil
				     elseof S then [{WrapP S}]
				     end
				  end
			restartS: fun {$ P O MRD ?KP}
				     case {BestRestart P O MRD ?KP}
				     of nil then nil
				     elseof S then [S]
				     end
				  end)

   end

   local

      local
	 %%S is a list with all possibles branches in the current level
	 proc {Recompute S|Sr C}
	    if {Space.is S} then C={Space.clone S}
	    else {Recompute Sr C} {Space.commitB C (S.1).1}
	    end
	 end

	 class ReClass 
	    attr
	       stack:nil cur rd sol:nil prev:nil
	       isStopped:false backtrack:false
	    feat
	       mrd manager order

	    meth init(P O D)
	       cur       <- {Space.new P}
	       rd        <- D
	       isStopped <- false
	       backtrack <- false
	       self.mrd   = D
	       self.order = O
	    end

	    meth stop
	       isStopped <- true
	    end
	    
	    meth resume
	       isStopped <- false
	    end

	    meth last($)
	       case {self next($)}
	       of stopped then stopped
	       [] nil     then @prev
	       elseof S   then prev<-S ReClass,last($)
	       end
	    end

	    meth next($)
	       if @backtrack then
		  ReClass, backtrack
		  backtrack <- false
	       end
	       {self explore($)}
	    end
	    
	    meth push(M)
	       if self.mrd==@rd then
		  rd    <- 1
		  stack <- M#@sol|{Space.clone @cur}|@stack
	       else
		  rd    <- @rd + 1
		  stack <- M#@sol|@stack
	       end
	    end
	 
	    meth backtrack
	       case @stack of nil then cur <- false
	       [] S1|Sr then
		  case S1
		  of I#Sol then
		     if I.2 == nil then
			stack <- Sr rd <- @rd -1
			ReClass,backtrack
		     else
			NextI = I.2 S2|Srr=Sr in
			%%NextI has one element
			if NextI \= nil andthen NextI.2 == nil andthen {Space.is S2} then
			   {Space.commitB S2 NextI.1}
			   stack <- Srr
			   rd    <- self.mrd
			   cur   <- S2
			   if @sol\=Sol then
			      {Better S2 self.order @sol}
			   end
			elseif @sol==Sol then
			   stack <- NextI#Sol|Sr
			   cur   <- {Recompute @stack}
			else
			   cur   <- {Recompute Sr}
			   {SPCommit2 @cur NextI}
			   {Better @cur self.order @sol}
			   rd    <- self.mrd
			   stack <- Sr
			end
		     end
		  else
		     stack <- Sr ReClass,backtrack
		  end
	       end
	    end
	 end

      in

	 class All from ReClass prop final
	    meth explore(S)	      
	       C = @cur
	    in
	       if @isStopped then S=stopped
	       elseif C==false then S=nil
	       else
		  case {Space.ask C} 
		  of failed then
		     All,backtrack  All,explore(S)
		  [] succeeded then
		     S=C backtrack <- true
		  [] branch(M) then
		     All,push(M) {Space.commitB C M.1} All,explore(S)
		  end
	       end
	    end
	 end
	 
	 class Best from ReClass prop final
	    meth explore(S)
	       C = @cur
	    in
	       if @isStopped then S=stopped
	       elseif C==false then S=nil
	       else
		  case {Space.ask C} 
		  of failed then
		     Best,backtrack Best,explore(S)
		  [] succeeded then
		     S=C sol<-C backtrack<-true
		  [] branch(M) then
		     ReClass,push(M) {Space.commitB C M.1} Best,explore(S)
		  end
	       end
	    end
	 end
      end

      proc {Dummy _}
	 skip
      end
      
   in

      class SearchObject
	 prop
	    locking
	 attr
	    RCD:     1
	    MyAgent: Dummy

	 meth script(P ...) = M
	    lock
	       D = {CondSelect M rcd @RCD}
	    in
	       MyAgent <- if {HasFeature M 2} then {New Best init(P M.2 D)}
			  else {New All init(P false D)}
			  end
	       RCD     <- D
	    end
	 end
      
	 meth Next($)
	    lock A=@MyAgent in {A resume} {A next($)} end
	 end
	 
	 meth next($)
	    S=SearchObject,Next($)
	 in
	    if {Space.is S} then [{Space.merge {Space.clone S}}]
	    else S
	    end
	 end
	 
	 meth nextS($)
	    S=SearchObject,Next($)
	 in
	    if {Space.is S} then [{Space.clone S}]
	    else S
	    end
	 end
	 
	 meth nextP($)
	    S=SearchObject,Next($)
	 in
	    if {Space.is S} then [{WrapP S}]
	    else S
	    end
	 end
	 
	 meth Last($)
	    lock A=@MyAgent in {A resume} {A last($)} end
	 end
	 
	 meth last($)
	    S=SearchObject,Last($)
	 in
	    if {Space.is S} then [{Space.merge {Space.clone S}}]
	    else S
	    end
	 end
	 
	 meth lastS($)
	    S=SearchObject,Last($)
	 in
	    if {Space.is S} then [{Space.clone S}]
	    else S
	    end
	 end
	 
	 meth lastP($)
	    S=SearchObject,Last($)
	 in
	    if {Space.is S} then [{WrapP S}]
	    else S
	    end
	 end
	 
	 meth stop
	    {@MyAgent stop}
	 end
	 
	 meth clear
	    lock
	       {@MyAgent stop}
	       MyAgent <- Dummy
	    end
	 end

      end
   end

   %%
   %% Often used short cuts
   %%
   fun {SearchOne P}
      {OneModule.depth P 1 _}
   end
   
   fun {SearchAll P}
      {All P 1 _}
   end
   
   fun {SearchBest P O}
      {BestModule.bab P O 1 _}
   end

   SearchBase = base(one:  SearchOne
		     all:  SearchAll
		     best: SearchBest)
   

end

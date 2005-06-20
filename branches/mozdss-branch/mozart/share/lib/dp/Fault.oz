%%%
%%% Authors:
%%%   Per Brand (perbrand@sics.se)
%%%   Erik Klintskog (erik@sics.se)
%%%
%%% Copyright:
%%%   Per Brand, 1998
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
   Glue at 'x-oz://boot/Glue'
   DPInit
export
   getEntityCond:     GetEntityCond
   enable:            Enable
   disable:           Disable
   install:           Install
   deInstall:         DeInstall
   installWatcher:    InstallWatcher
   deInstallWatcher:  DeInstallWatcher
   defaultEnable:     DefaultEnable
   defaultDisable:    DefaultDisable   
define
   proc{WrongFormat}
      {Exception.raiseError
       type(dp('incorrect fault format'))}
   end

   proc{NotImplemented}
      {Exception.raiseError
       dp('not implemented')}
   end

   proc{Except Entity Cond Op}
      {Exception.'raise'
       system(dp(entity:Entity conditions:Cond op:Op))}
   end

   fun{DConvertToInj Cond}
      injector(entityType:all 'thread':all 'cond':Cond)
   end

   fun{SConvertToInj Entity Cond}
      injector(entityType:single entity:Entity 'thread':all 'cond':Cond)
   end

   fun{TConvertToInj Entity Cond Thread}
      safeInjector(entityType:single entity:Entity
		   'thread':Thread 'cond':Cond)
   end

   fun{GConvertToInj Entity Cond}
      {NotImplemented}
      false
   end

   fun{I_Impl Level Entity Cond Proc}
      case Level of global then
	 {Glue.distHandlerInstall {GConvertToInj Entity Cond} Proc}
      elseof site then
	 {Glue.distHandlerInstall {SConvertToInj Entity Cond} Proc}
      else
	 {WrongFormat}
	 false
      end
   end

   fun{D_Impl Level Entity Cond Proc}
      case Level of global then
	 {Glue.distHandlerDeInstall {GConvertToInj Entity any} Proc}
      elseof site then
	 {Glue.distHandlerDeInstall {SConvertToInj Entity any} Proc}
      else
	 {WrongFormat}
	 false
      end
   end

   fun{DefaultEnableImpl Cond}
      we_dont_suport_defaultEnabled = Cond
   end

   fun{DefaultDisableImpl Cond}
      we_dont_suport_defaultDisable = Cond
   end

   fun{EnableImpl Entity Level Cond}
      {I_Impl Level Entity Cond Except}
   end

   fun{InstallImpl Entity Level Cond Proc}
      {I_Impl Level Entity Cond Proc}
   end

   fun{DisableImpl Entity Level}
      {D_Impl Level Entity any any}
   end

   fun{DeInstallImpl Entity Level}
      {D_Impl Level Entity any any}
   end

   fun{Cond2Int Cond}
      R = r(permFail:6  tempFail:1) 
   in
      {FoldL Cond fun{$ Ind E} R.E + Ind end 0}
   end

   fun{Int2Cond Int}
      R = r(home_removed:permFail home_prm_unavail:permFail home_tmp_unaval:tempFail)
   in
      {Map
       {FoldL [home_removed#4 home_prm_unavail#2 home_tmp_unaval#1]
	fun{$ Val#Res Txt#Int}
	   if Val div Int == 1 then
	      (Val mod Int)#(Txt|Res)
	   else
	      Val#Res
	   end
	end
	Int#nil}.2
       fun{$ Txt}
	  R.Txt
       end}
   end
   FaultPort 

   {Wait Glue}
   {DPInit.init connection_settings _}

   thread
      {ForAll {NewPort $ FaultPort}
       proc{$ Msg}
	  case Msg of
	     watcher(entity:Entity action:Proc condition:IntC) then
	     thread {Proc Entity {Int2Cond IntC}} end
	  end
       end}
   end
   
   {Glue.installFaultPort FaultPort}
   
   GetEntityCond  = Glue.getEntityCond

   Enable       = fun{$ Entity Level Cond}
		     {NotImplemented}
		     true
		  end
   Disable       = fun{$ Entity Level}
		      {NotImplemented}
		      true
		   end
   Install      = fun{$ Entity Level Cond Proc}
		     true
		  end
   DeInstall    = fun{$ Entity Level}
		     true
		  end
   DefaultEnable = fun{$ Cond}
		      {NotImplemented}
		      true
		   end
   DefaultDisable= fun{$}
		      {NotImplemented}
		      true
		   end
   InstallWatcher= fun{$ Entity Cond Proc}
		      Cint = {Cond2Int Cond}
		   in
		      {Glue.distHandlerInstall Entity Cint Proc}
		   end
   DeInstallWatcher=fun{$ Entity Cond Proc}
		       Cint = {Cond2Int Cond}
		    in
		       {Glue.distHandlerDeInstall Entity Cint Proc}
		    end
end






















%%%
%%% Authors:
%%%   Lars Rasmusson (Lars.Rasmusson@sics.se)
%%%
%%% Copyright:
%%%   Lars Rasmusson, 1998
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
   PsqlLib at './PsqlLib.so{native}'
   Finalize

export
   % these are commands that talk to postgres
   Connectdb
   Setdb
   Exec
   ResultStatus
   CmdStatus
   Ntuples
   Nfields
   Fname
   Fsize
   Getlength
   Getvalue

   % useful oz functions that make use
   % of the Psql commands
   StartSQL
   QueryResults

   % useful
   Backquote
define

   proc {Connectdb Str Conn}
      Conn = {PsqlLib.connectdb Str}
      {Finalize.register Conn PsqlLib.finish}
   end

   proc {Setdb Host Port Options TTY DBName Conn}
      Conn = {PsqlLib.setdb Host Port Options TTY DBName Conn}
      {Finalize.register Conn PsqlLib.finish}
   end

   proc {Exec Conn Str Res}
      {PsqlLib.exec Conn Str Res}
      {Finalize.register Res PsqlLib.clear}
   end

   ResultStatus = PsqlLib.resultStatus
   CmdStatus = PsqlLib.cmdStatus

   Ntuples   = PsqlLib.ntuples
   Nfields   = PsqlLib.nfields
   Fname     = PsqlLib.fname
   Fsize     = PsqlLib.fsize
   Getlength = PsqlLib.getlength
   Getvalue  = PsqlLib.getvalue

/*
      {"setdb"          ,5,1,mm_setdb},
      {"finish"         ,1,0,mm_finish},
      {"clear"          ,1,0,mm_clear},
      {"exec"           ,2,1,mm_exec},
      {"resultStatus"   ,1,1,mm_resultStatus},
      {"cmdtStatus"     ,1,1,mm_cmdtStatus},
      {"ntuples"        ,1,1,mm_ntuples},
      {"nfields"        ,1,1,mm_nfields},
      {"fname"          ,1,1,mm_fname},
      {"fsize"          ,2,1,mm_fsize},
      {"getlength"      ,3,1,mm_getlength},
      {"getvalue"       ,3,1,mm_getvalue},
*/


   fun {Dash A B} A#B end

   fun {GetValues Res Tup I Nf}
      if I==Nf then nil
      else
         {Getvalue Res Tup I}|{GetValues Res Tup I+1 Nf}
      end
   end

   fun {GetNames Res I Nf}
      if I==Nf then nil
      else
         {Fname Res I}|{GetNames Res I+1 Nf}
      end
   end

   fun {QueryResults Res}
      Nt Nf FNames MakeTuples
   in
      Nt = {Ntuples Res}
      Nf = {Nfields Res}
      FNames = {GetNames Res 0 Nf}

      fun {MakeTuples T}
         if T==Nt then nil
         else
            Vals = {GetValues Res T 0 Nf}
            Tup  = {List.toRecord t {List.zip FNames Vals Dash}}
         in
            Tup | {MakeTuples T+1}
         end
      end

      {MakeTuples 0}
   end

   fun {StartSQL ConnectStr}
      Conn = {Connectdb ConnectStr}
      fun {ExecSQL Str}
         {Exec Conn Str}
      end
      fun {Query Str}
         R1 Res in
         try
            {ExecSQL "BEGIN" _}
            {ExecSQL "DECLARE myportal CURSOR FOR "#Str R1}
            if {CmdStatus R1}\="DECLARE CURSOR" then
               raise sql(bad_query Str {CmdStatus R1}) end
            end

            Res = {ExecSQL "FETCH ALL in myportal"}
            if {ResultStatus Res} \= 'PGRES_TUPLES_OK' then
               raise sql(bad_fetch Str {ResultStatus Res}) end
            end

            {ExecSQL "CLOSE myportal" _}
            {ExecSQL "END" _}
         catch sql(...)=Exception then
            {ExecSQL "END" _}
            raise Exception end
         end
         {QueryResults Res}
      end
      fun {Insert Rec}
         {ExecSQL {InsertRecStr Rec}}
      end
   in
      s(exec:ExecSQL query:Query insert:Insert)
   end


   % prepare an SQL INSERT string from a record

   fun {Comb A B}
      A#", "#B
   end

   fun {CommaSeparatedVS H|T}
      {FoldL T Comb H}
   end

   fun {Backquote V}
      case V of nil then nil
      elseof &\\ | V then &\\|&\\|{Backquote V}
      elseof &'  | V then &\\|&'| {Backquote V}
      elseof C | V then C|{Backquote V}
      end
   end

   fun {QuoteValue V}
      if {IsInt V} orelse {IsFloat V} then V
      else
         {IsVirtualString V} = true
         "'"#{Backquote {VirtualString.toString V}}#"'"
      end
   end

   fun {InsertRecStr R}
      Table = {Label R}
      Columns = {CommaSeparatedVS {Arity R}}
      Val1 = {Record.toList R}
      Val2 = {Map Val1 QuoteValue}
      Values = {CommaSeparatedVS Val2}
      Cmd = "INSERT INTO "#Table#
      " ("#Columns#") "#
      " VALUES ("#Values#") ;"

   in
      Cmd
   end



end

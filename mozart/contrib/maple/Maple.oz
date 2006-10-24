/*
 *  Authors:
 *    Jürgen Zimmer (jzimmer@ps.uni-sb.de)
 * 
 *  Contributors:
 *    Tobias Müller (tmueller@ps.uni-sb.de)
 * 
 *  Copyright:
 *    Organization or Person (Year(s))
 * 
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 * 
 *  This file is part of Mozart, an implementation 
 *  of Oz 3:
 *     $MOZARTURL$
 * 
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

			      functor

import

   OS(getEnv system)
   Compiler(virtualStringToValue)
   ExportMaple(call) at 'maple.so{native}'
   System(show: Show)

export

   Call
   Simplify
   Solve

prepare

   MapleEnvVar = 'MAPLE'
   MapleDefaultPath = "/share/global/linux/maple/bin/maple"

   MapleInfix =  ['+' '-' '*' '/']
   MaplePrefix = ['min' 'abs']

define

   fun{Oz2Maple Obj}
      {VirtualString.toString {OzToMaple Obj}}
   end

   fun{OzToMaple Term}
      case Term
      of nil then nil
      [] _|_ then
	 "{"#{List2Maple Term}#"}"
      else
	 if {IsRecord Term} then
	    Func = {Label Term}
	 in
	    if {Member Func MapleInfix} then
	       "("#{OzToMaple Term.1}#" "#Func#" "#{OzToMaple Term.2}#")"
	    elseif {Member Func MaplePrefix} then
	       Func#"("#{OzToMaple Term.1}#")"
	    else
	       case Term
	       of Name#_ then
		  Name
	       else Term end
	    end
	 else Term end
      end
   end

   fun{List2Maple Ts}
      case Ts
      of nil then nil
      [] T|Tr then
	 if Tr == nil then
	    {OzToMaple T}
	 else
	    {OzToMaple T}#", "#{List2Maple Tr}
	 end
      else
	 Ts
      end
   end

   fun{Simplify Term}
      MapleTerm = {OzToMaple Term}
      Result = {Call simplify MapleTerm}
   in
      Result %NewTerm
   end

   fun{Solve Term}
      MapleTerm = {OzToMaple Term}
      Result = {Call solve MapleTerm}
   in
      Result %NewTerm
   end


   fun {Call Command Args}

      if {IsAtom Command} then
	 CmdString = 'convert('#Command
	 #'('
	 #{Oz2Maple Args}
	 #')'
	 #', string);'

	 Result = {ExportMaple.call CmdString {MapleProgram}}
	 Term = {Compiler.virtualStringToValue Result}
      in
	 Term
      else
	 {Show 'Maple command must be an atom.'}
	 unit
      end
   end

   fun {MapleProgram}
      EnvMaple = {OS.getEnv MapleEnvVar}
   in
      if EnvMaple == false then
	 ExistsMaple = {OS.system 'which maple > /dev/null'}
      in
	 %{Show ExistsMaple}
	 if ExistsMaple == 0 then
	    "maple"
	 else
	    MapleDefaultPath
	 end
      else
	 EnvMaple
      end
   end

end









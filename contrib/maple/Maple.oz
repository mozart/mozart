%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Maple.oz ; This file is part of the OMEGA system
%
%   major updates: 27.7.1999,
%
%   Authors: Juergen Zimmer
%
%   email: jzimmer@ags.uni-sb.de
%
% For information about this program, write to:
%   OMEGA Project
%   AG Siekmann/FB Informatik
%   Universitaet des Saarlandes
%   Bau 36, 4. Stock
%   D-66041 Saarbruecken
%   Germany
%
% For information about the newest version of Omega, see
%   http://www.ags.uni-sb.de/~omega/
%
% This program is free software; it can be used under the terms of the GNU General
% Public License as published by the Free Software Foundation; either version 2 of
% the License, or any later version.
%
% This program is distributed in the hope that it will be useful, but
% WITHOUT ANY WARRANTY; without even the implied warranty of
% merchantibility or fitness for a particular purpose.
% See the GNU General Public License
% (http://www.fsf.org/copyleft/gpl.html)
% for more details.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

functor
   %% The Maple call and simplification functions
   %%      jzimmer 15.7.99

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
      %%Edited  = "27-Jun-1999 16:22"
      %%Authors = [jzimmer]
      %%Value   = "The Object in Maple syntax."

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
      %%Edited  = "27-Jun-1999 16:22"
      %%Authors = [jzimmer]
      %%Value   = "The Result of the execution of the Maple(tm) simplification on the given Term."
      MapleTerm = {OzToMaple Term}
      Result = {Call simplify MapleTerm}
   in
      Result %NewTerm
   end

   fun{Solve Term}
      %%Edited  = "27-Jun-1999 16:22"
      %%Authors = [jzimmer]
      %%Value   = "The Result of the execution of the Maple(tm) simplification on the given Term."
      MapleTerm = {OzToMaple Term}
      Result = {Call solve MapleTerm}
   in
      Result %NewTerm
   end


   fun {Call Command Args}
      %%Edited  = "27-Jun-1999 16:22"
      %%Authors = [jzimmer]
      %%Value   = "The Result of the execution of the Maple(tm) Command with the "
      %%          "given Arguments."

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
      %%Edited  = "20-APR-1999 16:22"
      %%Authors = [jzimmer]
      %%Value   = "The location of the maple executable in the file system."

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

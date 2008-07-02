functor
export
   ExecuteProgram
   ExecuteCommand
   ToUserVS
import
   Property(get)
   OS(getEnv system)
prepare
   VS2S = VirtualString.toString
   ToLower = Char.toLower

   fun {QuoteUsing CMD Quote}
      %% CMD is a list: we are going to quote each element of
      %% this list using the Quote string specified and then
      %% we are going to concatenate all of them separated by
      %% single spaces
      {FoldL CMD
       %% in principle, we should be careful about embedded
       %% occurrences of characters used for quoting.  We will
       %% ignore this issue for the time being.
       fun {$ VS I} VS#' '#Quote#I#Quote end nil}
   end

   %% and advantage of using an arbitrary VS as a Quote is that
   %% we can also use an empty VS, which is useful when we want
   %% to display commands in a readable way to the user.
   
   fun {ToUserVS CMD} {QuoteUsing CMD ''} end
define
   ToProgramVS
   ToCommandVS

   %% arguments on the command given to the shell for execution
   %% need to be quoted to avoid problems with embedded spaces
   %% in filenames and special shell characters
   
   if {Property.get 'platform.os'}=='win32' then
      SHELL
   in
      case {Reverse {Map {VS2S {OS.getEnv 'COMSPEC'}} ToLower}}
      of &e|&x|&e|&.|&d|&m|&c|_ then
	 SHELL = nil
      else
	 SHELL = 'COMMAND.COM /C '
      end
      fun {!ToProgramVS CMD}
	 SHELL#{QuoteUsing CMD '"'}
      end
      fun {!ToCommandVS CMD}
	 case CMD of H|T then
	    SHELL#H#{QuoteUsing T '"'}
	 end
      end
   else
      fun {!ToProgramVS CMD}
	 {QuoteUsing CMD '\''}
      end
      !ToCommandVS = ToProgramVS
   end

   proc {Execute VS}
      if {OS.system VS}\=0 then raise shell(VS) end end
   end

   proc {ExecuteProgram CMD} {Execute {ToProgramVS CMD}} end
   proc {ExecuteCommand CMD} {Execute {ToProgramVS CMD}} end
end

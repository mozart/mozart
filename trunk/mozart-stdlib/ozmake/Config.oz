functor
export
   'class' : Config
import
   Path at 'Path.ozf'
prepare
   IsPrefix = List.isPrefix
   fun {Return X} X end
   fun {PadKey X}
      N = {VirtualString.length X}
   in
      if N<20 then
	 for I in 1..(20-N) collect:C do {C & } end#X
      else X end
   end
   fun {ToVS X}
      if X==nil then 'nil'
      elseif {IsVirtualString X} then '"'#X#'"'
      elseif {IsList X} then
	 '['#{FoldL X fun {$ Accu X}
			 Accu#if Accu==nil then nil else ' ' end#{ToVS X}
		      end nil}#']'
      else {Value.toVirtualString X 1000000 1000000}
      end
   end
define
   class Config
      attr
	 DB : unit

      meth config_read()
	 %%
	 %% old-style database:
	 %%     ~/.oz/apps/ozmake/DATABASE.ozf
	 %%     ~/.oz/apps/ozmake/DATABASE.txt
	 %% new-style database:
	 %%     ~/oz/apps/ozmake/config.db
	 %%
	 if @DB==unit then
	    F = {self get_configfile($)}
	 in
	    if {Path.exists F} then
	       DB <- {self databaselib_read(F $)}
	    else
	       F = {self get_configfile_oldstyle($)}
	    in
	       if {Path.exists F#'.ozf'} orelse {Path.exists F#'.txt'} then
		  {self trace('detected old-style config database')}
		  {self incr}
		  case {self databaselib_read_oldstyle(F Return $)}
		  of E|_ then
		     DB<-{Record.toDictionary E}
		     {self trace('replacing by new-style config database')}
		     {self config_save}
		     if {Path.exists F#'.ozf'} then
			{self exec_rm(F#'.ozf')}
		     end
		     if {Path.exists F#'.txt'} then
			{self exec_rm(F#'.txt')}
		     end
		  end
		  {self decr}
	       else
		  DB <- {NewDictionary}
	       end
	    end
	 end
      end

      meth config_save()
	 if @DB\=unit then
	    {self databaselib_save({self get_configfile($)} @DB)}
	 end
      end

      meth config_install(Args OPTLIST)
	 {self config_read}
	 for Key#Set#Flag#_ in OPTLIST do
	    if Flag
	       andthen {HasFeature @DB Key}
	       andthen {Not {HasFeature Args Key}}
	    then
	       {self Set(@DB.Key)}
	    end
	 end
      end

      meth config(Args OPTLIST)
	 {self config_read}
	 if @DB==unit then DB<-{NewDictionary} end
	 case {self get_config_action($)}
	 of put then
	    {self incr}
	    try
	       for Key#_#Flag#_ in OPTLIST do
		  if {HasFeature Args Key} then
		     if Flag then
			{self xtrace('setting '#Key#': '#{ToVS Args.Key})}
			@DB.Key := Args.Key
		     else
			{self vtrace('ignoring: '#Key)}
		     end
		  end
	       end
	    finally {self decr} end
	    {self config_save}
	 [] delete then
	    {self incr}
	    try
	       for S in Args.1 do Key={StringToAtom S} in
		  if {HasFeature @DB Key} then
		     {self trace('unsetting : '#Key)}
		     {Dictionary.remove @DB Key}
		  else
		     {self trace('no default: '#Key)}
		  end
	       end
	    finally {self decr} end
	    {self config_save}
	 [] list then
	    for K#V in {Sort {Dictionary.entries @DB}
			fun {$ K1#_ K2#_} K1<K2 end}
	    do
	       {self print({PadKey K}#':  '#{ToVS V})}
	    end
	 end
      end

      meth config_validate_action(S $)
	 case for A in [put delete list] collect:C do
		 if {IsPrefix S {AtomToString A}} then {C A} end
	      end
	 of nil then raise ozmake(config:unknownaction(S)) end
	 [] [A] then A
	 []  L  then raise ozmake(config:ambiguousaction(S L)) end
	 end
      end
      
   end
end

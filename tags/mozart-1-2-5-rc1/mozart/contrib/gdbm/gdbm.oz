%%% -*-oz-*-
%%% Copyright (c) by Denys Duchier, Jun 1997, Universität des Saarlandes
%%% ------------------------------------------------------------------
%%%
%%%

local
   FlagTable = o(read	 :{BitString.make 5 [0]}
		 write	 :{BitString.make 5 [1]}
		 create	 :{BitString.make 5 [2]}
		 new	 :{BitString.make 5 [3]}
		 truncate:FlagTable.new
		 fast	 :{BitString.make 5 [4]}
		 r	 :FlagTable.read
		 w	 :FlagTable.write
		 c	 :FlagTable.create
		 n	 :FlagTable.new
		 t	 :FlagTable.new
		 f	 :FlagTable.fast
		)
   None = {BitString.make 5 nil}
   fun {FlagsEncode Spec}
      if {IsAtom Spec} then FlagTable.Spec
      else {FoldL Spec
	    fun {$ B X} {BitString.disj B FlagTable.X} end None} end
   end
   DefaultMode = 0644
in

   functor
   import
      NativeGDBM(
	 is		: IS
	 open		: OPEN
	 get		: GET
	 condGet	: CONDGET
	 put		: PUT
	 firstkey	: FIRSTKEY
	 nextkey	: NEXTKEY
	 close		: CLOSE
	 free		: FREE
	 remove		: REMOVE
	 reorganize	: REORGANIZE
	 member		: MEMBER
	 ) at 'gdbm.so{native}'
      Finalize(guardian	:Guardian)
      Resolve( expand	:Expand) URL
      Error(registerFormatter)
      MODE at 'x-oz://contrib/os/mode'
   export
      is	: IS
      open	: Open
      get	: GET
      condGet	: CONDGET
      put	: PUT
      firstkey	: FIRSTKEY
      nextkey	: NEXTKEY
      close	: CLOSE
      remove	: REMOVE
      reorganize: REORGANIZE
      member	: MEMBER
      keys	: Keys
      entries	: Entries
      items	: Items
      forAll	: ForAll
      forAllInd	: ForAllInd
      new	: New
   define

      Register = {Guardian FREE}

      fun {Open Name Flags Mode Block}
	 ZFlags = {FlagsEncode Flags}
	 ZMode  = {MODE.make Mode}
	 ZFile  = {URL.toAtom {Expand Name}}
	 DB     = {OPEN ZFile ZFlags ZMode Block}
      in
	 {Register DB}
	 DB
      end

      %% more convenient way of opening a DB
      %% the label indicates how to create it (flags)
      %% feature 1 is the file name
      %% optional feature mode is the permission mode for creation

      fun {New Spec}
	 Flag = {Label Spec}
	 File = Spec.1
	 Mode = {CondSelect Spec mode DefaultMode}
	 Fast = {CondSelect Spec fast false}
	 Flags = if Fast then [Flag fast] else Flag end
      in
	 {Open File Flags Mode 0}
      end

      %% lazy list of keys

      fun {Keys DB}
	 fun lazy {Loop K}
	    if K==unit then nil else
	       K|{Loop {NEXTKEY DB K}}
	    end
	 end
      in {Loop {FIRSTKEY DB}} end

      %% lazy list of entries

      fun {Entries DB}
	 fun lazy {Loop K}
	    if K==unit then nil else
	       (K#{GET DB K})|{Loop {NEXTKEY DB K}}
	    end
	 end
      in {Loop {FIRSTKEY DB}} end

      %% lazy list of items

      fun {Items DB}
	 fun lazy {Loop K}
	    if K==unit then nil else
	       {GET DB K}|{Loop {NEXTKEY DB K}}
	    end
	 end
      in {Loop {FIRSTKEY DB}} end

      %% iterating through all items

      proc {ForAll DB PROC}
	 proc {Loop K}
	    if K==unit then skip else
	       {PROC {GET DB K}}
	       {Loop {NEXTKEY DB K}}
	    end
	 end
      in {Loop {FIRSTKEY DB}} end

      %% iterating though all entries

      proc {ForAllInd DB PROC}
	 proc {Loop K}
	    if K==unit then skip else
	       {PROC K {GET DB K}}
	       {Loop {NEXTKEY DB K}}
	    end
	 end
      in {Loop {FIRSTKEY DB}} end

      %% Error Formatting

      fun {GdbmFormatter E}
	 T = 'GDBM Error'
      in
	 case E
	 of gdbm(Proc Args Msg) then
	    error(kind: T
		  msg: case Msg
		       of 'alreadyClosed' then 'database already closed'
		       [] 'keyNotFound'   then 'key not found'
		       else Msg end
		  items: [hint(l:'Operation' m:Proc)
			  hint(l:'Input Args' m:oz(Args))])
	 else
	    error(kind: T
		  items: [line(oz(E))])
	 end
      end

      {Error.registerFormatter gdbm GdbmFormatter}
   end

end

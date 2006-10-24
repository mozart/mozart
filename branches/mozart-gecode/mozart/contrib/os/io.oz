local
   %% read write append create truncate
   FlagTable = table(read	:{BitString.make 5 [0]}
		     write	:{BitString.make 5 [1]}
		     append	:{BitString.make 5 [2]}
		     create	:{BitString.make 5 [3]}
		     truncate	:{BitString.make 5 [4]}
		     r		:FlagTable.read
		     w		:FlagTable.write
		     a		:FlagTable.append
		     c		:FlagTable.create
		     t		:FlagTable.truncate
		    )
   UserFlags = table(read	:{BitString.make 5 [0]}
		     write	:{BitString.make 5 [1 3 4]}
		     append	:{BitString.make 5 [1 2 3]})
   None = {BitString.make 5 nil}
   fun {Encode Spec}
      if {IsList Spec} then
	 {FoldL Spec
	  fun {$ B X} {BitString.disj B {Encode X}} end None}
      else FlagTable.Spec end
   end
   FLAG = flag(read	:fun {$ B} {BitString.get B 0} end
	       write	:fun {$ B} {BitString.get B 1} end
	       append	:fun {$ B} {BitString.get B 2} end
	       create	:fun {$ B} {BitString.get B 3} end
	       truncate	:fun {$ B} {BitString.get B 4} end)
in
   functor
   import
      IO(
	 is		: IS
	 write		: WRITE
	 read		: READ
	 make		: MAKE
	 close		: CLOSE
	 free		: FREE
	 open		: OPEN
	 socketpair	: SOCKETPAIR
	 dup		: DUP
	 pipe		: PIPE
	 getfd		: GETFD
	 ) at 'io.so{native}'
      PROC at 'process'
      MODE at 'mode'
      Finalize URL Resolve OS
   export
      is	: IS
      make	: MAKE
      getfd	: GETFD
      close	: CLOSE
      read	: READ
      socketpair: SOCKETPAIR
      dup	: DUP
      Write ReadS ReadSLazy Open Run Pipe DevNull
      Stdin Stdout Stderr
   define

      proc {Write FD X}
	 B = if {ByteString.is X} then X else {ByteString.make X} end
      in
	 {DoWrite FD B 0}
      end

      proc {DoWrite FD B I}
	 J = {WRITE FD B I}
      in
	 if J==unit then skip else {DoWrite FD B J} end
      end

      proc {ReadS FD STRING}
	 B = {READ FD}
      in
	 if B==unit then STRING=nil else MORE in
	    {ByteString.toStringWithTail B MORE STRING}
	    {ReadS FD MORE}
	 end
      end

      fun lazy {ReadSLazy FD}
	 B = {READ FD}
      in
	 if B==unit then nil else STRING MORE in
	    {ByteString.toStringWithTail B MORE STRING}
	    {ReadSLazy FD MORE}
	    STRING
	 end
      end

      proc {Open Spec ?FD}
	 File  = Spec.1
	 Url   = {Resolve.expand {URL.make File}}
	 Mode  = {MODE.make {CondSelect Spec mode 0644}}
	 Flags = {BitString.disj
		  {Encode {CondSelect Spec flags nil}}
		  case {Label Spec}
		  of read   then UserFlags.read
		  [] write  then UserFlags.write
		  [] append then UserFlags.append
		  [] file   then None
		  end}
      in
	 if {CondSelect Url scheme unit}\=unit andthen
	    ({FLAG.write    Flags} orelse
	     {FLAG.append   Flags} orelse
	     {FLAG.truncate Flags})
	 then
	    {Raise {Exception.system
		    open(urlIsReadOnly Spec)}}
	 else
	    R  = {Resolve.localize Url}
	 in
	    FD = try {OPEN R.1 Flags Mode}
		 finally
		    if new=={Label R} then
		       try {OS.unlink R.1} catch _ then skip end
		    end
		 end
	    {Finalize.register FD FREE}
	 end
      end

      fun {Pipe} RD WR in
	 {PIPE RD WR}
	 pipe(read:RD write:WR)
      end

      fun {DevNull}
	 {Open file('/dev/null' flags:[read write])}
      end

      proc {Run Spec FD}
	 PID = {CondSelect Spec pid _}
      in
	 case {Label Spec}
	 of read      then
	    pipe(read:RD write:WR) = {Pipe}
	 in
	    RD=FD {RunProcess {Adjoin Spec process(stdout:WR)} PID}
	    {CLOSE WR}
	 [] write     then
	    pipe(read:RD write:WR) = {Pipe}
	 in
	    WR=FD {RunProcess {Adjoin Spec process(stdin:RD)} PID}
	    {CLOSE RD}
	 [] readWrite then FD1 FD2 in
	    {SOCKETPAIR FD1 FD2}
	    FD=FD1 {RunProcess {Adjoin Spec process(stdin:FD2 stdout:FD2)} PID}
	 [] process   then
	    FD=PID {RunProcess Spec PID}
	 end
      end

      fun {RunProcess Spec}
	 CMD  = Spec.1
	 IN   = {CondSelect Spec stdin  unit}
	 OUT  = {CondSelect Spec stdout unit}
	 ERR  = {CondSelect Spec stderr unit}
	 ARGS = {CondSelect Spec 2      nil}
	 L0=nil L1 L2 L3
	 if IN ==unit then L1=L0 else L1=({GETFD IN }#0)|L0 end
	 if OUT==unit then L2=L1 else L2=({GETFD OUT}#1)|L1 end
	 if ERR==unit then L3=L2 else L3=({GETFD ERR}#2)|L2 end
	 IOMAP= L3
      in {PROC.make CMD ARGS IOMAP} end

      Stdin  = {MAKE 0}
      Stdout = {MAKE 1}
      Stderr = {MAKE 2}

   end
end

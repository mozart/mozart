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
in
   functor
   import
      IO(
	 is		: IS
	 write		: WRITE
	 read		: READ
	 make		: MAKE
	 close		: CLOSE
	 release	: RELEASE
	 readLock	: READLOCK
	 writeLock	: WRITELOCK
	 open		: OPEN
	 socketpair	: SOCKETPAIR
	 dup		: DUP
	 fork		: FORK
	 execvp		: EXECVP
	 pipe		: PIPE
	 ) @ 'io.so{native}'
      MODE @ 'io/mode'
      Finalize
   export
      Make Write Read ReadAsString Open Close SocketPair Dup
      Fork Run
   define

      fun {Make I}
	 {MAKE I {NewLock} {NewLock}}
      end

      proc {Close FD}
	 lock {WRITELOCK FD} then
	    lock {READLOCK FD} then {CLOSE FD} end
	 end
      end

      proc {Write FD X}
	 B = if {ByteString.is X} then X else {ByteString.make X} end
      in
	 lock {WRITELOCK FD} then {DoWrite FD B 0} end
      end

      proc {DoWrite FD B I}
	 J = {WRITE FD B I}
      in
	 if J==unit then skip else {DoWrite FD B J} end
      end

      fun {Read FD}
	 lock {READLOCK FD} then {READ FD} end
      end

      fun {ReadAsString FD}
	 UNLOCK
	 fun lazy {Loop}
	    B = {READ FD}
	 in
	    case B==unit then UNLOCK=unit nil else Front Back in
	       {ByteString.toStringWithTail B Back Front}
	       {Loop Back}
	       Front
	    end
	 end
      in
	 %% acquire and hold the read lock until all input has eventually
	 %% be (lazily) read.
	 thread lock {READLOCK FD} then {Wait UNLOCK} end end
	 {Loop}
      end

      proc {Open Spec ?FD}
	 File  = Spec.1
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
	 FD = {Make {OPEN File Flags Mode}}
	 {Finalize.register FD RELEASE}
      end

      proc {SocketPair FD1 FD2}
	 I1 I2
      in
	 {SOCKETPAIR I1 I2}
	 {Make I1 FD1}
	 {Make I2 FD2}
      end

      fun {Dup FD}
	 {Make {DUP FD}}
      end

      fun {Fork} {FORK} end

      fun {Pipe} I1 I2 in
	 {PIPE I1 I2}
	 pipe(read:{Make I1} write:{Make I2})
      end

      fun {DevNull}
	 {Open file('/dev/null' flags:[read write])}
      end

      proc {Run Spec FD}
	 case {Label Spec}
	 of read      then
	    pipe(read:RD write:WR) = {Pipe}
	 in
	    RD=FD {RunProcess {Adjoin Spec process(stdout:WR)}}
	 [] write     then
	    pipe(read:RD write:WR) = {Pipe}
	 in
	    WR=FD {RunProcess {Adjoin Spec process(stdin:RD)}}
	 [] readWrite then FD1 FD2 in
	    {SocketPair FD1 FD2}
	    FD=FD1 {RunProcess {Adjoin Spec process(stdin:FD2 stdout:FD2)}}
	 [] process   then
	    FD=unit {RunProcess Spec}
	 end
      end

      proc {RunProcess Spec}
	 CMD  = Spec.1
	 ARGS = {CondSelect Spec 2      nil}
	 IN   = {CondSelect Spec stdin  unit}
	 OUT  = {CondSelect Spec stdout unit}
	 ERR  = {CondSelect Spec stderr unit}
	 PID  = {Fork}
      in
	 if PID==0 then
	    %% child
	    IN2  = if IN \=unit then {Dup IN } else unit end
	    OUT2 = if OUT\=unit then {Dup OUT} else unit end
	    ERR2 = if ERR\=unit then {Dup ERR} else unit end
	 in
	    %% close std{in,out,err}
	    if IN  \=unit then {CLOSE IN } end
	    if OUT \=unit then {CLOSE OUT} end
	    if ERR \=unit then {CLOSE ERR} end
	    %% reopen them
	    if IN2 \=unit then {DUP IN2  _} {CLOSE IN2 } end
	    if OUT2\=unit then {DUP OUT2 _} {CLOSE OUT2} end
	    if ERR2\=unit then {DUP ERR2 _} {CLOSE ERR2} end
	    {EXECVP CMD ARGS}
	 else
	    %% parent
	    skip
	 end
      end

   end
end

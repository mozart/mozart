%% A replacement for Open, based on contrib/os/io

local
   NOARG      = {NewName}
   READSIZE   = 1024

   ReadFD     = {NewName}
   WriteFD    = {NewName}
   Buffer     = {NewName}
   ReadLock   = {NewName}
   WriteLock  = {NewName}
   FillBuffer = {NewName}
   EmptyBufer = {ByteString.make nil}
   Offset     = {NewName}
   FillLine   = {NewName}
   InitBase   = {NewName}
   Init       = {NewName}

   WHENCE     = whence('SEEK_SET'	:0  set :0
		       'SEEK_CUR'	:1  cur :1 current:1
		       'SEEK_END'	:2 'end':2)

   proc {OpenExc What Self Msg}
      {Raise {Exception.system open(What Self Msg)}}
   end

   proc {GetLines IN READLOCK} UNLOCK in
      lock READLOCK then
	 %% this will block until the current lock is released
	 thread lock READLOCK then {Wait UNLOCK} end end
	 ...
      end
   end
      

   fun lazy {Lines IN}
      try {IN gets($)}|{Lines IN}
      catch system(open(alreadyClosed _ _) debug:_) then nil end
   end

in
   functor
   import
      Exception
      IO   @ 'io'
      FLAGS @
   export

   define

      class IOBase
	 prop native locking
	 attr
	    !ReadFD	: unit
	    !ReadLock
	    !WriteFD	: unit
	    !WriteLock
	    !Buffer	: unit
	    !Offset	: 0
	 meth !InitBase
	    ReadLock  <- {NewLock}
	    WriteLock <- {NewLock}
	    Buffer    <- EmptyBufer
	 end
	 %%
	 %% ensures that there are at least N chars in the buffer, or
	 %% that ReadFD is at EOF. Assumes that self.ReadLock has been
	 %% acquired.
	 %%
	 meth !FillBuffer(N)
	    if @Buffer==unit then skip else
	       Length = {ByteString.length @Buffer}
	       Avail  = Length - @Offset
	    in
	       if Avail==0 then Buffer <- unit end
	       if Avail<N then
		  if @ReadFD==unit then
		     %% closed
		     skip
		  else
		     More = {IO.read @ReadFD}
		  in
		     if More==unit then
			ReadFD <- unit
		     elseif Avail==0 then
			Buffer <- More
			IOBase,FillBuffer(N)
		     else
			if @Offset\=0 then
			   Buffer <- {ByteString.append
				      {ByteString.slice @Offset Length}
				      More}
			   Offset <- 0
			else
			   Buffer <- {ByteString.append @Buffer More}
			end
			IOBase,FillBuffer(N)
		     end
		  end
	       end
	    end
	 end
	 %%
	 %% get 1 character
	 %%
	 meth getc(C)
	    lock @ReadLock then
	       IOBase,FillBuffer(1)
	       if @Buffer==unit then
		  {OpenExc alreadyClosed self getc(C)}
	       else
		  C = {ByteString.get @Buffer @Offset}
		  Offset <- @Offset + 1
	       end
	    end
	 end
	 %%
	 %% get n bytes
	 %%
	 meth getn(N Bytes)
	    lock @ReadLock then
	       IOBase,FillBuffer(N)
	       if @Buffer==unit then
		  {OpenExc alreadyClosed self getn(N Bytes)}
	       else
		  Length = {ByteString.length @Buffer}
		  Avail  = Length - @Offset
	       in
		  if Avail<N then
		     if @Offset==0 then
			Bytes=@Buffer
		     else
			Bytes={ByteString.slice @Buffer @Offset Length}
		     end
		     Buffer<-unit %% now fully closed
		     Offset<-0    %% for consistency (but unnecessary)
		  else
		     if @Offset==0 andthen Length==N
			Bytes=@Buffer Buffer <- EmptyBuffer
		     else TO = @Offset+N in
			Bytes={ByteString.slice @Buffer @Offset TO}
			if TO==Length then
			   Buffer=EmptyBuffer
			   Offset <- 0
			else
			   Offset <- TO
			end
		     end
		  end
	       end
	    end
	 end
	 %%
	 %% ensures that the buffer contains at least 1 newline, or
	 %% that ReadFD is at EOF.  assumes that ReadLock has been
	 %% acquired.
	 %%
	 meth !FillLine(ATLEAST)
	    IOBase,FillBuffer(ATLEAST)
	    if @Buffer==unit orelse
	       {ByteString.length @Buffer}<ATLEAST orelse
	       {ByteString.strchr @Buffer @Offset &\n}\=false
	    then skip else
	       IOBase,FillLine(ATLEAST+READSIZE)
	    end
	 end
	 %%
	 %% get 1 line
	 %%
	 meth gets(Line)
	    lock @ReadLock then
	       IOBase,FillLine(1)
	       if @Buffer==unit then
		  {OpenExc alreadyClosed self gets(Line)}
	       else
		  I = {ByteString.strchr @Buffer @Offset &\n}
		  Length = {ByteString.length @Buffer}
	       in
		  if I==false then
		     if @Offset==0 then Line=@Buffer
		     else Line={ByteString.slice @Buffer @Offset Length} end
		     Buffer <- unit
		     Offset <- 0
		  else
		     Line={ByteString.slice @Buffer @Offset @I}
		     if I+1==Length then
			Buffer <- EmptyBuffer
			Offset <- 0
		     else
			Offset <- I+1
		     end
		  end
	       end
	    end
	 end
	 %%
	 %% get 1 line as a string
	 %%
	 meth getS(Line)
	    try B={self gets($)} in
	       {ByteString.toString B Line}
	    catch system(open(alreadyClosed _ _) debug:_) then
	       false
	    end
	 end
	 %%
	 %% get 1 character (old name)
	 %%
	 meth getC(C) {self getc(C)} end
	 %%
	 %% lseek
	 %%
	 meth LSEEK(FD Offset Whence Point) = M
	    if FD==unit then
	       {OpenExc lseek self M}
	    else
	       {IO.lseek FD Offset Whence Point
	    end
	 end
	 meth seek(1:X<=read offset:O<=0 whence:W<=cur)
	    L FD
	 in
	    case X
	    of read  then L=@ReadLock  FD=@ReadFD
	    [] write then L=@WriteLock FD=@WriteFD
	    end
	    lock L then
	       {self LSEEK(FD O W _)}
	    end
	 end
	 %%
	 %% tell
	 %%
	 meth tell(1:X<=read offset:O) L FD in
	    case X
	    of read  then L=@ReadLock  FD=@ReadFD
	    [] write then L=@WriteLock FD=@WriteFD
	    end
	    lock L then
	       {IO.lseek FD 0 WHENCE.cur O}
	    end
	 end
      end

      class File from IOBase
	 meth init(name	: Name
		   flags: Flags	<= [read]
		   mode	: Mode	<= mode(owner:[write] all:[read]))
	    IOBase,InitBase
	    FD = case Name
		 of stdin  then IO.stdin
		 [] stdout then IO.stdout
		 [] stderr then IO.stderr
		 else {IO.open file(Url flags:Flags mode:Mode)} end
	 in
	    ReadFD  <- FD
	    WriteFD <- FD
	 end
	 meth !Init(read:RD<=unit write:WR<=unit)
	    IOBase,InitBase
	    ReadFD  <- RD
	    WriteFD <- WR
	 end
      end

      %%
      %% stdout can be specified in several ways
      %%
      %% (1) by default it is `stdout' which means it remains connected
      %%     to the usual standard output
      %% (2) `unit' indicates that the output should be discarded.
      %%     normally this is implemented by opening /dev/null
      %% (3) `io(FD)' indicates that a new IO stream should be created
      %%     and that FD should be bound to it
      %% (4) `open(OBJ)' indicates that a new File object should be
      %%     created and that OBJ should be bound to it.
      %%
      %% what about stdin?
      %%
      %% (1) by default stdin is `unit' which means it should be connected
      %%     to /dev/null
      %% (2) it may be `stdin', in which case it remains connected to the
      %%     current standard input.
      %% (3) `io(FD)' indicates that a new IO steam should be created
      %%     and that FD should be bound to it
      %% (4) `open(OBJ)' indicates that a new File object should be
      %%     created and that OBJ should be bound to it.

      fun {ProcessFD DIR X}
	 case X
	 of unit then {IO.devNull}
	 [] stdin then IO.stdin
	 [] stdout then IO.stdout
	 [] stderr then IO.stderr
	 [] io(FD) then L R in
	    {IO.socketPair L R} FD=L R
	 [] open(OBJ) then L R in
	    {IO.socketPair L R}
	    {New File Init(DIR:L) OBJ} R
	 end
      end

      class Process from IOBase
	 meth init(cmd   : Cmd
		   args  : Args <= nil
		   stdin : IN   <= unit
		   stdout: OUT  <= stdout
		   stderr: ERR  <= stderr)
	    IOBase,InitBase
	    %%
	    FD_IN  = {ProcessFD IN }
	    FD_OUT = {ProcessFD OUT}
	    FD_OUT = {ProcessFD ERR}
	    %% compute IO map
	    L0 = nil
	    L1 = if 
	 end
      end

end

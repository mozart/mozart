%% A replacement for Open, based on contrib/os/io
%\define WEIRD

local
   READSIZE    = 1024

   ReadFD      = {NewName}
   WriteFD     = {NewName}
   Buffer      = {NewName}
   ReadLock    = {NewName}
   WriteLock   = {NewName}
   FillBuffer  = {NewName}
   EmptyBuffer = {ByteString.make nil}
   Offset      = {NewName}
   FillLine    = {NewName}
   InitBase    = {NewName}
   Init        = {NewName}

   WHENCE      = whence('SEEK_SET'      :0  set :0
                        'SEEK_CUR'      :1  cur :1 current:1
                        'SEEK_END'      :2 'end':2)

   proc {OpenExc What Self Msg}
      {Raise {Exception.system open(What Self Msg)}}
   end

in
   functor
   import
      IO   at 'x-oz://contrib/os/io'
      PROC at 'x-oz://contrib/os/process'
   export
      File Process
   define

\ifdef WEIRD
      proc {NOOP _} skip end
\endif

      class IOBase
         prop locking
         attr
            !ReadFD     : unit
            !ReadLock
            !WriteFD    : unit
            !WriteLock
            !Buffer     : unit
            !Offset     : 0
         meth !InitBase
            ReadLock  <- {NewLock}
            WriteLock <- {NewLock}
            Buffer    <- EmptyBuffer
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
\ifdef WEIRD
                     {NOOP see(foo)}
\endif
                     More = {IO.read @ReadFD}
                  in
\ifdef WEIRD
                     {NOOP see(foo)}
\endif
                     if More==unit then
                        ReadFD <- unit
                     elseif Avail==0 then
                        Buffer <- More
                        Offset <- 0
                        IOBase,FillBuffer(N)
                     else
                        %% the forced record construction is here
                        %% necessary
\ifdef WEIRD
                        {NOOP see(foo)} %% <-- WEIRD BUG FIX
\endif
                        if @Offset\=0 then
                           Buffer <- {ByteString.append
                                      {ByteString.slice
                                       @Buffer @Offset Length}
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
               if @Buffer==unit then C=false
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
               if @Buffer==unit then Bytes=false
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
                     if @Offset==0 andthen Length==N then
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
\ifdef WEIRD
            {NOOP see(foo)}
\endif
            IOBase,FillBuffer(ATLEAST)
            BUFFER = @Buffer
         in
\ifdef WEIRD
            {NOOP see(foo)}
\endif
            if BUFFER==unit orelse
               {ByteString.length BUFFER}<ATLEAST orelse
               {ByteString.strchr BUFFER @Offset &\n}\=false
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
               BUFFER = @Buffer
            in
               if BUFFER==unit then Line=false
               else
                  OFFSET = @Offset
                  I = {ByteString.strchr BUFFER OFFSET &\n}
                  Length = {ByteString.length BUFFER}
               in
                  if I==false then
                     if OFFSET==0 then Line=BUFFER
                     else Line={ByteString.slice BUFFER OFFSET Length} end
                     Buffer <- unit
                     Offset <- 0
                  else
                     Line={ByteString.slice BUFFER OFFSET I}
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
         meth getS(Line) B={self gets($)} in
            if B==false then Line=false else
               {ByteString.toString B Line}
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
               {IO.lseek FD Offset Whence Point}
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
         %%
         meth close
            lock @ReadLock then
               lock @WriteLock then
                  if @ReadFD\=unit then
                     try {IO.close @ReadFD} catch _ then skip end
                     ReadFD <- unit
                  end
                  if @WriteFD\=unit then
                     try {IO.close @WriteFD} catch _ then skip end
                     WriteFD <- unit
                  end
               end
            end
         end
      end

      class File from IOBase
         meth init(name : Name
                   flags: Flags <= [read]
                   mode : Mode  <= mode(owner:[write] all:[read]))
            IOBase,InitBase
            FD = case Name
                 of stdin  then IO.stdin
                 [] stdout then IO.stdout
                 [] stderr then IO.stderr
                 else {IO.open file(Name flags:Flags mode:Mode)} end
         in
            ReadFD  <- FD
            WriteFD <- FD
         end
         meth !Init(Which FD)
            IOBase,InitBase
            case Which
            of stdin  then WriteFD <- FD
            [] stdout then ReadFD <- FD
            [] stderr then ReadFD <- FD
            end
         end
      end

      %%
      %% Each of stdin, stdout, and stderr can be redirected in
      %% several ways:
      %%
      %% (1) an atom (one of stdin, stdout, stderr) indicates
      %%     redirection to the corresponding fd
      %% (2) `true' means to redirect to the process object
      %% (3) `false' means to redirect to /dev/null
      %% (3) `io(FD)' indicates that a new IO stream should be created
      %%     and that FD should be bound to it
      %% (4) `open(OBJ)' indicates that a new File object should be
      %%     created and that OBJ should be bound to it
      %%
      %% FROM is one of stdin, stdout, or stderr and indicates the
      %%        stream to be redirected
      %% TO is a spec as described above that specifies the redirection
      %% PROC_FD is the descriptor to use for spec (2) `true' and
      %%        corresponds to a redirection to the process object
      %% the result is a pair USE#CLOSE of descriptors. USE is the
      %% descriptor to be used for redirection by the child process and
      %% CLOSE is a boolean that indicates whether the parent should
      %% close USE after spawning off the child.  USE may also be unit
      %% to indicate that no redirection should take place.

      fun {ProcessFD FROM TO PROC_FD}
         %% FROM is one of stdin, stdout, or stderr
         if FROM==TO then unit#false
         elsecase TO
         of true   then PROC_FD#false
         [] false  then FD={IO.devNull} in FD#true
         [] stdin  then IO.stdin#false
         [] stdout then IO.stdout#false
         [] stderr then IO.stderr#false
         [] io(FD) then L R in
            {IO.socketpair L R} FD=L R#true
         [] open(OBJ) then L R in
            {IO.socketpair L R}
            {New File Init(FROM L) OBJ} R#true
         end
      end

      class Process from IOBase
         attr process
         meth init(1:Cmd
                   2:Args <= nil
                   %% by default, stdin and stdout are connected to
                   %% this object, and stderr is sent to /dev/null
                   stdin : IN   <= true
                   stdout: OUT  <= true
                   stderr: ERR  <= false)
            IOBase,InitBase
            %%
            SELF_FD PROC_FD
            if IN==true orelse OUT==true orelse ERR==true
            then {IO.socketpair SELF_FD PROC_FD}
            else SELF_FD=unit PROC_FD=unit end
            ReadFD  <- SELF_FD
            WriteFD <- SELF_FD
            %%
            STDIN  # CLOSE1 = {ProcessFD stdin  IN  PROC_FD}
            STDOUT # CLOSE2 = {ProcessFD stdout OUT PROC_FD}
            STDERR # CLOSE3 = {ProcessFD stderr ERR PROC_FD}
         in
            %%
            {IO.run process(Cmd Args
                            stdin : STDIN
                            stdout: STDOUT
                            stderr: STDERR)
             @process}
            if PROC_FD\=unit then {IO.close PROC_FD} end
            if CLOSE1 then {IO.close STDIN } end
            if CLOSE2 then {IO.close STDOUT} end
            if CLOSE3 then {IO.close STDERR} end
         end
         meth status($) {PROC.status @process} end
         meth kill(1:N<=9) {PROC.kill @process N} end
      end
   end
end

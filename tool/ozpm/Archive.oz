functor
export
   'class' : Archive
   Make
   MakeFrom
import
   URL(toVirtualStringExtended make isAbsolute)
   Resolve(expand)
   ZFile at 'zfile.so{native}'
   OS(stat unlink)
   Open(file)
   FileUtils(withSlash:WithSlash fullName:FullName exists:Exists)
define
   fun {Encode F}
      case F
      of nil  then nil
      [] &#|T then &%|&2|&3|{Encode T}
      [] &{|T then &%|&7|&b|{Encode T}
      []  H|T then        H|{Encode T}
      end
   end
   fun {Expand F}
      {URL.toVirtualStringExtended
       {Resolve.expand {Encode {VirtualString.toString F}}}
       o(full:true raw:true)}
   end

   fun {Int2Bytes N}
      B1 = N mod 256
      R1 = N div 256
      B2 = R1 mod 256
      R2 = R1 div 256
      B3 = R2 mod 256
      R3 = R2 div 256
      B4 = R3 mod 256
      R4 = R3 div 256
   in
      if R4\=0 then raise foo end end
      [B1 B2 B3 B4]
   end

   fun {Bytes2Int [B1 B2 B3 B4]}
      ((B4*256+B3)*256+B2)*256+B1
   end

   proc {WriteBytes Z S}
      case {ZFile.write Z S}
      of unit then skip
      [] B then {WriteBytes Z B} end
   end
   
   proc {WriteInt Z N}
      {WriteBytes Z {Int2Bytes N}}
   end
   
   fun {ReadInt Z}
      B1 = {ZFile.getc Z}
      B2 = {ZFile.getc Z}
      B3 = {ZFile.getc Z}
      B4 = {ZFile.getc Z}
   in
      {Bytes2Int [B1 B2 B3 B4]}
   end

   proc {WriteString Z S}
      B = {ByteString.make S}
      N = {ByteString.width B}
   in
      {WriteInt Z N}
      {WriteBytes Z B}
   end

   fun {ReadBytes Z N}
      B = {ZFile.read Z N}
      M = {ByteString.width B}
   in
      if M==N then B
      elseif M==0 then raise zfile end
      else B#{ReadBytes Z N-M} end
   end

   fun {ReadString Z}
      N = {ReadInt Z}
   in
      {ReadBytes Z N}
   end

   proc {WriteFile Z Path}
      O = {New Open.file init(name:Path)}
      proc {Loop}
	 case {O read(list:$)}
	 of nil then {ZFile.flush Z}
	 [] S then {WriteBytes Z S} {Loop} end
      end
   in
      {Loop}
   end
   
   proc {MakeFrom File Files Home}

      fun {FileInfo File}
	 F = {FullName {Expand File} Home}
	 R = {OS.stat F}
      in
	 {Adjoin r(path:{Expand File}
		   fullpath:F) R}
      end
      
      F = {Expand File}
      Infos = {Map Files FileInfo}
      Z = {ZFile.open F "wb9"}
   in
      %% write how many files:
      {WriteInt Z {Length Files}}
      %% write info for each file
      {ForAll Infos
       proc {$ Info}
	  % file name
	  {WriteString Z Info.path}
	  % file size
	  {WriteInt Z Info.size}
       end}
      {ZFile.flush Z}
      %% write each file
      {ForAll Infos
       proc {$ Info}
	  {WriteFile Z Info.fullpath}
       end}
      {ZFile.finish Z}
      {ZFile.close Z}
   end


   proc {Make File Files}
      {MakeFrom File Files ""}
   end

   proc {ReadFile Z Size File}
      O = {New Open.file init(name:File flags:[write truncate create])}
      proc {Loop N}
	 if N==0 then {O close}
	 else
	    M = {Min N 1024}
	    B = {ZFile.read Z M}
	 in
	    {O write(vs:B)}
	    {Loop N-M}
	 end
      end
   in
      {Loop Size}
   end

   class Archive
      attr zfile offset toc
      meth init(File)
	 zfile <- {ZFile.open File "rb9"}
	 toc <- 
	 {Map {List.number 1 {ReadInt @zfile} 1}
	  fun {$ I}
	     Path = {ReadString @zfile}
	     Size = {ReadInt @zfile}
	  in
	     file(path:{VirtualString.toAtom Path} size:Size
		  offset:_
		 )
	  end}
	 offset <- {ZFile.tell @zfile}
	 {FoldL @toc
	  fun {$ N F}
	     F.offset = N
	     N+F.size
	  end @offset _}
	 toc <- {List.toRecord toc
		 {Map @toc
		  fun {$ F} F.path#F end}}
      end
      meth extract(From To)
	 F = @toc.{VirtualString.toAtom From}
      in
	 {ZFile.seek @zfile F.offset 0}
	 {ReadFile @zfile F.size To}
      end
      meth extractAndReplace(From To)
	 F = @toc.{VirtualString.toAtom From}
      in
	 {ZFile.seek @zfile F.offset 0}
	 if {Exists To} then {OS.unlink To} end
	 {ReadFile @zfile F.size To}
      end	 
      meth ls($) {Arity @toc} end
      meth lsla($) {Record.toList @toc} end
      meth close
	 if @zfile\=unit then {ZFile.close @zfile} zfile<-unit end
      end
   end
end

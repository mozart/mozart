functor
export
   Is Make 'class':Path

   ToString
   ToAtom
   Length
   IsAbsolute
   IsRelative
   Dirname
   Basename
   Exists
   Stat
   IsDir
   IsFile
   Size
   Mtime
   Resolve
   Getcwd
   Mkdir
   Mkdirs
   IsRoot
   Readdir
   Extension DropExtension AddExtension
   MaybeAddPlatform

prepare
   VS2S = VirtualString.toString
   S2A  = String.toAtom
   IS_PATH = {NewName}
   Token = String.token
   LLength = List.length

   fun {Is X} {HasFeature X IS_PATH} end

   %% {Split L P} returns a list of subsequences of L
   %% separated by an element for which P is true.
   %% The code was stolen from Christian's Tokens
   %% function from String.oz

   fun {Split L P} S in {SplitAux L P S S} end
   
   fun {SplitAux L P Js Jr}
      case L
      of nil then Jr=nil
	 case Js of nil then nil else [Js] end
      [] H|T then
	 if {P H} then NewJs in
	    Jr=nil Js|{SplitAux T P NewJs NewJs}
	 else NewJr in
	    Jr=H|NewJr {SplitAux T P Js NewJr}
	 end
      end
   end

   fun {IsSlashOrBackslash C} C==&/ orelse C==&\\ end
   fun {IsSlash C} C==&/ end

   fun {SplitWindows S} {Split S IsSlashOrBackslash} end
   fun {SplitPosix   S} {Split S IsSlash} end
import
   OS Property
   Shell at 'Shell.ozf'
define
   
   fun {Make X}
      if {Is X} then X else {New Path init(X)} end
   end

   IS_WINDOWS = {Property.get 'platform.os'}=='win32'

   %% we provide no locking for concurrent processing because,
   %% at worst, we only perform redundant work

   class Path
      feat !IS_PATH:unit
      attr
	 info

	 %% newFromRecord(+R ?P) makes it easier to create instances of
	 %% classes derived from Path
	 
      meth newFromRecord(R $)
	 {New Path initFromRecord(R)}
      end
      meth initFromRecord(R)
	 info <- R
      end

      meth new(S $ windows:WIN<=IS_WINDOWS exact:Exact<=false)
	 {New Path init(S windows:WIN exact:Exact)}
      end

      %% init(...) is the main user-oriented constructor
      
      meth init(S windows:WIN<=IS_WINDOWS exact:Exact<=false)
	 STR1 = {VS2S S}
	 STR2 =
	 if Exact then STR1 else
	    case {Reverse STR1}
	    of nil then nil
	    [] H|T then
	       Pred = if IS_WINDOWS then IsSlashOrBackslash else IsSlash end
	    in
	       if {Pred H} then
		  {Reverse {List.dropWhile T Pred}}
	       end
	    end	       
	 end
	 Drive
	 NonDrive
	 Items Items2 Items3
	 SlashInitial
	 SlashFinal
      in
	 if WIN then
	    case STR2
	    of C|&:|L then Drive=C    NonDrive=L
	    []      L then Drive=unit NonDrive=L
	    end
	 else Drive=unit NonDrive=STR2 end
	 Items = {if WIN then SplitWindows else SplitPosix end
		  NonDrive}
	 case Items
	 of nil|L then SlashInitial=true Items2=L
	 else SlashInitial=false Items2=Items
	 end
	 case {Reverse Items2}
	 of nil|L then SlashFinal=true Items3={Reverse L}
	 else SlashFinal=false Items3=Items2
	 end
	 {self initFromRecord(
		  unit(
		     string       : STR2
		     windows      : WIN
		     drive        : Drive
		     slashinitial : SlashInitial
		     slashfinal   : SlashFinal
		     components   : Items3
		     ))}
      end
      meth toString($) @info.string end
      meth toAtom($) {S2A @info.string} end
      meth length($) {LLength @info.string} end
      meth isAbsolute($) @info.slashinitial end
      meth isRelative($) {Not Path,isAbsolute($)} end
      meth dirname($)
	 INFO = @info
      in
	 if INFO.slashfinal then
	    {self newFromRecord({AdjoinAt INFO slashfinal false} $)}
	 else
	    COM = INFO.components
	    STR
	    INFO2 = {Adjoin INFO
		     unit(
			string       : STR
			slashinitial :
			   if COM==nil then false else INFO.slashinitial end
			slashfinal   : false
			components   :
			   if COM==nil then nil
			   else {Reverse {Reverse COM}.2} end
			)}
	 in
	    Path,ComputeString(INFO2 STR)
	    {self newFromRecord(INFO2 $)}
	 end
      end
      meth ComputeString(INFO $)
	 INFO = @info
	 DEV = INFO.drive
	 INI = INFO.slashinitial
	 FIN = INFO.slashfinal
	 COM = INFO.components
	 L1  = if DEV==unit then nil else [DEV &:] end
	 L2  = if INI then L1#'/' else L1 end
	 L3  = L2 # {FoldL COM
		     fun {$ Accu C}
			Accu#(if Accu==nil then C else Accu#'/'#C end)
		     end nil}
	 L4  = if FIN then L3#'/' else L3 end
      in
	 {VS2S L4}
      end
      meth basenameString($)
	 INFO = @info
      in
	 if INFO.slashfinal then nil else
	    case {Reverse INFO.components}
	    of nil then nil
	    [] H|_ then [H] end
	 end
      end
      meth basename($)
	 INFO = @info
      in
	 if INFO.slashfinal then
	    {self new(nil $ window:INFO.windows)}
	 else
	    STR
	    INFO2 = unit(
		       string       : STR
		       drive        : unit
		       slashinitial : false
		       slashfinal   : false
		       components   :
			  case {Reverse INFO.components}
			  of nil then nil
			  [] H|_ then [H] end
		       windows      : INFO.windows)
	 in
	    Path,ComputeString(INFO2 STR)
	    {self newFromRecord(INFO2 $)}
	 end
      end
      meth exists($)
	 try Path,stat(_) true catch _ then false end
      end
      meth stat($)
	 {OS.stat @info.string}
      end
      meth isDir($)
	 (Path,stat($)).type == 'dir'
      end
      meth isFile($)
	 (Path,stat($)).type == 'reg'
      end
      meth size($)
	 (Path,stat($)).size
      end
      meth mtime($)
	 (Path,stat($)).mtime
      end
      meth GetInfo($) @info end
      meth resolve(X $)
	 P = {Make X}
      in
	 if {P isAbsolute($)} then P else
	    INFO  = @info
	    INFO2 = {P GetInfo($)}
	    STR
	    INFO3 = unit(
		       string       : STR
		       drive        : INFO.drive
		       slashinitial : INFO.slashinitial
		       slashfinal   : INFO2.slashfinal
		       components   : {Append INFO.components INFO2.components}
		       windows      : INFO.windows)
	 in
	    Path,ComputeString(INFO3 STR)
	    {self newFromRecord(INFO3 $)}
	 end
      end
      meth getcwd($)
	 {Getcwd}
      end
      meth mkdir()
	 {OS.mkDir @info.string}
      end
      meth mkdirs()
	 if @info.components==nil then skip else
	    {Path,dirname($) mkdirs}
	 end
	 if Path,exists($) then skip else
	    Path,mkdir
	 end
      end
      meth rmdir()
	 R = {CondSelect OS rmDir unit}
      in
	 if R==unit then
	    {Shell.executeCommand ['rmdir' @info.string]}
	 else
	    {R @info.string}
	 end
      end
      meth isRoot($)
	 @info.components==nil
      end
      meth readdir($)
	 for S in {OS.getDir @info.string} collect:COL do
	    if S=="." orelse S==".." then skip else
	       {COL Path,resolve(S $)}
	    end
	 end
      end
      meth SplitExtension(Base Ext)
	 BaseSTR = Path,basenameString($)
      in
	 if  {Member &. BaseSTR} then S1 S2 in
	    {Token {Reverse BaseSTR} &. S1 S2}
	    Base={Reverse S2} Ext={Reverse S1}
	 else Base=BaseSTR Ext=unit end
      end
      meth extension($)
	 Path,SplitExtension(_ $)
      end
      meth dropExtension($)
	 {Path,dirname($) resolve(Path,SplitExtension($ _) $)}
      end
      meth addExtension(Ext $)
	 {Path,dirname($) resolve((Path,basenameString($))#'.'#Ext $)}
      end
      meth maybeAddPlatform($)
	 case Path,extension($)
	 of "so" then
	    {Path,dirname($)
	     resolve((Path,basenameString($))
		     #'-'#{Property.get 'platform.name'} $)}
	 else self end
      end
      meth makeExecutable()
	 try {Shell.executeCommand ['chmod' '+x' @info.string]}
	 catch E then
	    if IS_WINDOWS then skip else
	       raise E end
	    end
	 end
      end
   end

   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

   fun {ToString   P} {{Make P} toString($)} end
   fun {ToAtom     P} {{Make P} toAtom($)} end
   fun {Length     P} {{Make P} length($)} end

   fun {IsAbsolute P} {{Make P} isAbsolute($)} end
   fun {IsRelative P} {{Make P} isRelative($)} end
   fun {Dirname    P} {{{Make P} dirname($)} toString($)} end
   fun {Basename   P} {{{Make P} basename($)} toString($)} end
   fun {Exists     P} {{Make P} exists($)} end
   fun {Stat       P} {{Make P} stat($)} end
   fun {IsDir      P} {{Make P} isDir($)} end
   fun {IsFile     P} {{Make P} isFile($)} end
   fun {Size       P} {{Make P} size($)} end
   fun {Mtime      P} {{Make P} mtime($)} end
   fun {Resolve P1 P2} {{{Make P1} resolve(P2 $)} toString($)} end
   fun {Getcwd} {Make {OS.getCWD}} end
   fun {Mkdir      P} {{Make P} mkdir} end
   fun {Mkdirs     P} {{Make P} mkdirs} end
   fun {IsRoot     P} {{Make P} isRoot($)} end
   fun {Readdir    P} {{Make P} readdir($)} end
   fun {Extension  P} {{Make P} extension($)} end
   fun {DropExtension P} {{{Make P} dropExtension($)} toString($)} end
   fun {AddExtension P E} {{{Make P} addExtension(E $)} toString($)} end
   fun {MaybeAddPlatform P} {{{Make P} maybeAddPlatform(P $)} toString($)} end
end

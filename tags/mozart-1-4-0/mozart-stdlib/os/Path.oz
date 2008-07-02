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
   Rmdir

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

   fun {IsWinDelimiter C} C==&/ orelse C==&\\ end
   fun {IsPosixDelimiter C} C==&/ end

   fun {SplitWindows S} {Split S IsWinDelimiter} end
   fun {SplitPosix   S} {Split S IsPosixDelimiter} end

   fun {PathToString P} {P toString($)} end

   %% computes full path from given path instance info record
   fun {ComputeString INFO}
      Delimiter = if INFO.windows then '\\' else '/' end 
      Drive = if INFO.drive==unit then nil else [[INFO.drive &:]] end
      Initial = if INFO.slashinitial then [Delimiter] else nil end
      Body = if INFO.components==nil
	     then nil
	     else INFO.components.1 | {Map INFO.components.2 fun {$ X} Delimiter#X end}
	     end
      Final = if (INFO.slashfinal andthen INFO.exact) then [Delimiter] else nil end
   in
      {VS2S
       {FoldL % concatenate to single VS
	{FoldL % remove all nil
	 [Drive Initial Body Final] Append nil}
	fun {$ X Y} X#Y end ""}}
   end
   
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
	 %% info is record of form
% 	  unit(
%	     %% path as string (with or without optional terminal slash according to Exact)
% 	     string       : STR
% 	     %% B, defaults to true on windows
% 	     windows      : WIN
% 	     %% Win drive letter or unit
% 	     drive        : Drive 
% 	     %% B, true for path starting with slash/backslash
% 	     slashinitial : SlashInitial
% 	     %% B, true for path ending with slash/backslash 
% 	     slashfinal   : SlashFinal
% 	     %% list of strings dir names (and possibly basename) without starting and ending slash/backslash 
% 	     components   : Items
%	     %% B, indicates whether terminal slash is printed when transforming path back to string
%            exact: Exact	 
% 	     )
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
      %%
      %% [TMP: implementation changed: exact has only influence of computation of output string with ComputeString. However, the SlashFinal is set nevertheless always to true when input path ends with slash]
      %%
      meth init(S windows:WIN<=IS_WINDOWS exact:Exact<=false)
	 STR1 = {VS2S S}
	 IsLastCharSlash	% aux: B, denoting terminal slash
	 STR2			% STR1 without terminal slash
	 Drive			% Win Drive letter
	 NonDrive		% aux: Path after Drive
	 Items			% aux: path components with SlashInitial indicator
	 Items2			% path components 
	 SlashInitial		
	 SlashFinal		
      in
	 %% set IsLastCharSlash and STR2: 
	 %% if Exact=false and STR1 is neither nil nor can be '/', then STR2 is STR without terminal slash, otherwise STR2 = STR1
	 if {LLength STR1} < 2  % test for '/' and nil
	 then
	    IsLastCharSlash = false
	    STR2 = STR1
	 else LastChar = {Reverse STR1}.1
	 in
	    IsLastCharSlash = ((WIN andthen {IsWinDelimiter LastChar})
			       orelse
			       ({Not WIN} andthen {IsPosixDelimiter LastChar}))
	    STR2 = if {Not Exact} andthen IsLastCharSlash
		   then {Reverse {Reverse STR1}.2}
		   else STR1
		   end
	 end
	 %% set Drive and NonDrive
	 if WIN then
	    case STR1
	    of DriveLetter|&:|L then Drive=DriveLetter    NonDrive=L
	    []      L then Drive=unit NonDrive=L
	    end
	 else Drive=unit NonDrive=STR1 end
	 %% set Items
	 Items = {if WIN then SplitWindows else SplitPosix end
		  NonDrive}
	 %% set SlashInitial and Items2 
	 case Items
	 of nil|L then SlashInitial=true Items2=L
	 else SlashInitial=false Items2=Items
	 end
	 %% set SlashFinal
	 %% [TMP: changed: Split never returns nil as last element if path ends in single slash/backslash..]
	 if IsLastCharSlash
	 then SlashFinal=true
	 else SlashFinal=false
	 end
	 %% create path instance 
	 {self initFromRecord(
		  unit(
		     string       : STR2
		     windows      : WIN
		     drive        : Drive
		     slashinitial : SlashInitial
		     slashfinal   : SlashFinal
		     components   : Items2
		     exact        : Exact
		     ))}
      end
      
      meth toString($) @info.string end
      meth toAtom($) {S2A @info.string} end
      meth length($) {LLength @info.string} end
      meth isAbsolute($) @info.slashinitial end
      meth isRelative($) {Not Path,isAbsolute($)} end
      meth dirnameString($)
	 {{self dirname($)} toString($)}
      end
      meth dirname($)
	 INFO = @info
	 COM = INFO.components
      in
	 if {LLength COM} == 1
	 then {Make nil}
	 else 
	    STR
	    INFO2 = {Adjoin INFO
		     unit(
			string       : STR
			slashfinal   :
			   %% check for root
			   if COM==nil then false else true end
			components   :
			   if COM==nil then nil
			   else {Reverse {Reverse COM}.2} end
			)}
	 in
	    {ComputeString INFO2 STR}
	    {self newFromRecord(INFO2 $)}
	 end
      end
      %% [TMP: changed def: return plain string, not wrapped in a list]  
      meth basenameString($)
	 {{self basename($)} toString($)}
      end
      %% [TMP: changed def: UNIX basename always returns last component of path, even if path ends with dir]
      meth basename($)
	 INFO = @info
	 COM = INFO.components
	 STR
	 INFO2 = {Adjoin INFO
		  unit(string       : STR
		       drive        : unit
		       slashinitial :
			  %% check for root case
		          if COM==nil then INFO.slashinitial else false end
		       components   :
			  if COM==nil then nil
			  else [{Reverse COM}.1] end
		      )}
      in
	 {ComputeString INFO2 STR}
	 {self newFromRecord(INFO2 $)}
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
      meth isDir2($)
	 @info.slashfinal orelse {self isRoot($)}
      end
      meth isFile($)
	 (Path,stat($)).type == 'reg'
      end
      meth isFile2($)
	 {Not @info.slashfinal}
      end
      meth size($)
	 (Path,stat($)).size
      end
      meth mtime($)
	 (Path,stat($)).mtime
      end
      
      %% tmp meth for debugging
      /* meth getInfo($) @info end */
      meth GetInfo($) @info end
      meth resolve(X $)
	 P = {Make X}
      in
	 if {P isAbsolute($)} then P else
	    INFO  = @info
	    INFO2 = {P GetInfo($)}
	    STR
	    INFO3 = {Adjoin INFO
		     unit(
			string       : STR
			slashfinal   : INFO2.slashfinal
			components   : {Append INFO.components INFO2.components}
			)}
	 in
	    {ComputeString INFO3 STR}
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
	 @info.components==nil andthen @info.slashinitial
      end
      meth readdir($)
	 INFO = @info
      in
	 for S in {OS.getDir INFO.string} collect:COL do
	    if S=="." orelse S==".." then skip
	    else
	       P_S = {Make S}
	       STR_Result
	       INFO_S = {P_S GetInfo($)}
	       INFO_Result = {Adjoin INFO
			      unit(string       : STR_Result
				   slashfinal   : Slashfinal_Result
				   components   : {Append INFO.components
						   INFO_S.components})}
	       %% if S is dir then set finalslash accordingly
	       Slash = if INFO.exact then nil else
			  if INFO.windows then '\\' else '/' end 
		       end
	       Slashfinal_Result = {OS.stat INFO.string#Slash#S}.type == 'dir'
	    in
	       {ComputeString INFO_Result STR_Result}
	       {COL {self newFromRecord(INFO_Result $)}}
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
   fun {Getcwd} {Make {OS.getCWD}#'/'} end
   proc {Mkdir     P} {{Make P} mkdir} end
   proc {Mkdirs    P} {{Make P} mkdirs} end
   fun {IsRoot     P} {{Make P} isRoot($)} end
   fun {Readdir    P} {Map {{Make P} readdir($)} PathToString} end
   fun {Extension  P} {{Make P} extension($)} end
   fun {DropExtension P} {{{Make P} dropExtension($)} toString($)} end
   fun {AddExtension P E} {{{Make P} addExtension(E $)} toString($)} end
   fun {MaybeAddPlatform P} {{{Make P} maybeAddPlatform(P $)} toString($)} end
   proc {Rmdir     P} {{Make P} rmdir} end
end

%%% The point of this functor is to automatically compute dependencies
%%% For the moment, we only do this for Oz functors.
functor
import
   Compiler URL
   DefaultURL
   Path at 'Path.ozf'
   Utils at 'Utils.ozf'
prepare
   VS2A = VirtualString.toAtom
   DictKeys = Dictionary.keys
   NODEPS = o(build:nil install:nil)
export
   'class' : Depends
define

   proc {NoNarrator Msg}
      if {Label Msg}==error then raise parseOzFile(Msg) end end
   end

   fun {SwitchProc _} false end

   fun {IsMozartModule N}
      {Member N DefaultURL.functorNames}
   end

   class Depends
      attr Target2Depends:unit
	 
      meth Get(Target $)
	 if {self get_autodepend($)} then
	    case {CondSelect @Target2Depends Target unit}
	    of unit then
	       Deps =
	       case {Path.extensionAtom Target}
	       of 'ozf' then
		  A1 = {Path.replaceExtension Target 'oz'}
		  DIR  = {Path.dirname A1}
		  A2 = {Path.resolve {self get_srcdir($)} A1}
	       in
		  if {Path.exists A2} then
		     {self vtrace('scanning '#A1#' for dependencies')}
		  in
		     local X=Depends,GetOzDeps(A2 DIR $) in
			if {self get_veryVerbose($)} then
			   for U in X.build do
			      {self vtrace('... '#U#' (when building)')}
			   end
			   for U in X.install do
			      {self vtrace('... '#U#' (when installing)')}
			   end
			end
			X
		     end
		  else NODEPS end
	       else NODEPS end
	    in
	       if @Target2Depends==unit then
		  Target2Depends <- {NewDictionary}
	       end
	       @Target2Depends.Target := Deps
	       Deps
	    [] Deps then Deps
	    end
	 else NODEPS end
      end
      meth get_autodepend_build(SRC $)
	 (Depends,Get(SRC $)).build
      end
      meth get_autodepend_install(SRC $)
	 (Depends,Get(SRC $)).install
      end
      meth ParseOzFile(SRC $)
	 {Compiler.parseOzFile SRC
	  NoNarrator SwitchProc {NewDictionary}}
      end
      meth GetOzDeps(SRC DIR $)
	 ITable = {NewDictionary}
	 RTable = {NewDictionary}
      in
	 try E={self ParseOzFile(SRC $)} in
	    case E of [fFunctor(_ L _)] then
	       for X in L do
		  case X
		  of fImport(L _)  then {self Process(L ITable)}
		  [] fRequire(L _) then {self Process(L RTable)}
		  else skip end
	       end
	    end
	 catch parseOzFile(Msg) then
	    raise parseOzFile(SRC Msg) end
	 end
	 %%
	 %% add SRC's base to all the collected deps
	 %%
	 local
	    fun {Normalize U}
	       {Path.resolveAtom DIR {Path.dropInfoURL {URL.make U}}}
	    end
	    Imports  = {Map {DictKeys ITable} Normalize}
	    Requires = {Map {DictKeys RTable} Normalize}
	 in
	    o(install : Imports
	      build   : Requires)
	 end
      end

      meth Process(L Table)
	 for fImportItem(fVar(A _) _ AT) in L do
	    case AT
	    of fNoImportAt then
	       if {IsMozartModule A} then skip else
		  Table.{VS2A A#'.ozf'} := unit
	       end
	    [] fImportAt(fAtom(U _)) then
	       if {Path.isRelative U} then
		  Table.U := unit
	       end
	    end
	 end
      end

      meth get_depends_rec(T $)
	 Table = {NewDictionary}
	 Stack = {Utils.newStack}
      in
	 for X in {self get_depends(T $)} do
	    {Stack.push X}
	 end
	 for while:{Not {Stack.isEmpty}} do
	    T = {Stack.pop}
	 in
	    if {HasFeature Table T} then skip else
	       Table.T := unit
	       for X in {self get_depends(T $)} do
		  {Stack.push X}
	       end
	       for X in {self get_autodepend_install(T $)} do
		  {Stack.push X}
	       end
	    end
	 end
	 {DictKeys Table}
      end
   end
end

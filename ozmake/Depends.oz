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
   DictRemove = Dictionary.remove
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
	       Rule = {self get_rule(Target $)}
	       Deps =
	       case Rule.tool
	       of 'ozc' then
		  A1  = Rule.file
		  DIR = {Path.dirname A1}
		  A2  = {Path.resolve {self get_srcdir($)} A1}
	       in
		  if {Path.exists A2} then
		     {self vtrace('scanning '#A1#' for dependencies')}
		     X = Depends,GetOzDeps(A2 DIR $)
		  in
		     if {self get_veryVerbose($)} then
			for U in X.build do
			   {self vtrace('... '#U#'\t[Build Time]')}
			end
			for U in X.install do
			   {self vtrace('... '#U#'\t[Run Time]')}
			end
		     end
		     X
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
	 try
	    for E in {self ParseOzFile(SRC $)} do
	       case E of fFunctor(_ L _) then
		  for X in L do
		     case X
		     of fImport(L _)  then {self Process(L ITable)}
		     [] fRequire(L _) then {self Process(L RTable)}
		     else skip end
		  end
	       [] dirSwitch(_) then skip
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

      %% get_depends_build(T $)
      %%
      %% return the build-time dependencies of target T.  If T is an
      %% ozl-linked ozf file, then we want to compute the recursive
      %% run-time ozf imports.  If one of the recursive imports is
      %% again an ozl-linked ozf file, then we don't recurse through
      %% it since it already contains all its recursive imports.
      %%
      %% if T is a regular ozf file, then we also want the recursive
      %% run-time dependencies of its build-time dependencies

      meth get_depends_build(T $)
	 Rule = {self get_rule(T $)}
      in
	 case Rule.tool
	 of 'ozl' then {self get_depends_ozl(T $)}
	 [] 'ozc' then {self get_depends_ozc(T $)}
	 else {self get_depends(T $)} end
      end

      %% get_depends_install_just_so(T $)
      %%
      %% return the recursive run-time .so{native} imports.  All
      %% other (ozf) imports are already included in T, an ozl-linked
      %% ozf file.

      meth get_depends_install_just_so(T $)
	 Done = {NewDictionary}
	 Todo = {Utils.newStack}
	 Accu = {NewDictionary}
      in
	 {Todo.push T}
	 for while:{Not {Todo.isEmpty}} do
	    D = {Todo.pop}
	 in
	    if {HasFeature Done D} then skip else
	       Done.D := unit
	       %% recurse with install dependencies
	       for X in {self get_autodepend_install(D $)} do
		  {Todo.push X}
	       end
	       %% take note if this is a .so{native} file
	       %% i.e. if it is built using the ld tool
	       if {self get_rule(D $)}.tool=='ld' then
		  Accu.D := unit
	       end
	    end
	 end
	 {DictKeys Accu}
      end

      %% get_depends_install(T $)
      %%
      %% return the run-time dependencies of target T, i.e. all
      %% additional files which need to be installed in order for
      %% T to be usable at run-time.

      meth get_depends_install(T $)
	 Done = {NewDictionary}
	 Todo = {Utils.newStack}
	 Accu = {NewDictionary}
      in
	 {Todo.push T}
	 for while:{Not {Todo.isEmpty}} do
	    D = {Todo.pop}
	 in
	    if {HasFeature Done D} then skip else
	       Done.D := unit
	       Rule = {self get_rule(D $)}
	    in
	       case Rule.tool
	       of 'ozc' then
		  %% for a regular ozf file, take note of it and
		  %% recurse with its imports
		  Accu.D := unit
		  for X in {self get_autodepend_install(D $)} do
		     {Todo.push X}
		  end
	       [] 'ozl' then
		  %% for a ozl-linked ozf file, take note of it
		  %% and of its (recursive) .so imports which are
		  %% the only recursive imports not already included
		  Accu.D := unit
		  for X in {self get_depends_install_just_so(D $)} do
		     Accu.X := unit
		  end
	       [] 'ozg' then
		  %% for an ozg created file, i.e. created as a
		  %% side-effect of building another file, just
		  %% take note of it - this might not be entirely
		  %% accurate, but we would have to examine the
		  %% created file itself rather than the source
		  %% to determine the run-time dependencies if
		  %% any and we would need to know more about the
		  %% side-effecting mechanism which created the
		  %% file in the first place
		  Accu.D := unit
	       [] 'ld' then
		  %% for a .so{native} functor, just take note of it
		  Accu.D := unit
	       [] 'cc' then skip
	       [] unit then skip
	       end
	    end
	 end
	 %% due to the way the loop is started, the argument target T
	 %% normally ends up being recorded.  we remove it now, since
	 %% it does not count as a run-time dependency of itself.
	 {DictRemove Accu T}
	 {DictKeys Accu}
      end

      %% get_depends_ozl(T $)
      %%
      %% given an ozl-linked target T, return its build-time
      %% dependencies, i.e. mostly its ozf source and its the
      %% recursive ozf imports.  If a recursive ozf import is an
      %% ozl-linked file we do not recurse through it since it already
      %% contains all its recursive ozf imports.

      meth get_depends_ozl(T $)
	 Done = {NewDictionary}
	 Todo = {Utils.newStack}
	 Accu = {NewDictionary}
	 Rule = {self get_rule(T $)}
      in
	 %% this method is supposed to be called only for a target
	 %% which is created by tool ozl
	 if Rule.tool==ozl then
	    %% start with all normal build dependencies
	    for X in {self get_depends(T $)} do
	       %% take note anyway since they may not be ozf files
	       Accu.X := unit
	       {Todo.push X}
	    end
	    %% add run-time dependencies of source ozf file
	    for X in {self get_autodepend_install(Rule.file $)} do
	       {Todo.push X}
	    end
	 else
	    raise ozmake(get_depends_ozl:T) end
	 end
	 for while:{Not {Todo.isEmpty}} do
	    D = {Todo.pop}
	 in
	    if {HasFeature Done D} then skip else
	       Rule = {self get_rule(D $)}
	    in
	       Done.D := unit
	       case Rule.tool
	       of 'ozc' then
		  Accu.D := unit
		  %% recurse with the imports, i.e. run-time
		  %% dependencies
		  for X in {self get_autodepend_install(D $)} do
		     {Todo.push X}
		  end
	       [] 'ozl' then
		  Accu.D := unit
	       [] 'ozg' then
		  Accu.D := unit
	       else skip end
	    end
	 end
	 {DictKeys Accu}
      end

      %% get_depends_ozc(T $)
      %%
      %% given a regular ozf file T to be built using ozc, return
      %% its build-time dependencies.  This includes the recursive
      %% run-time dependencies of its build-time dependencies.

      meth get_depends_ozc(T $)
	 Todo = {Utils.newStack}
	 Accu = {NewDictionary}
	 Rule = {self get_rule(T $)}
      in
	 if Rule.tool==ozc then
	    for X in {self get_depends(T $)} do
	       {Todo.push X}
	    end
	 else
	    raise ozmake(get_depends_ozc:T) end
	 end
	 for while:{Not {Todo.isEmpty}} do
	    D = {Todo.pop}
	 in
	    if {HasFeature Accu D} then skip else
	       Accu.D := unit
	       for X in {self get_depends_install(D $)} do
		  {Todo.push X}
	       end
	    end
	 end
	 {DictKeys Accu}
      end
   end
end

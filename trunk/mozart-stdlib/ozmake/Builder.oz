functor
export
   'class' : Builder
import
   Path at 'Path.ozf'
define
   class Builder

      meth build(Targets)
	 {self makefile_read}
	 if Targets==nil then Builder,build_all
	 else for T in Targets do Builder,build_target(T) end end
      end

      meth build_all
	 if {self get_includelibs($)} then
	    for T in {self get_lib_targets($)} do
	       Builder,build_target(T runtime:true)
	    end
	 end
	 if {self get_includebins($)} then
	    for T in {self get_bin_targets($)} do
	       Builder,build_target(T runtime:true)
	    end
	 end
	 {self recurse(build_all)}
      end

      %% input arg runtime indicates whether we should also build the
      %% runtime dependencies.
      
      meth build_target(T runtime:RunTime<=false)
	 {self extend_resolver}
	 {self trace('target '#T)}
	 {self incr}
	 try
	    if {Not {self get_fullbuild($)}} andthen {self target_is_src(T $)} then
	       {self make_src(T _)}
	    else
	       R={self get_rule(   T $)}
	       L=if R.tool==ozl
		 then {self get_depends_rec(T $)}
		 else {self get_depends(    T $)} end
	    in
	       for D in L do
		  Builder,build_target(D runtime:RunTime)
	       end
	       if Builder,Outdated(T L $) then
		  {self exec_rule(T R)}
		  if Builder,Outdated(T L $) then
		     raise ozmake(build:outdated(T)) end
		  end
	       else {self trace(T#' is up to date')} end
	    end
	    if RunTime then
	       for D in {self get_autodepend_install(T $)} do
		  Builder,build_target(D runtime:true)
	       end
	    end
	 finally {self decr} end
      end

      meth Outdated(Target Deps $)
	 SrcDir = {self get_srcdir($)}
	 DstDir = {self get_builddir($)}
	 T_full = {Path.maybeAddPlatform Target}
	 T_src  = {Path.resolve SrcDir T_full}
	 T_dst  = {Path.resolve DstDir T_full}
	 T_file = if     {self exec_exists(T_dst $)} then T_dst
		  elseif {self exec_exists(T_src $)} then T_src
		  else unit end
	 /*
	 T_file = if {Path.exists T_dst} orelse
		     ({self get_justprint($)} andthen {self simulated_exists(T_dst $)})
		  then T_dst
		  elseif {Path.exists T_src} orelse
		     ({self get_justprint($)} andthen {self simulated_exists(T_src $)})
		  then T_src
		  else unit end
	 */
      in
	 if T_file==unit then
	    {self trace(Target#' is missing')}
	    true
	 else
	    T_mtime = try {Path.stat T_file}.mtime
		      catch _ then {self get_simulated_mtime(T_file $)} end
	 in
	    for D in Deps default:false return:Return do
	       D_full = {Path.maybeAddPlatform D}
	       D_src  = {Path.resolve SrcDir D_full}
	       D_dst  = {Path.resolve DstDir D_full}
	       D_file = if {Path.exists D_dst} orelse
			   ({self get_justprint($)} andthen {self simulated_exists(D_dst $)})
			then D_dst
			elseif {Path.exists D_src} orelse
			   ({self get_justprint($)} andthen {self simulated_exists(D_src $)})
			then D_src
			else unit end
	       in
	       if D_file\=unit then
		  D_mtime = try {Path.stat D_file}.mtime
			    catch _ then {self get_simulated_mtime(D_file $)} end
	       in
		  if T_mtime<D_mtime then
		     {self trace(D_file#' is newer')}
		     {Return true}
		  end
	       end
	    end
	 end
      end

   end
end

functor
export
   'class' : Cleaner
import
   Path  at 'Path.ozf'
prepare
   fun {Matches RE STR}
      case RE
      of &?|RE then
	 case STR of _|STR then {Matches RE STR} else false end
      [] &*|RE2 then
	 {Matches RE2 STR} orelse
	 case STR of _|STR then {Matches RE STR} else false end
      [] C|RE then
	 case STR of !C|STR then {Matches RE STR} else false end
      [] nil then STR==nil
      end
   end
   VSToString = VirtualString.toString
   DEFAULT_CLEAN = ["*~" "*.ozf" "*.o" "*.so-*" "*.exe"]
define
   class Cleaner

      %% given a list L of pseudo-glob patterns, delete all
      %% files in build directory that match one of them.

      meth CleanPatterns(L)
	 B = {self get_builddir($)}
      in
	 for F in {Path.dir if B==nil then '.' else B end} do
	    FF = {Path.resolve B F}
	 in
	    if {Path.isDir FF} then skip else
	       for P in L break:Break do
		  if {Matches P F} then
		     {self exec_rm(FF)}
		     {Break}
		  end
	       end
	    end
	 end
      end

      meth GetCleanPatterns($)
	 L={self get_clean($)}
      in
	 if L==unit then DEFAULT_CLEAN else {Map L VSToString} end
      end

      meth GetVeryCleanPatterns($)
	 L={self get_veryclean($)}
      in
	 if L==unit then nil else {Map L VSToString} end
      end

      meth clean
	 {self makefile_read}
	 Cleaner,CleanPatterns(Cleaner,GetCleanPatterns($))
	 {self recurse(clean)}
      end

      meth veryclean
	 Cleaner,clean
	 Cleaner,CleanPatterns(Cleaner,GetVeryCleanPatterns($))
	 {self recurse(veryclean)}
      end
   end
end
functor
import
   Utils at 'Utils.ozf'
   Path  at 'Path.ozf'
   Manager at 'Manager.ozf'
export
   'class' : Requires
prepare
   DictKeys = Dictionary.keys
define
   class Requires

      meth fork($)
	 Args = {self get_args($)}
	 Man = {New Manager.'class' init}
      in
	 for Key#Set#_#OnFork in {self get_optlist($)} do
	    if OnFork andthen {HasFeature Args Key} then
	       {Man Set(Args.Key)}
	    end
	 end
	 Man
      end

      meth InstallRequire(MogulID)
	 {self trace('installing missing required package: '#MogulID)}
	 {self incr}
	 try
	    Man = {self fork($)}
	 in
	    {Man set_grade(any)}
	    {Man set_package(MogulID)}
	    {Man install(nil)}
	 finally {self decr} end
      end

      meth install_requires()
	 if {self get_dorequires($)} then
	    Reqs = {self get_requires($)}
	 in
	    if Reqs==unit then skip else
	       for R in Reqs do
		  if {Utils.isMogulID R} then
		     Key={Path.toAtom R}
		  in
		     if {self database_get_package(Key $)}==unit then
			Requires,InstallRequire(Key)
		     end
		  end
	       end
	    end
	    {self recurse(install_requires)}
	 end
      end
   end
end

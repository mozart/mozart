functor
export
   'class' : Config
prepare
   fun {Return X} X end
define
   class Config
      attr
	 DB : unit

      meth config_read()
	 if @DB==unit then
	    F = {self get_configfile($)}
	    L = {self databaselib_read(F Return $)}
	 in
	    case L of E|_ then
	       DB <- {Record.toDictionary E}
	    else skip end
	 end
      end

      meth config_save()
	 if @DB\=unit then
	    F = {self get_configfile($)}
	    R = {Dictionary.toRecord o @DB}
	 in
	    {self databaselib_save(F Return [R])}
	 end
      end

      meth config()
	 {self config_read}
	 case {self get_mogulpkgurl($)}
	 of unit then skip
	 [] U then @DB.pkgurl := U end
	 case {self get_moguldocurl($)}
	 of unit then skip
	 [] U then @DB.docurl := U end
	 case {self get_mogulsecurl($)}
	 of unit then skip
	 [] U then @DB.securl := U end
      end
   end
end

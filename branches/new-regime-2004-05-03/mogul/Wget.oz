functor
import
   OS(system getCWD chDir)
   Admin(manager:Manager)
   URL(make)
   Directory(mkDir:MkDir)
export
   WgetPkg WgetDoc
define
   proc {WgetPkg PkgUrl PkgDir PkgFile}
      if {Manager ignoreURL(PkgUrl $)} then
	 {Manager trace('Ignoring URL '#PkgUrl)}
      else
	 {MkDir PkgDir}
	 Cmd = {Manager get_wget($)}
	 #if {Manager is_verbose($)} then ' -v' else ' -nv' end
	 #' -N -nH -nd -P "'#PkgDir#'" "'#PkgUrl#'" -O "'#(PkgDir#'/'#PkgFile)#'"'
      in
	 {Manager trace(Cmd)}
	 try
	    if {OS.system Cmd}\=0 then raise oops end end
	 catch _ then {Raise mogul(wget_pkg(PkgUrl))} end
      end
   end
   proc {WgetDoc DocUrl DocDir}
      if {Manager ignoreURL(DocUrl $)} then
	 {Manager trace('Ignoring URL '#DocUrl)}
      else
	 CurDir={OS.getCWD}
	 {MkDir DocDir}
	 Cuts = {Length {URL.make DocUrl}.path} - 1
	 Cmd  = {Manager get_wget($)}
	 #if {Manager is_verbose($)} then ' -v' else ' -nv' end
	 #' -N -nH --cut-dirs='#Cuts#' -r -p -k --no-parent -Q 2m "'#DocUrl#'"'
      in
	 {Manager trace(Cmd)}
	 try
	    {OS.chDir DocDir}
	    if {OS.system Cmd}\=0 then raise oops end end
	 catch _ then {Raise mogul(wget_doc(DocUrl))}
	 finally {OS.chDir CurDir} end
      end
   end
end

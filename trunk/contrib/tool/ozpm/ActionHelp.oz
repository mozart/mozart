functor
export
   Run
import
   System(showInfo:Print)
   Application(exit)
define
   Help = [""
	   "ozpm is the Oz package manager"
	   ""
	   "ozpm                        : starts ozpm in interactive mode"
	   "ozpm --help|-h              : displays this help"
	   "ozpm --install|-i --in file : installs the package File. Install can be forced by --force"
	   "ozpm --remove --in pkg      : removes package whose name or package file is pkg"
	   "ozpm --list                 : list all installed packages"
	   "ozpm --info --in pkg        : displays informations for package whose name or package file is pkg"
	   "ozpm --check                : check package system's integrity"
	   ""
	   "ozpm --create --in pkgdesc  : creates the package file described in the text file pkgdesc"
	   "ozpm --view --in pkgfile    : displays informations about the package file pkgfile"
	   ""
	  ]
   proc {Run}
      {ForAll Help Print}
      {Application.exit 0}
   end
end

functor
export
   'class' : Installer
import
   OS
   Path at 'Path.ozf'
define
   class Installer

      meth install_from_package
	 DIR={OS.tmpnam}
      in
	 {self exec_mkdir(DIR)}
	 {self set_srcdir(DIR)}
	 {self set_builddir(DIR)}
	 {self set_extractdir(DIR)}
	 {self extract_package}
	 %% !!! NOT FINISHED !!!
      end

      meth install_all
   end
end
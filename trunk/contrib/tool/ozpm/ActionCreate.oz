functor
export
   Run
import
   Message(slurp:Slurp parse:Parse)
   FileUtils(fileTree      : FileTree
	     fullName      : FullName
	     expand        : Expand
	     addToPath     : AddToPath)
   System(showInfo:Print)
   Global(fileMftPkl     : FILEMFTPKL
	  fileMftTxt     : FILEMFTTXT
	  filePkgDft     : FILEPKGDFT
	  args             : Args)
   Pickle(save)
   Open(file)
   Archive(makeFrom)
   OS(unlink)
   Application(exit)
define   
   proc {Create In Out Inf}
      TXT = {Slurp Inf}
      O   = {Parse TXT}
      {O check_keys([id])}	% id is mandatory
      FT  = {FileTree if In=="" then "." else In end}
      Files
      local
	 L={Length {VirtualString.toString FT.path}}+1
	 fun {Loop R}
	    if R.type==dir then
	       {List.map R.entries Loop}
	    else
	       {String.toAtom {List.drop {VirtualString.toString R.path} L}}
	    end
	 end
      in
	 Files={List.subtract {List.subtract {List.flatten {Loop FT}} FILEMFTPKL} FILEMFTTXT}
	 {ForAll Files Print}
      end
      Info={Record.adjoinAt
	    {Record.map
	     {List.toRecord package {O entries($)}}
	     List.last}
	    filelist Files}
      F
      MFTPKL={Expand {FullName FILEMFTPKL In}}
      MFTTXT={Expand {FullName FILEMFTTXT In}}
   in
      try
	 {Pickle.save Info MFTPKL}
	 F={New Open.file init(name:MFTTXT flags:[write create truncate])}
	 try
	    {F write(vs:TXT)}
	 finally
	    try {F close} catch _ then skip end
	 end
	 {Archive.makeFrom Out FILEMFTPKL|FILEMFTTXT|Files In}
      finally
	 try {OS.unlink MFTPKL} catch _ then skip end
	 try {OS.unlink MFTTXT} catch _ then skip end
      end
   end
   %%
   proc {Run}
      {Create {CondSelect Args 'prefix' ""} Args.'out'
       {CondSelect Args 'in' {AddToPath {CondSelect Args 'prefix' "."} FILEPKGDFT}}}
      {Application.exit 0}
   end
end

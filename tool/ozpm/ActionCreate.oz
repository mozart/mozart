functor
export
   Run
import
   Message(slurp:Slurp parse:Parse)
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
   Path(make) at 'x-ozlib://duchier/sp/Path.ozf'
define   
   proc {Create In Out Inf}
      TXT = {Slurp Inf}
      O   = {Parse TXT}
      {O check_keys([id])}	% id is mandatory
      IN  = {{{{Path.make if In=="" then "." else In end}
	       expand($)} toBase($)} dirname($)}
      N   = {Length {IN toString($)}}+1
      Files =
      {Filter {Map {IN leaves($)}
	       fun {$ F}
		  {String.toAtom {List.drop {{Path.make F} toString($)} N}}
	       end}
       fun {$ F} F\=FILEMFTPKL andthen F\=FILEMFTTXT end}
      {ForAll Files Print}
      Info={Record.adjoinAt
	    {Record.map
	     {List.toRecord package {O entries($)}}
	     List.last}
	    filelist Files}
      F
      MFTPKL={{{IN resolve(FILEMFTPKL $)} expand($)} toString($)}
      MFTTXT={{{IN resolve(FILEMFTTXT $)} expand($)} toString($)}
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
       case {CondSelect Args 'in' unit}
       of unit then
	  {{{{Path.make {CondSelect Args 'prefix' "."}}
	      toBase($)} resolve(FILEPKGDFT $)} toString($)}
       [] P then P end}
      {Application.exit 0}
   end
end

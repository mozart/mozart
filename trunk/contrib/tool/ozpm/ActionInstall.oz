functor
export Run Install LoadPackage CalcActions DoInstall
import
   Resolve(localize)
   OS(unlink tmpnam)
   Archive('class')
   Global(fileMftPkl    : FILEMFTPKL
	  localDB       : LocalDB
	  packageMogulDB: MogulDB
	  args          : Args
	  dirPrefix     : DirPrefix)
   Application(exit)
   System(showInfo:Print show:Show)
   Pickle(load save)
   Browser(browse:Browse)
define
   
   fun{MergeInfoMogul Info}
      %%
      %% in  : a record containing informations about a package
      %% out : the record mixed with the informations for this package in the mogul DB
      %%
      {Record.adjoin
       {MogulDB condGet(Info.id r $)}
       Info}
   end
   
   proc{LoadPackage PackageRef ?Arch ?Info}
      %%
      %% in  : PackageRef is a reference to a package (URL or mogul name)
      %% out : Arch is bounded to the archive object
      %%       Info is bounded to a record containing the information inside the package
      %%
      PackageFile
   in
      try
	 PackageFile={Resolve.localize PackageRef}
      catch _ then skip end
      if {IsFree PackageFile} then
	 try
	    MogulData={MogulDB get({VirtualString.toAtom PackageRef} $)}
	    fun{Loop L}
	       case L of X|Xs then
		  V={VirtualString.toString X}
		  L={List.length V}
	       in
		  if (L>=4) andthen ({List.drop V L-4}==".pkg") then
		     V
		  else
		     {Loop Xs}
		  end
	       else
		  _
	       end
	    end
	    PackageURL={Loop MogulData.url_pkg}
	 in
	    PackageFile={Resolve.localize PackageURL}
	 end
      end
      if {IsFree PackageFile} then Arch=unit Info=notFound(package:PackageRef)
      else
	 Msg=if {Label PackageFile}==old then init(PackageFile.1) else initTemp(PackageFile.1) end
	 {New Archive.'class' Msg Arch}
	 Tmp={OS.tmpnam}
	 PInfo
	 try
	    {Arch extract(FILEMFTPKL Tmp)}
	    {Pickle.load Tmp PInfo}
	 finally
	    try {OS.unlink Tmp} catch _ then skip end
	 end
      in
	 Info={Record.adjoinAt
	       {MergeInfoMogul PInfo}
	       lsla {Arch lsla($)}}
      end
   end

   fun{Sort Set}
      {List.sort
       {List.map Set VirtualString.toAtom}
       Value.'<'}
   end
   

   %%
   %% This function calculates the action that have to be achieved
   %% in order to install the package and returns a record of the form :
   %% install(actions:L
   %%         file:F)
   %% where L is the list of actions to achieve, and F the result of resolving
   %% the package
   %%

   fun{CalcActions Info}
      %%
      %% Step A : calculate the dependencies of the package
      %%          and determine other packages that may have to be installed
      %%
      PackagesToInstall
      Requirements={Sort {CondSelect Info requires nil}}
      fun{Purge S}
	 Vs={VirtualString.toString S}
	 L={List.dropWhile Vs fun{$ C} C\=&\040 end}
      in
	 {VirtualString.toAtom
	  case L
	  of &\040|Xs then Xs
	  else Vs end}
      end
      ProvidedInstalled=if Requirements==nil then nil
			else
			   fun{ILoop L Xs}
			      case L of Y|Ys then
				 {Purge Y}|{ILoop Ys Xs}
			      else
				 {Loop Xs}
			      end
			   end
			   fun{Loop L}
			      case L of X|Xs then
				 {ILoop {CondSelect {MergeInfoMogul X} provides nil} Xs}
			      else
				 nil
			      end
			   end
			in
			   {Sort {Loop {Global.localDB items($)}}}
			end
      SubstractSet
      local
	 fun{Loop X Xs L}
	    case L of Y|Ys then
	       if Y==X then % element is found and must be removed from the subset
		  {SubstractSet Xs L}
	       elseif Y<X then % next element
		  {Loop X Xs Ys}
	       else % element was not found and must be kept
		  X|{SubstractSet Xs L}
	       end
	    else
	       X|nil
	    end
	 end
      in
	 fun{SubstractSet Sub1 Sub2}
	    %%
	    %% in : two sorted sets containing elements that can be compared with <
	    %% out : all elements in Sub1 that are not present in Sub2
	    %%
	    case Sub1
	    of X|Xs then
	       {Loop X Xs Sub2}
	    else
	       nil
	    end
	 end
      end
      local
	 fun{Loop Needed Given}
	    Remain={SubstractSet Needed Given}
	    {Show needed#Needed}
	    {Show given#Given}
	    {Show remain#Remain}
	 in
	    if Remain==nil then% and we are done....
	       nil
	    else
	       %%
	       %% new package to install
	       %%
	       Req={Purge Remain.1}
	       %% step 1 : find the package that provides Req
	       Package
	       local
		  fun{Provide Pkg Req}
		     {List.some {CondSelect Pkg provides nil}
		      fun{$ P}
			 {Purge P}==Req
		      end}
		  end
		  fun{Loop L}
		     case L of X|Xs then
			if {Provide X Req} then
			   %% we found the candidate
			   {Show found#X.id}
			   X
			else
			   {Loop Xs}
			end
		     else
			%% create a fake candidate
			notFound(id:{VirtualString.toAtom 'Unable to provide '#Req}
				 provides:[Req])
		     end
		  end
	       in
		  Package={Loop {MogulDB items($)}}
	       end
	       %% step 2 : compute a new "given" set
	       NewGiven={List.sort
			 {List.append
			  {List.map {CondSelect Package provides nil} Purge}
			  Given}
			 Value.'<'}
	       %% step 3 : compute a new "needed" set
	       NewNeeded={List.sort
			  {List.append
			   {List.map {CondSelect Package requiress nil} Purge}
			   Needed}
			  Value.'<'}
	    in
	       %% step 4 : loop with the new data
	       Package|{Loop NewNeeded NewGiven}
	    end
	 end
      in
	 PackagesToInstall={Loop Requirements ProvidedInstalled}
      end
      %%
      %%
      %% Step B : determine any conflicts that may exist with the current installation
      %%
      %%
      AlreadyInstalled=
      if {LocalDB member(Info.id $)} then
	 true
      else
	 false
      end
      ConflictsCell={NewCell nil}
      for Entry in {LocalDB items($)} do
	 for F in Entry.filelist do
	    for E in Info.filelist do
	       if E==F then
		  {Assign ConflictsCell used(name:E loc:Entry pkg:Info)|{Access ConflictsCell}}
	       end
	    end
	 end
      end
      Conflicts={Access ConflictsCell}
      Ret=install(package:Info
		  installfiles:Info.filelist
		  requires:PackagesToInstall
		  alreadyinstalled:AlreadyInstalled
		  conflicts:Conflicts)
   in
      Ret
   end

   proc{DoInstall Package InstallFiles Archive}
      for File in InstallFiles do
	 F = {DirPrefix resolve(File $)}
      in
	 {F mkdirs}
	 if {F isFile($)} then
	    {F rmtree}
	 end
	 {Archive extract(File {F toString($)})}
      end
      %% update ozpminfo
      local
	 ToSave={Record.adjoin
		 {Record.adjoinAt Package lsla {Archive lsla($)}}
		 ipackage}
      in
	 {LocalDB put(Package.id ToSave)}
	 {LocalDB save}
      end
      {Archive close}
   end
   
   fun{Install Package Force Leave Simulate}
      {InstallReq Package Force Leave Simulate nil}
   end

   fun{InstallReq Package Force Leave Simulate Req}
      Archive Info
   in
      {LoadPackage Package Archive Info}
      if {Label Info}==notFound then
	 notFound(package:Package)
      else
	 Actions={CalcActions Info}
      in
	 if Simulate then
	    {Archive close}
	    simulate(package:Actions.package
		     requires:Actions.requires
		     alreadyinstalled:Actions.alreadyinstalled
		     conflicts:Actions.conflicts)
	 elseif Force==false andthen Leave==false andthen Actions.conflicts\=nil then
	    %%
	    %% unable to proceed with the installation : conflicting filenames and
	    %% don't want to overwrite or keep the older files
	    {Archive close}
	    conflicts(package:Actions.package
		      conflicts:Actions.conflicts
		      alreadyinstalled:Actions.alreadyinstalled)
	 else
	    %%
	    %% being here means we are able to install the package
	    %%
	    %% install all required packages first
	    fun{Loop R}
	       if R==nil then
		  success
	       else
		  %% install the first package of the set
		  P=R.1
		  Ret={InstallReq P.id Force Leave Simulate Req}
	       in
		  case {Label Ret}
		  of success then
		     {Loop {List.drop R 1}}
		  else Ret
		  end
	       end
	    end
	    Ret={Loop {List.filter Actions.requires %% filter out already required packages
		       fun{$ R} {Not {List.member R Req}} end}}
	 in
	    if Ret\=success then
	       Ret
	    else
	       %% all required packages installed, proceed with the installation of this package
	       Info=Actions.package
	       FileList={NewCell Info.filelist}
	       Conflicts={NewCell nil}
	    in
	       if {Not Force} then
		  for Entry in {LocalDB items($)} do
		     for F in Entry.filelist do
			for E in Info.filelist do
			   if E==F then
			      if Leave then
				 {Assign FileList {Record.subtract {Access FileList} E}} %% don't install this file
			      else
				 {Assign Conflicts used(name:E loc:Entry pkg:Info)|{Access Conflicts}}
			      end
			   end
			end
		     end
		  end
	       end
	       if {Access Conflicts}\=nil then
		  {Archive close}
		  conflict(package:Info
			   conflicts:{Access Conflicts}
			   alreadyinstalled:false)
	       else
		  for File in {Access FileList} do
		     F = {DirPrefix resolve(File $)}
		  in
		     {F mkdirs}
		     if {F isFile($)} then
			{F rmtree}
		     end
		     {Archive extract(File {F toString($)})}
		  end
		  %% update ozpminfo
		  local
		     ToSave={Record.adjoin
			     {Record.adjoinAt Info lsla {Archive lsla($)}}
			     ipackage}
		  in
		     {LocalDB put(Info.id ToSave)}
		     {LocalDB save}
		  end
		  {Archive close}
		  success(package:Info)
	       end
	    end
	 end
      end
   end
   
   
%   fun {Install Package Force Leave}
%      PackageResult = {Resolve.localize Package}
%      A
%   in
%      try
%	 PackageFile=PackageResult.1
%	 {New Archive.'class' init(PackageFile) A}
%	 PLS={A lsla($)}
%	 PInfo Tmp={OS.tmpnam}
%	 FileList
%	 Conflicts={NewCell nil}
%      in
%	 try
%	    {A extract(FILEMFTPKL Tmp)}
%	    {Pickle.load Tmp PInfo}
%	 finally
%	    try {OS.unlink Tmp} catch _ then skip end
%	 end
%	 FileList={NewCell PInfo.filelist}
%	 %% now we have the information and file names of the package
%	 %%
%	 %% cross checks with the informations of the local installation is done here
%	 %%
%	 if {Not Force} then
%	    if {LocalDB member(PInfo.id $)} then
%	       raise ok(alreadyinstalled(loc:{LocalDB get(PInfo.id $)} pkg:PInfo)) end
%	    end
%	    for Entry in {LocalDB items($)} do
%	       for F in Entry.filelist do
%		  for E in PInfo.filelist do
%		     if E==F then
%			if Leave then
%			   {Assign FileList {Record.subtract {Access FileList} E}} %% don't install this file
%			else
%			   {Assign Conflicts used(name:E loc:Entry pkg:PInfo)|{Access Conflicts}}
%			end
%		     end
%		  end
%	       end
%	    end
%	    if {Access Conflicts}\=nil then
%	       raise ok(nameclash({Access Conflicts})) end
%	    end
%	 end
%	 %%
%	 %% getting this far means the package can be installed
%	 %%
%	 for File in {Access FileList} do
%	    F = {DirPrefix resolve(File $)}
%	 in
%	    {F mkdirs}
%	    if {F isFile($)} then
%	       {F rmtree}
%	    end
%	    {A extract(File {F toString($)})}
%	 end
%	 %% update ozpminfo
%	 local
%	    ToSave={Record.adjoin
%		    {Record.adjoinAt PInfo lsla PLS}
%		    ipackage}
%	 in
%	    {LocalDB put(PInfo.id ToSave)}
%	    {LocalDB save}
%	 end
%	 success(pkg:PInfo)
%      catch ok(E) then E
%      finally
%	 if {IsDet A} then try {A close} catch _ then skip end end
%	 case PackageResult of new(F) then {OS.unlink F}
%	 else skip end
%      end
%   end
   %%
   proc {Run}
      Ret={Install Args.'in' Args.'force' Args.'leave' Args.'simulate'}
   in
      case !Ret
      of simulate(package:Package
		  requires:PackagesToInstall
		  alreadyinstalled:AlreadyInstalled
		  conflicts:Conflicts) then
	 if PackagesToInstall\=nil then
	    {Print ""}
	    {Print "Installation of the package "#Package.id#" requires the following :"}
	    {ForAll {Reverse PackagesToInstall}
	     proc{$ P}
		{Print "Installation of required package "#P.id}
	     end}
	    {Print ""}
	    {Print "Warning : these packages may require the installation of further packages"}
	    {Print "Warning : no checks where made to see if this may raise conflicts"}
	 else skip end
	 if AlreadyInstalled then {Print ""} {Print "Warning : the package is already installed."} end
	 if Conflicts\=nil then
	    {Print ""}
	    {Print "Warning : installation of this package conflicts with the following files :"}
	    {ForAll Conflicts
	     proc{$ P} {Print P.name#" from the package "#P.pkg.id} end}
	 end
	 {Application.exit 0}
      [] conflicts(package:Package
		   conflicts:Conflicts
		   alreadyinstalled:AlreadyInstalled) then
	 {Print ""}
	 {Print "Unable to install package "#Package.id}
	 if AlreadyInstalled then {Print ""} {Print "Error : the package is already installed."} end
	 if Conflicts\=nil then
	    {Print ""}
	    {Print "Error : installation of this package conflicts with the following files :"}
	    {ForAll Conflicts
	     proc{$ P} {Print P.name#" from the package "#P.pkg.id} end}
	 end
	 {Print ""}
	 {Print "Use either the --force parameter to overwrite the older files"}
	 {Print "        or the --leave parameter to install the new package, keeping older files"}
	 {Application.exit 1}
      [] success(package:P) then
	 {Print "Package "#P.id#" was successfully installed"}
	 {Application.exit 0}
      end
   end
end

functor
import
   System(show:Show)
   OS
   Open
   GlobalStore(newLocal:NewLocal)
define   
   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   %%% Store emulator pid in a file
   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   F Pid
   try
      Pid={OS.getPID}
      F={New Open.file init(name:'/tmp/pidclient'#Pid flags:[write create])}      
      {F write(vs:Pid)}
      {F close}
   catch
   _
   then
   {Show 'ErrorCreateFile'}
   end
   %%%%%%%%%%%%%%%%%%%%%%%%%
   %% Initiate a local store
   %%%%%%%%%%%%%%%%%%%%%%%%%
   NewObj LocalStore
   try      
      {NewLocal "globalstore" user1  ?NewObj ?LocalStore}
   catch connectionfailed
   then
      {Show connectionfailed}
   end
   %%%%%%%%%%%%%%%%%%%%
   %% Set pool of hosts
   %%%%%%%%%%%%%%%%%%%%
   {LocalStore setpool(['potemkine.info.ucl.ac.be'
			'rainbow.info.ucl.ac.be' ])}
   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   %% Create objects in the store:
   %  Obj1 Obj2 Obj3 are references in
   %  the LocalStore 
   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%   
   class Counter
      attr val
      meth show(Value) Value=@val end
      meth inc(Value)
	 val <- @val+Value	
      end
      meth dec(Value)
	 val <- @val+Value 
      end
      meth init(Value) val <- Value end  
   end
   
   Obj1={NewObj  Counter init(0)}
   Obj2={NewObj  Counter  init(0)}
   Obj3={NewObj  Counter  init(0)}
   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   %% Save  Obj1 reference to use it on other clients
   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   {LocalStore saveobj(Obj1 '/usr/staff/ila/=Oz/store//o1globalref')}
   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   % Procedure to show a coherent
   % snapshot of the global store
   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   proc {CoherentSnapshot}
      C={NewCell 0}     
      proc{Trans Output}
	 Val1 Val2 Val3 in
	 {Obj1 show(Val1)} 
	 {Obj2 show(Val2)}
	 {Obj3 show(Val3)}
	 Output=state(Val1 Val2 Val3)
      end
      Trid
      Out
      Res
    in
     % State of objects Obj1 Obj2 is coherent here
     % show  state of  objects in a file after 1mn of period
      if {OS.time  $} - {Access C $}>=20
      then
	 {LocalStore trans(Trans Out ?Trid)}
	 {LocalStore checktrans(Trid Res _)}		 
	 if Res==commit  then
	    {Show 'Obj1:'#Out.1#'Obj2:'#Out.2#'Obj3:'#Out.3}
	 else
	    skip
	 end	 
	 {Assign C {OS.time  $}}
      else
	 skip
      end
   end   
   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   % Initialise transactions
   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   LastTransid
   fun {Iterate I}
      Transid  Retransid       
      proc{Trans Out}
	 proc {Iteratebis J}	 
	    if J\=0
	    then
	       {Obj1 inc(1)}
	       {Obj1 inc(~1)}
	       {Obj2 inc(~1)}       
	       {Obj2 inc(2)}
	       {Obj1 inc(1)}
	       {Iteratebis J-1}
	    else
	       skip
	    end
	 end
	 OutPut
      in
	 %{LocalStore waitlocks([Obj1 Obj2])}  
	 {Iteratebis 1}
	 %  {LocalStore retrans(
%              proc {$ Out}
%  	       {Obj1 inc(1)}
% 	       {Obj3 inc(1)}         
% 	     end
% 	       OutPut
%             ?Retransid)
% 	 }
      end
      Out
   in
      if I\=0
      then    
	 {LocalStore trans(Trans Out ?Transid)}
	 %{Delay 10}
	
	 if I==1 then LastTransid=Transid
	 else
	    skip
	 end
	 (Transid#Transid)|{Iterate I-1}
	 % (Transid#Retransid)|{Iterate I-1}
      else
	 nil
      end
   end
   Ccommit={NewCell 0} Cabort={NewCell 0}
   Crcommit={NewCell 0} Crabort={NewCell 0}
   Crecompute={NewCell 0} Crrecompute={NewCell 0}
   
   Transset={Iterate 2000}
   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   %% Wait for the end of  updates 
   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%  
   {Wait {LocalStore termination($)}} % For updates
   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   %% Compute number of committed/aborted transactions
   %% Compute number of committed/aborted retransactions
   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   
   {ForAll Transset 
    proc{$ Pair}
       Tr RTr Res Resb Nb Nbr in
       if Pair\=nil then
	  Pair=Tr#RTr
	  {LocalStore checktrans(Tr ?Res ?Nb)}
	  {Assign Crecompute  {Access Crecompute  $}+Nb}  
	  if  Res==commit
	  then
	     {Assign Ccommit {Access Ccommit  $}+1}
	  else
	     {Assign Cabort {Access Cabort $}+1}
	  end
	  if {Access Ccommit  $}+{Access Cabort  $}==5000 then
	     {Show 'Number of local committed transaction:'#{Access Ccommit  $}}
	     {Show 'Number of local aborted transaction:'#{Access Cabort  $}}
	     {Show 'Number of local recomputed transaction:'#{Access Crecompute  $}}
	  else
	     skip
 	  end
	  if {IsFree RTr} then
	     Resb=abort % Thread terminated before executing retrans
	     Nbr=0
	  else
	     {LocalStore checktrans(RTr Resb Nbr)}
	  end
	  {Assign Crrecompute  {Access Crrecompute  $}+Nbr} 
	  if Resb==commit
	  then
	     {Assign Crcommit {Access Crcommit  $}+1}
	  else
	     {Assign Crabort {Access Crabort $}+1}
	  end
	  if {Access Crcommit  $}+{Access Crabort  $}==5000 then
	     {Show 'Number of local aborted retrans:'#{Access Crabort  $}}
	     {Show 'Number of local committed retrans :'#{Access Crcommit  $}}
	     {Show 'Number of local recomputed retrans:'#{Access Crrecompute  $}}
	  else
	     skip
	  end
	  
       else
	  skip
       end
    end
   }
   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   % migrate the global store to a
   % specified host
   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   %{LocalStore migrate('rainbow.info.ucl.ac.be')}
   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   %% Show states of objects
   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%  
   {CoherentSnapshot}
      
end

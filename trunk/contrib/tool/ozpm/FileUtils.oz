functor
export
   Expand FileTree Mkdir Rmtree Exists
import
   URL(toVirtualStringExtended) Resolve(expand)
   OS(getDir stat system unlink)
   Shell(shellCommand isWindows:IsWindows rmdir)
define
   %%
   %% {Expand F}
   %%	expand a file name without loosing the weird chars
   %%
   fun {Encode F}
      case F
      of nil  then nil
      [] &#|T then &%|&2|&3|{Encode T}
      [] &{|T then &%|&7|&b|{Encode T}
      []  H|T then        H|{Encode T}
      end
   end
   fun {Expand F}
      {URL.toVirtualStringExtended
       {Resolve.expand {Encode {VirtualString.toString F}}}
       o(full:true raw:true)}
   end
   %%
   %% {FileTree +DIR ?R}
   %%	returns a record R representing the file structure root at DIR
   %% R has one of the following forms
   %%	stat(type:file size:N ...)
   %%	stat(type:dir  size:N entries:[R1 ... Rk] ...)
   %% where Ri is again of the same form
   %%
   fun {IsNotDot F}
      F\="." andthen F\=".."
   end
   fun {DirEntries F}
      {Map {Filter {OS.getDir F} IsNotDot}
       fun {$ FF}
	  {AdjoinAt {FileTree F#'/'#FF} name FF}
       end}
   end
   fun {FileTree F}
      F2 = {Expand F}
      R1 = try {OS.stat F2} catch _ then notFound(type:unknown size:0) end
      R2 = {AdjoinAt R1 path F2}
   in
      if R2.type==dir then
	 {AdjoinAt R2 entries {DirEntries F2}}
      else
	 R2
      end
   end
   %%
   %% {Exists F ?B}
   %%	determines if a file/dir exists
   %%
   fun {Exists F}
      F2 = {Expand F}
   in
      try {OS.stat F2 _} true catch _ then false end
   end
   %%
   %% {Mkdir F} {Rmdir F}
   %%	make or delete a directory
   %%
   proc {Mkdir F}
      F2 = {Expand F}
   in
      if {Exists F2} then
	 raise ozpm(mkdir F2 existsAlready) end
      elseif {Shell.shellCommand 'mkdir '#F2}\=0 then
	 raise ozpm(mkdir F2 commandFailed) end
      end
   end
   Rmtree
   if IsWindows then
      proc {RmLoop R}
	 case R.type
	 of dir then
	    for Ri in R.contents do {RmLoop Ri} end
	    if {Shell.rmdir R.path}\=0 then
	       raise ozpm(rmdir R.path commandFailed) end
	    end
	 else {OS.unlink R.path} end
      end
   in
      proc {Rmtree F}
	 {RmLoop {FileTree F}}
      end
   else
      proc {Rmtree F}
	 F2 = {Expand F}
      in
	 if {Shell.shellCommand 'rm -rf '#F2}\=0 then
	    raise ozpm(rmdir F2 commandFailed) end
	 end
      end
   end
end

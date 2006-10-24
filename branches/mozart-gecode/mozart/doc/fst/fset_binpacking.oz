declare
fun {SpreadFiles Files DiskCap}
   proc {$ Disks}
      FileSizes = {Map Files fun {$ F} F.size end}
      Size = {FoldL FileSizes Number.'+' 0}
      LB = Size div DiskCap +
           if Size mod DiskCap==0 then 0 else 1 end
      NbDisks    = {FD.int LB#FD.sup}
      AllFiles  = {List.number 1 {Length Files} 1}                
      Ds
   in
      {FD.distribute naive [NbDisks]}                

      {FS.var.list.upperBound NbDisks AllFiles Ds}

      {FS.partition Ds {FS.value.make AllFiles}}

      {ForAll Ds proc {$ D} BL in
		    {FS.reified.areIn AllFiles D BL}             
		    {FD.sumC FileSizes BL '=<:' DiskCap}       
		 end}
      
      {FS.distribute naive Ds}

      Disks = {Map Ds
	       fun {$ D}
		  Disk = {RecordC.tell diskette}
	       in
		  {ForAll {FS.monitorIn D}
		   proc {$ E}
		      F = {Nth Files E}
		   in
		      Disk^(F.name) = F.size
		   end}
		  {RecordC.width Disk} = {FS.card D}
		  Disk
	       end}
   end
end

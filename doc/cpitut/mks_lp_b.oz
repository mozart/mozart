declare
fun {DuplicateRIs Vs}
   {Map Vs
    fun {$ V}
       {RI.var.bounds
	{RI.getLowerBound V}
	{RI.getUpperBound V}}
    end}
end

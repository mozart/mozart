declare
Connect = {{New Module.manager init}
	   link(url: 'sync.so{native}' $)}.connect
{Wait Connect}
{Show Connect}

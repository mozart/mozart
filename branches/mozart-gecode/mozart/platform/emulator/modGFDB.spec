###$module_init_fun_name = "module_init_geintvar";

###$module_init_fun      = "oz_init_module";

###$module_name          = "GFD";

%builtins_all = 
(
 #
 # Built-ins
 #
 
 ## Variable declaration
	'int'		=> { in  => ['+value'],
			     	 out => ['int'],
					 bi  => new_intvar,
					 fcp => ignore},
 ## Variable type testing
    'is'		=> { in  => ['+value'],
			     	 out => ['bool'],
					 bi  => intvar_is,
                     fcp => ignore},
 ## misc. operations
	'inf'       => { in  => [],
	     			 out => ['int'],
					 bi  => int_inf},

    'sup'       => { in  => [],
	               	 out => ['int'],
					 bi  => int_sup},
 
 
 ## Variable reflection
    'reflect.min'		=> { in  => ['*int'],
					 out => ['+int'],
					 bi  => intvar_getMin},
    
    'reflect.max'	    => { in  => ['*int'],
			      	 out => ['+int'],
					 bi  => intvar_getMax},

    'reflect.size'      => { in  => ['*int'],
			  	     out => ['+int'],
					 bi  => intvar_getSize},
	
	'reflect.domList'   => { in  => ['+value'],
	                      out => ['+value'],
						  bi  => int_domList},
	'reflect.dom'       => { in  => ['+value'],
	                      out => ['+value'],
						  bi  => int_dom},
    'reflect.nextLarger'     => { in  => ['+value','int'],
	                      out => ['int'],
			      bi  => int_nextLarger},

    'reflect.nextSmaller'    => { in  => ['+value','int'],
	                      out => ['int'],
			      bi  => int_nextSmaller},

	'reflect.nbProp'    => { in  => ['+value'],
	                      out => ['int'],
			      bi  => intvar_propSusp},
			      
## Still missing:
## -mid
    'reflect.med' 		=> { in  => ['*int'],
	                 out => ['+int'],
					 bi  => intvar_getMed},

    'reflect.width'      => { in  => ['*int'],
	                  out => ['+int'],
					  bi  => intvar_getWidth},

    'reflect.regretMin'  => { in  => ['*int'],
	 		          out => ['+int'],
					  bi  => intvar_getRegretMin},

    'reflect.regretMax'  => { in  => ['*int'],
	                  out => ['+int'],
					  bi  => intvar_getRegretMax}

);

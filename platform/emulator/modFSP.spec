$module_init_fun_name = "fsp_init";

%builtins_all =
(
    'isIn'              => { in  => ['int','fset','bool'],
                             out => [],
                             bi  => fsp_isIn},

    'equalR'    => { in  => ['fset','fset','bool'],
                             out => [],
                             bi  => fsp_equalR},

    'isInR'             => { in  => ['int','fset','int'],
                             out => [],
                             bi  => fsp_isInR},

    'include'   => { in  => ['int','fset'],
                             out => [],
                             bi  => fsp_include},

    'exclude'   => { in  => ['int','fset'],
                             out => [],
                             bi  => fsp_exclude},

    'match'             => { in  => ['fset','+value'],
                             out => [],
                             bi  => fsp_match},

    'seq'               => { in  => ['+value'],
                             out => [],
                             bi  => fsp_seq},

    'minN'              => { in  => ['fset','+value'],
                             out => [],
                             bi  => fsp_minN},

    'maxN'              => { in  => ['fset','+value'],
                             out => [],
                             bi  => fsp_maxN},

    'card'              => { in  => ['fset','int'],
                             out => [],
                             bi  => fsp_card},

    'union'             => { in  => ['fset','fset','fset'],
                             out => [],
                             bi  => fsp_union},

    'intersection'      => { in  => ['fset','fset','fset'],
                             out => [],
                             bi  => fsp_intersection},

    'subsume'   => { in  => ['fset','fset'],
                             out => [],
                             bi  => fsp_subsume},

    'disjoint'  => { in  => ['fset','fset'],
                             out => [],
                             bi  => fsp_disjoint},

    'distinct'  => { in  => ['fset','fset'],
                             out => [],
                             bi  => fsp_distinct},

    'monitorIn' => { in  => ['fset','value'],
                             out => [],
                             bi  => fsp_monitorIn},

    'min'               => { in  => ['fset','int'],
                             out => [],
                             bi  => fsp_min},

    'max'               => { in  => ['fset','int'],
                             out => [],
                             bi  => fsp_max},

    'convex'    => { in  => ['fset'],
                             out => [],
                             bi  => fsp_convex},

    'diff'              => { in  => ['fset','fset','fset'],
                             out => [],
                             bi  => fsp_diff},

    'includeR'  => { in  => ['int','fset','int'],
                             out => [],
                             bi  => fsp_includeR},

    'bounds'    => { in  => ['+fset','fset','int','int','int'],
                             out => [],
                             bi  => fsp_bounds},

    'boundsN'   => { in  => ['+value','+value','+value',
                                     '+value','+value'],
                             out => [],
                             bi  => fsp_boundsN},

    'disjointN' => { in  => ['+value'],
                             out => [],
                             bi  => fsp_disjointN},

    'unionN'    => { in  => ['+value','fset'],
                             out => [],
                             bi  => fsp_unionN},

    'partition' => { in  => ['+value','fset'],
                             out => [],
                             bi  => fsp_partition},

    'partitionReified'=> { in  => ['+value','fset','+value'],
                             out => [],
                             bi  => fsp_partitionReified},

 );

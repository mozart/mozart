# -*-perl-*-

%builtins_all =
    (
     'is'       => { in  => ['+value'],
                     out => ['+bool'],
                     bi  => BIisObjectB},

     '@'        => { in  => ['value'],
                     out => ['value'],
                     bi  => BIat},

     '<-'       => { in  => ['value','value'],
                     out => [],
                     bi  => BIassign},

     'ooExch'   => { in  => ['value','value'],
                     out => ['value'],
                     bi  => BIexchange},

     'copyRecord'=> { in  => ['+record'],
                      out => ['+record'],
                      BI  => BIcopyRecord},

     'makeClass'=> { in  => ['+dictionary','+record','+record',
                             '+dictionary','+bool','+bool'],
                     out => ['+class'],
                     BI  => BImakeClass},

     ','        => { in  => ['+class','+record'],
                     out => [],
                     bi  => BIcomma},

     'send'     => { in  => ['+record','+class','+object'],
                     out => [],
                     bi  => BIsend},

     'ooGetLock'=> { in  => ['lock'],
                     out => [],
                     bi  => BIooGetLock},

     'newObject'=> { in  => ['+class'],
                     out => ['+object'],
                     bi  => BInewObject},

     'new'      => { in  => ['+class','+record','value'],
                     out => [],
                     bi  => BINew},
     );
1;;

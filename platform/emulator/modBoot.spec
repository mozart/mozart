# -*-perl-*-

%builtins_all =
    (
      'builtin' => { in  => ['+virtualString','+int'],
                     out => ['+procedure'],
                     BI  => BIbuiltin},


      'manager' => { in     => ['+virtualString'],
                     out    => ['+record'],
                     BI     => BIBootManager},
      );
1;;

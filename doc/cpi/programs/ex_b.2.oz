declare X Y Z in 
{Browse [X Y Z]}    % [X Y Z]

X :: [1 3 5 7 9]    % [X{1 3 5 7 9}
Y :: [1 3 5 7 9]    %  Y{1 3 5 7 9}
Z :: 0#10           %  Z{0#10}]

{FD_PROP.add X Y Z} % [X{1 3 5 7 9} Y{1 3 5 7 9}
                    %  Z{2 4 6 8 10}]
X = Y               % [Y{1 3 5} Y{1 3 5} Z{2 6 10}]


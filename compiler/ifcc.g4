grammar ifcc;

axiom : prog EOF ;

prog : 'int' 'main' '(' ')' '{' statement* '}' ;

statement : (return_stmt | declaration | assignment | declaration_assignement) ';' ;

return_stmt: RETURN expr ;

declaration_assignement : 'int' VAR '=' expr ; 
declaration : 'int' VAR ;
assignment : VAR '=' expr ;

expr : additive ;

additive 
    : multiplicative             # multiplicativeExprRef
    | additive '+' multiplicative # addition
    | additive '-' multiplicative # substraction
    ;

multiplicative 
    : unary                      # unaryExprRef
    | multiplicative '*' unary    # multiplication
    | multiplicative '/' unary    # division
    ;

unary 
    : '-' primitive              # unaryMinus
    | '+' primitive              # unaryPlus
    | primitive                  # primitiveExprRef
    ;

primitive 
    : '(' expr ')'               # parenthesis
    | VAR                        # variable
    | CONST                      # constant
    ;

RETURN : 'return' ;
CONST : '-'?[0-9]+ ;
VAR : [a-zA-Z_][a-zA-Z0-9_]* ;
COMMENT : '/*' .*? '*/' -> skip ;
DIRECTIVE : '#' .*? '\n' -> skip ;
WS    : [ \t\r\n] -> channel(HIDDEN);

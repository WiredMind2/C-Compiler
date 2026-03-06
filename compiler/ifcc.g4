grammar ifcc;

axiom : prog EOF ;

prog : statement* ;

statement : ((declaration | assignment | declaration_assignement | function_call ) ';') | function_definition | function_declaration ;

return_stmt: RETURN expr ';' ;

declaration_assignement : 'int' VAR '=' expr ; 
declaration : 'int' (VAR (',' VAR)*)? ;
assignment : VAR '=' expr ;

var_declarations_function : 'int' VAR ;

function_declaration : 'int' VAR '(' (var_declarations_function ',')* var_declarations_function? ')' ';' ;
function_definition  : 'int' VAR '(' (var_declarations_function ',')* var_declarations_function? ')' '{' statement* return_stmt? '}' ;

expr : bitwiseOR ;

bitwiseOR : bitwiseXOR         # bitwiseORRef
    | bitwiseOR '^' bitwiseXOR # bitwiseORRule
    ;

bitwiseXOR : bitwiseAND         # bitwiseXORRef
    | bitwiseXOR '^' bitwiseAND # bitwiseXORRule
    ;

bitwiseAND : equality         # bitwiseANDRef
    | bitwiseAND '&' additive # bitwiseANDRule
    ;

equality : additive          # equalityExprRef
    | equality '==' additive # equals
    | equality '!=' additive # different
    ;

additive 
    : multiplicative             # multiplicativeExprRef
    | additive '+' multiplicative # addition
    | additive '-' multiplicative # substraction
    ;

multiplicative 
    : unary                      # unaryExprRef
    | multiplicative '*' unary    # multiplication
    | multiplicative '/' unary    # division
    | multiplicative '%' unary    # modulo
    ;

unary 
    : '-' primitive              # unaryMinus
    | '+' primitive              # unaryPlus
    | '!' primitive              # unaryNot
    | primitive                  # primitiveExprRef
    ;

primitive 
    : '(' expr ')'               # parenthesis
    | function_call              # functionCall
    | VAR                        # variable
    | CONST                      # constant
    ;


function_call : VAR '(' (expr (',' expr)*)? ')' ;
RETURN : 'return' ;
CONST : '-'?[0-9]+ ;
VAR : [a-zA-Z_][a-zA-Z0-9_]* ;
COMMENT : '/*' .*? '*/' -> skip ;
DIRECTIVE : '#' .*? '\n' -> skip ;
WS    : [ \t\r\n] -> channel(HIDDEN);

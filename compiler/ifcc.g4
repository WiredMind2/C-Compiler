grammar ifcc;

axiom : prog EOF ;

prog : statement* ;

statement : ((expr | return_stmt ) ';') | scope | function_definition | function_declaration | condition | while_loop | for_loop | var_decl | declaration_list | var_decl_with_init ;

return_stmt: RETURN expr ;

var_decl : 'int' VAR ';' ;
declaration_list : 'int' VAR (',' VAR)* ';' ;
var_decl_with_init : 'int' VAR '=' expr ';' ;


param : 'int' VAR ;
param_list : param (',' param)* ;

function_declaration : 'int' VAR '(' param_list? ')' ';' ;
function_definition  : 'int' VAR '(' param_list? ')' scope;
condition : 'if' '(' expr ')' scope ('else' (scope | condition) )? ;
while_loop : 'while' '(' expr ')' scope ;
for_loop: 'for' '(' expr? ';' expr? ';' expr? ')' scope ;

scope : '{' statement* '}' ;

expr : sequential ;

sequential : compoundAssignment # sequentialExprRef
    | compoundAssignment ',' sequential # sequentialRule
    ;

compoundAssignment : logicalOR # compoundAssignmentRef
    | VAR '=' compoundAssignment # Assignment
    ;

logicalOR : logicalAND # logicalORRef
    | logicalOR '||' logicalAND # logicalORRule
    ;

logicalAND : bitwiseOR # logicalANDRef
    | logicalAND '&&' bitwiseOR # logicalANDRule
    ;

bitwiseOR : bitwiseXOR         # bitwiseORRef
    | bitwiseOR '^' bitwiseXOR # bitwiseORRule
    ;

bitwiseXOR : bitwiseAND         # bitwiseXORRef
    | bitwiseXOR '^' bitwiseAND # bitwiseXORRule
    ;

bitwiseAND : equality         # bitwiseANDRef
    | bitwiseAND '&' additive # bitwiseANDRule
    ;

equality : relational          # equalityExprRef
    | equality '==' relational # equals
    | equality '!=' relational # different
    ;

relational : additive # relationalExprRef
    | relational '<' additive # smallerStrictThan
    | relational '>' additive # greaterStrictThan
    | relational '<=' additive # smallerThan
    | relational '>=' additive # greaterThan
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

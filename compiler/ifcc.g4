grammar ifcc;

axiom : prog EOF ;

prog : statement* ;

statement : ((expr | return_stmt ) ';')
    | scope
    | function_definition
    | function_declaration
    | condition
    | while_loop
    | for_loop
    | break_stmt
    | continue_stmt
    | declaration
    ;

return_stmt: RETURN expr ;

type_specifier : 'int' | 'double' ;

declaration : type_specifier? declaration_instance (',' declaration_instance)* ';' ;
declaration_instance : VAR ('=' expr)? ;

param : 'int' VAR ;
param_list : param (',' param)* ;

function_declaration : 'int' VAR '(' param_list? ')' ';' ;
function_definition  : 'int' VAR '(' param_list? ')' scope;
condition : 'if' '(' expr ')' scope ('else' else_block )? ;
else_block : scope | condition ;
while_loop : 'while' '(' expr ')' scope ;
for_loop: 'for' '(' expr? ';' expr? ';' expr? ')' scope ;
break_stmt : 'break' ';' ;
continue_stmt : 'continue' ';' ;

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
    | DOUBLE_CONST               # double_constant
    ;


function_call : VAR '(' (expr (',' expr)*)? ')' ;
RETURN : 'return' ;
CONST : '-'?[0-9]+ ;
DOUBLE_CONST : [0-9]+ '.' [0-9]* | [0-9]* '.' [0-9]+ ;
VAR : [a-zA-Z_][a-zA-Z0-9_]* ;
COMMENT : '/*' .*? '*/' -> skip ;
LINE_COMMENT : '//' ~[\r\n]* -> skip ;
DIRECTIVE : '#' .*? '\n' -> skip ;
WS    : [ \t\r\n] -> channel(HIDDEN);

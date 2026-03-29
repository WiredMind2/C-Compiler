grammar ifcc;

axiom : prog EOF ;

prog : statement* ;

statement : ((expr | return_stmt ) ';') | scope | function_definition | function_declaration | condition | while_loop | for_loop | switch_stmt | break_stmt | continue_stmt | var_decl_list | var_decl_with_init ;

return_stmt: RETURN expr? ;

type_specifier : 'void' | 'int' | 'double' | 'char' ;

declarator : '*'* VAR ('[' constant ']')? ;
var_decl_list : type_specifier declarator (',' declarator)* ';' ;
var_decl_with_init : type_specifier declarator '=' expr ';' ;

param : type_specifier VAR ;
param_list : param (',' param)* ;

function_declaration : type_specifier VAR '(' param_list? ')' ';' ;
function_definition  : type_specifier VAR '(' param_list? ')' scope;
condition : 'if' '(' expr ')' statement ('else' else_block )? ;
else_block : scope | condition ;
while_loop : 'while' '(' expr ')' scope ;
for_loop: 'for' '(' expr? ';' expr? ';' expr? ')' scope ;
switch_stmt : 'switch' '(' expr ')' '{' case_block* default_block? '}' ;
case_block : 'case' expr ':' (statement* | scope) ;
default_block : 'default' ':' (statement* | scope) ;
break_stmt : 'break' ';' ;
continue_stmt : 'continue' ';' ;

scope : '{' statement* '}' ;

lvalue: primitive ('[' expr ']')? | '*' lvalue ; 

expr : sequential ;

sequential : compoundAssignment # sequentialExprRef
    | compoundAssignment ',' sequential # sequentialRule
    ;

compoundAssignment : logicalOR # compoundAssignmentRef
    | lvalue '=' compoundAssignment # Assignment
    | lvalue '+=' compoundAssignment # AddAssignment
    | lvalue '-=' compoundAssignment # SubAssignment
    ;

logicalOR : logicalAND # logicalORRef
    | logicalOR '||' logicalAND # logicalORRule
    ;

logicalAND : bitwiseOR # logicalANDRef
    | logicalAND '&&' bitwiseOR # logicalANDRule
    ;

bitwiseOR : bitwiseXOR         # bitwiseORRef
    | bitwiseOR '|' bitwiseXOR # bitwiseORRule
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

relational : shift # relationalExprRef
    | relational '<' shift # smallerStrictThan
    | relational '>' shift # greaterStrictThan
    | relational '<=' shift # smallerThan
    | relational '>=' shift # greaterThan
    ;

shift
    : additive                 # shiftExprRef
    | shift '<<' additive      # shiftLeft
    | shift '>>' additive      # shiftRight
    ;

additive
    : multiplicative              # multiplicativeExprRef
    | additive '+' multiplicative # addition
    | additive '-' multiplicative # substraction
    ;

multiplicative
    : unary                       # unaryExprRef
    | multiplicative '*' unary    # multiplication
    | multiplicative '/' unary    # division
    | multiplicative '%' unary    # modulo
    ;

unary
    : '++' lvalue                # preIncrement
    | '--' lvalue                # preDecrement
    | lvalue '++'                # postIncrement
    | lvalue '--'                # postDecrement
    | '-' unary                  # unaryMinus
    | '+' unary                  # unaryPlus
    | '!' unary                  # unaryNot
    | '~' unary                  # unaryBitNot
    | '*' unary                  # dereference
    | '&' unary                  # addressOf
    | primitive                  # primitiveExprRef
    ;

primitive
    : '(' expr ')'               # parenthesis
    | function_call              # functionCall
    | primitive '[' expr ']'     # array_subscript
    | VAR                        # variable
    | constant                   # constantLiteral
    ;

constant
    : DEC_CONST            # decimalConstant
    | HEX_CONST        # hexConstant
    | DOUBLE_CONST     # doubleConstant
    | CHAR_CONST       # charConstant
    ;


function_call : VAR '(' (expr (',' expr)*)? ')' ;
RETURN : 'return' ;
HEX_CONST : '0' [xX] [0-9a-fA-F]+ ;
DEC_CONST : [0-9]+ ;
DOUBLE_CONST : [0-9]+ '.' [0-9]* | [0-9]* '.' [0-9]+ ;
CHAR_CONST : '\'' (~['\\] | '\\' .) '\'' ;
VAR : [a-zA-Z_][a-zA-Z0-9_]* ;
COMMENT : '/*' .*? '*/' -> skip ;
LINE_COMMENT : '//' ~[\r\n]* -> skip ;
DIRECTIVE : '#' .*? '\n' -> skip ;
WS    : [ \t\r\n] -> channel(HIDDEN);

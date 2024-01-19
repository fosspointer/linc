# LINC: Grammar

- **Program →** Declaration*
- **Statement →** ExpressionStatement | DeclarationStatement | ScopeStatement | PutCharacterStatement | PutStringStatement
- **Declaration →** VariableDeclaration | FunctionDeclaration | ArgumentDeclaration
- **Expression →** PrimaryExpression | UnaryExpression | BinaryExpression | '(' Expression ')'
- **ExpressionStatement →** Expression
- **DeclarationStatement →** Declaration
- **VariableDeclaration →** ('mut') Identifier Identifier '=' Expression
- **ArgumentDeclaration →** ('mut') Identifier Identifier ('=' Expression)
- **FunctionDeclaration →** 'fn' Identifier '(' ArgumentDeclaration* ')' ':' Identifier Statement 
- **ScopeStatement →** '{' Statement* '}'
- **PutCharacterStatement →** 'putc' '(' Expression ')'
- **PutStringStatement →** 'puts' '(' Expression ')'
- **PrimaryExpression →** Identifier | Value | IfElseExpression | WhileExpression | AssignmentExpression | FunctionCallExpression 
- **IfElseExpression →** 'if' Expression Statement ('else' Statement)
- **WhileExpression →** 'while' Expression Statement ('finally' Statement) ('else Statement)
- **AssignmentExpression →** Identifier '=' Expression
- **FunctionCallExpression →** Identifier '(' Expression* ')'
- **UnaryExpression →** UnaryOperator Expression
- **BinaryExpression →** Expression BinaryOperator Expression
- **UnaryOperator →** '+' | '-' | '!' | '@'
- **BinaryOperator →** '+' | '-' | '*' | '/' | '==' | '!=' | '&&' | '||' | '>' | '<' | '>=' | '<='
- **Value →** {{Integer | Floating} LiteralSuffix} | BooleanLiteral | CharacterLiteral | StringLiteral
- **LiteralSuffix →**  '' | 'u8' | 'u16' | 'u32' | 'u64' | 'u' | 'i8' | 'i16' | 'i32' | 'i64' | 'i' | 'f32' | 'f64' | 'f' | 'c' | 'b'
- **Identifier →** Letter | '_' (Letter | Digit | '_')*
- **CharacterLiteral →** ''' . .* '''
- **StringLiteral →** '"' .* '"'
- **BooleanLiteral →** true | false | {Integer LiteralSuffix}
- **Integer →** ('-') Digit+
- **Floating →** {('-') Digit+ '.' Digit*} | {('-') Digit* '.' Digit+ } | Integer
- **Letter →** 'a' | ... | 'z' | 'A' | ... | 'Z'
- **Digit →** '0' | ... | '9'

## Notes

- All binary expressions are currently left-to-right associative, something to change in the future for obvious reasons.
- `()` denotes optionality of the contained symbol.
- `{}` denotes simple grouping of an arbitrary number of contained symbols.
- `*` denotes zero or more of the preceding symbol.
- `.` denotes any character (similar to its regular expression counterpart).
- `→` is used as is in formal language theory.
- `|` functions as the logical OR operator.
- `''` denotes sequences of characters representing terminal symbols.
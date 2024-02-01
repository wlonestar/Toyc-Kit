%token	IDENTIFIER I_CONSTANT F_CONSTANT STRING_LITERAL FUNC_NAME SIZEOF
%token	PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token	AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token	SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token	XOR_ASSIGN OR_ASSIGN
%token	TYPEDEF_NAME ENUMERATION_CONSTANT

%token	TYPEDEF EXTERN STATIC AUTO REGISTER INLINE
%token	CONST RESTRICT VOLATILE
%token	BOOL CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE VOID
%token	COMPLEX IMAGINARY 
%token	STRUCT UNION ENUM ELLIPSIS

%token	CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

%token	ALIGNAS ALIGNOF ATOMIC GENERIC NORETURN STATIC_ASSERT THREAD_LOCAL

%start TranslationUnit
%%

PrimaryExpression
	: IDENTIFIER
	| Constant
	| String
	| '(' Expression ')'
	| GenericSelection
	;

Constant
	: I_CONSTANT		/* includes character_Constant */
	| F_CONSTANT
	| ENUMERATION_CONSTANT	/* after it has been defined as such */
	;

EnumerationConstant		/* before it has been defined as such */
	: IDENTIFIER
	;

String
	: STRING_LITERAL
	| FUNC_NAME
	;

GenericSelection
	: GENERIC '(' AssignmentExpression ',' GenericAssocList ')'
	;

GenericAssocList
	: GenericAssociation
	| GenericAssocList ',' GenericAssociation
	;

GenericAssociation
	: TypeName ':' AssignmentExpression
	| DEFAULT ':' AssignmentExpression
	;

PostfixExpression
	: PrimaryExpression
	| PostfixExpression '[' Expression ']'
	| PostfixExpression '(' ')'
	| PostfixExpression '(' ArgumentExpressionList ')'
	| PostfixExpression '.' IDENTIFIER
	| PostfixExpression PTR_OP IDENTIFIER
	| PostfixExpression INC_OP
	| PostfixExpression DEC_OP
	| '(' TypeName ')' '{' InitializerList '}'
	| '(' TypeName ')' '{' InitializerList ',' '}'
	;

ArgumentExpressionList
	: AssignmentExpression
	| ArgumentExpressionList ',' AssignmentExpression
	;

UnaryExpression
	: PostfixExpression
	| INC_OP UnaryExpression
	| DEC_OP UnaryExpression
	| UnaryOperator CastExpression
	| SIZEOF UnaryExpression
	| SIZEOF '(' TypeName ')'
	| ALIGNOF '(' TypeName ')'
	;

UnaryOperator
	: '&'
	| '*'
	| '+'
	| '-'
	| '~'
	| '!'
	;

CastExpression
	: UnaryExpression
	| '(' TypeName ')' CastExpression
	;

MultiplicativeExpression
	: CastExpression
	| MultiplicativeExpression '*' CastExpression
	| MultiplicativeExpression '/' CastExpression
	| MultiplicativeExpression '%' CastExpression
	;

AdditiveExpression
	: MultiplicativeExpression
	| AdditiveExpression '+' MultiplicativeExpression
	| AdditiveExpression '-' MultiplicativeExpression
	;

ShiftExpression
	: AdditiveExpression
	| ShiftExpression LEFT_OP AdditiveExpression
	| ShiftExpression RIGHT_OP AdditiveExpression
	;

RelationalExpression
	: ShiftExpression
	| RelationalExpression '<' ShiftExpression
	| RelationalExpression '>' ShiftExpression
	| RelationalExpression LE_OP ShiftExpression
	| RelationalExpression GE_OP ShiftExpression
	;

EqualityExpression
	: RelationalExpression
	| EqualityExpression EQ_OP RelationalExpression
	| EqualityExpression NE_OP RelationalExpression
	;

AndExpression
	: EqualityExpression
	| AndExpression '&' EqualityExpression
	;

ExclusiveOrExpression
	: AndExpression
	| ExclusiveOrExpression '^' AndExpression
	;

InclusiveOrExpression
	: ExclusiveOrExpression
	| InclusiveOrExpression '|' ExclusiveOrExpression
	;

LogicalAndExpression
	: InclusiveOrExpression
	| LogicalAndExpression AND_OP InclusiveOrExpression
	;

LogicalOrExpression
	: LogicalAndExpression
	| LogicalOrExpression OR_OP LogicalAndExpression
	;

ConditionalExpression
	: LogicalOrExpression
	| LogicalOrExpression '?' Expression ':' ConditionalExpression
	;

AssignmentExpression
	: ConditionalExpression
	| UnaryExpression AssignmentOperator AssignmentExpression
	;

AssignmentOperator
	: '='
	| MUL_ASSIGN
	| DIV_ASSIGN
	| MOD_ASSIGN
	| ADD_ASSIGN
	| SUB_ASSIGN
	| LEFT_ASSIGN
	| RIGHT_ASSIGN
	| AND_ASSIGN
	| XOR_ASSIGN
	| OR_ASSIGN
	;

Expression
	: AssignmentExpression
	| Expression ',' AssignmentExpression
	;

ConstantExpression
	: ConditionalExpression	/* with constraints */
	;

Declaration
	: DeclarationSpecifiers ';'
	| DeclarationSpecifiers InitDeclaratorList ';'
	| StaticAssertDeclaration
	;

DeclarationSpecifiers
	: StorageClassSpecifier DeclarationSpecifiers
	| StorageClassSpecifier
	| TypeSpecifier DeclarationSpecifiers
	| TypeSpecifier
	| TypeQualifier DeclarationSpecifiers
	| TypeQualifier
	| FunctionSpecifier DeclarationSpecifiers
	| FunctionSpecifier
	| AlignmentSpecifier DeclarationSpecifiers
	| AlignmentSpecifier
	;

InitDeclaratorList
	: InitDeclarator
	| InitDeclaratorList ',' InitDeclarator
	;

InitDeclarator
	: Declarator '=' Initializer
	| Declarator
	;

StorageClassSpecifier
	: TYPEDEF	/* Identifiers must be flagged as TYPEDEF_NAME */
	| EXTERN
	| STATIC
	| THREAD_LOCAL
	| AUTO
	| REGISTER
	;

TypeSpecifier
	: VOID
	| CHAR
	| SHORT
	| INT
	| LONG
	| FLOAT
	| DOUBLE
	| SIGNED
	| UNSIGNED
	| BOOL
	| COMPLEX
	| IMAGINARY	  	/* non-mandated extension */
	| AtomicTypeSpecifier
	| StructOrUnionSpecifier
	| EnumSpecifier
	| TYPEDEF_NAME		/* after it has been defined as such */
	;

StructOrUnionSpecifier
	: StructOrUnion '{' StructDeclarationList '}'
	| StructOrUnion IDENTIFIER '{' StructDeclarationList '}'
	| StructOrUnion IDENTIFIER
	;

StructOrUnion
	: STRUCT
	| UNION
	;

StructDeclarationList
	: StructDeclaration
	| StructDeclarationList StructDeclaration
	;

StructDeclaration
	: SpecifierQualifierList ';'	/* for anonymous struct/union */
	| SpecifierQualifierList StructDeclaratorList ';'
	| StaticAssertDeclaration
	;

SpecifierQualifierList
	: TypeSpecifier SpecifierQualifierList
	| TypeSpecifier
	| TypeQualifier SpecifierQualifierList
	| TypeQualifier
	;

StructDeclaratorList
	: StructDeclarator
	| StructDeclaratorList ',' StructDeclarator
	;

StructDeclarator
	: ':' ConstantExpression
	| Declarator ':' ConstantExpression
	| Declarator
	;

EnumSpecifier
	: ENUM '{' EnumeratorList '}'
	| ENUM '{' EnumeratorList ',' '}'
	| ENUM IDENTIFIER '{' EnumeratorList '}'
	| ENUM IDENTIFIER '{' EnumeratorList ',' '}'
	| ENUM IDENTIFIER
	;

EnumeratorList
	: Enumerator
	| EnumeratorList ',' Enumerator
	;

Enumerator	/* Identifiers must be flagged as ENUMERATION_CONSTANT */
	: EnumerationConstant '=' ConstantExpression
	| EnumerationConstant
	;

AtomicTypeSpecifier
	: ATOMIC '(' TypeName ')'
	;

TypeQualifier
	: CONST
	| RESTRICT
	| VOLATILE
	| ATOMIC
	;

FunctionSpecifier
	: INLINE
	| NORETURN
	;

AlignmentSpecifier
	: ALIGNAS '(' TypeName ')'
	| ALIGNAS '(' ConstantExpression ')'
	;

Declarator
	: Pointer DirectDeclarator
	| DirectDeclarator
	;

DirectDeclarator
	: IDENTIFIER
	| '(' Declarator ')'
	| DirectDeclarator '[' ']'
	| DirectDeclarator '[' '*' ']'
	| DirectDeclarator '[' STATIC TypeQualifierList AssignmentExpression ']'
	| DirectDeclarator '[' STATIC AssignmentExpression ']'
	| DirectDeclarator '[' TypeQualifierList '*' ']'
	| DirectDeclarator '[' TypeQualifierList STATIC AssignmentExpression ']'
	| DirectDeclarator '[' TypeQualifierList AssignmentExpression ']'
	| DirectDeclarator '[' TypeQualifierList ']'
	| DirectDeclarator '[' AssignmentExpression ']'
	| DirectDeclarator '(' ParameterTypeList ')'
	| DirectDeclarator '(' ')'
	| DirectDeclarator '(' IdentifierList ')'
	;

Pointer
	: '*' TypeQualifierList Pointer
	| '*' TypeQualifierList
	| '*' Pointer
	| '*'
	;

TypeQualifierList
	: TypeQualifier
	| TypeQualifierList TypeQualifier
	;


ParameterTypeList
	: ParameterList ',' ELLIPSIS
	| ParameterList
	;

ParameterList
	: ParameterDeclaration
	| ParameterList ',' ParameterDeclaration
	;

ParameterDeclaration
	: DeclarationSpecifiers Declarator
	| DeclarationSpecifiers AbstractDeclarator
	| DeclarationSpecifiers
	;

IdentifierList
	: IDENTIFIER
	| IdentifierList ',' IDENTIFIER
	;

TypeName
	: SpecifierQualifierList AbstractDeclarator
	| SpecifierQualifierList
	;

AbstractDeclarator
	: Pointer DirectAbstractDeclarator
	| Pointer
	| DirectAbstractDeclarator
	;

DirectAbstractDeclarator
	: '(' AbstractDeclarator ')'
	| '[' ']'
	| '[' '*' ']'
	| '[' STATIC TypeQualifierList AssignmentExpression ']'
	| '[' STATIC AssignmentExpression ']'
	| '[' TypeQualifierList STATIC AssignmentExpression ']'
	| '[' TypeQualifierList AssignmentExpression ']'
	| '[' TypeQualifierList ']'
	| '[' AssignmentExpression ']'
	| DirectAbstractDeclarator '[' ']'
	| DirectAbstractDeclarator '[' '*' ']'
	| DirectAbstractDeclarator '[' STATIC TypeQualifierList AssignmentExpression ']'
	| DirectAbstractDeclarator '[' STATIC AssignmentExpression ']'
	| DirectAbstractDeclarator '[' TypeQualifierList AssignmentExpression ']'
	| DirectAbstractDeclarator '[' TypeQualifierList STATIC AssignmentExpression ']'
	| DirectAbstractDeclarator '[' TypeQualifierList ']'
	| DirectAbstractDeclarator '[' AssignmentExpression ']'
	| '(' ')'
	| '(' ParameterTypeList ')'
	| DirectAbstractDeclarator '(' ')'
	| DirectAbstractDeclarator '(' ParameterTypeList ')'
	;

Initializer
	: '{' InitializerList '}'
	| '{' InitializerList ',' '}'
	| AssignmentExpression
	;

InitializerList
	: Designation Initializer
	| Initializer
	| InitializerList ',' Designation Initializer
	| InitializerList ',' Initializer
	;

Designation
	: DesignatorList '='
	;

DesignatorList
	: Designator
	| DesignatorList Designator
	;

Designator
	: '[' ConstantExpression ']'
	| '.' IDENTIFIER
	;

StaticAssertDeclaration
	: STATIC_ASSERT '(' ConstantExpression ',' STRING_LITERAL ')' ';'
	;

Statement
	: LabeledStatement
	| CompoundStatement
	| ExpressionStatement
	| SelectionStatement
	| IterationStatement
	| JumpStatement
	;

LabeledStatement
	: IDENTIFIER ':' Statement
	| CASE ConstantExpression ':' Statement
	| DEFAULT ':' Statement
	;

CompoundStatement
	: '{' '}'
	| '{'  BlockItemList '}'
	;

BlockItemList
	: BlockItem
	| BlockItemList BlockItem
	;

BlockItem
	: Declaration
	| Statement
	;

ExpressionStatement
	: ';'
	| Expression ';'
	;

SelectionStatement
	: IF '(' Expression ')' Statement ELSE Statement
	| IF '(' Expression ')' Statement
	| SWITCH '(' Expression ')' Statement
	;

IterationStatement
	: WHILE '(' Expression ')' Statement
	| DO Statement WHILE '(' Expression ')' ';'
	| FOR '(' ExpressionStatement ExpressionStatement ')' Statement
	| FOR '(' ExpressionStatement ExpressionStatement Expression ')' Statement
	| FOR '(' Declaration ExpressionStatement ')' Statement
	| FOR '(' Declaration ExpressionStatement Expression ')' Statement
	;

JumpStatement
	: GOTO IDENTIFIER ';'
	| CONTINUE ';'
	| BREAK ';'
	| RETURN ';'
	| RETURN Expression ';'
	;

TranslationUnit
	: ExternalDeclaration
	| TranslationUnit ExternalDeclaration
	;

ExternalDeclaration
	: FunctionDefinition
	| Declaration
	;

FunctionDefinition
	: DeclarationSpecifiers Declarator DeclarationList CompoundStatement
	| DeclarationSpecifiers Declarator CompoundStatement
	;

DeclarationList
	: Declaration
	| DeclarationList Declaration
	;

%%

#include <stdio.h>
#include "lex.yy.c"

int main(int argc, char **argv) {
  if (argc <= 1) {
    return 1;
  }
  FILE *f = fopen(argv[1], "r");
  if (!f) {
    perror(argv[1]);
    return 1;
  }
  yyrestart(f);
  yyparse();
  return 0;
}

void yyerror(const char *s) {
	fflush(stdout);
	fprintf(stderr, "*** %s\n", s);
}

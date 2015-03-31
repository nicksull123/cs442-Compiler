/* Semantics.h
   The action and supporting routines for performing semantics processing.
*/

/* Semantic Defines */
#define T_BOOL 0
#define T_INT 1

#define B_LT 0
#define B_LTE 1
#define B_GT 2
#define B_GTE 3
#define B_NE 4
#define B_EQ 5

#define B_TRUE 1
#define B_FALSE 0

/* Semantic Records */
struct IdList
{
    struct SymEntry* TheEntry;
    struct IdList* Next;
};

struct ExprRes
{
    int Reg;
    int Type;
    struct InstrSeq* Instrs;
};

struct ExprResList
{
    struct ExprRes* Expr;
    struct ExprResList* Next;
};

/* Semantics Actions */
void doDeclare(char *name, int type);
struct ExprRes* doIntLit( char* digits );
struct ExprRes *doBoolLit( int b );
struct ExprRes* doRval( char* name );
struct InstrSeq* doAssign( char* name, struct ExprRes *Expr );
struct ExprRes *doPow( struct ExprRes *base, struct ExprRes *pow );
struct ExprRes *doNegate( struct ExprRes *Expr );
struct ExprRes* doArith( struct ExprRes* Res1, struct ExprRes* Res2, char op );
struct InstrSeq* doPrint( struct ExprRes* Expr );
struct ExprRes* doBExpr( struct ExprRes* Res1, struct ExprRes* Res2, int op );

void Finish( struct InstrSeq* Code );

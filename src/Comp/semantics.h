/* Semantics.h
   The action and supporting routines for performing semantics processing.
*/

/* Semantic Defines */
#define B_LT 0
#define B_LTE 1
#define B_GT 2
#define B_GTE 3
#define B_NE 4
#define B_EQ 5

/* Semantic Records */
struct IdList
{
    struct SymEntry* TheEntry;
    struct IdList* Next;
};

struct ExprRes
{
    int Reg;
    struct InstrSeq* Instrs;
};

struct ExprResList
{
    struct ExprRes* Expr;
    struct ExprResList* Next;
};

struct BExprRes
{
    int Reg;
    struct InstrSeq* Instrs;
};

/* Semantics Actions */
struct ExprRes* doIntLit( char* digits );
struct ExprRes* doRval( char* name );
struct InstrSeq* doAssign( char* name, struct ExprRes* Res1 );
struct ExprRes *doPow( struct ExprRes *base, struct ExprRes *pow );
struct ExprRes *doNegate( struct ExprRes *Expr );
struct ExprRes* doArith( struct ExprRes* Res1, struct ExprRes* Res2, char op );
struct InstrSeq* doPrint( struct ExprRes* Expr );
struct BExprRes* doBExpr( struct ExprRes* Res1, struct ExprRes* Res2, int op );
struct InstrSeq* doIf( struct BExprRes* bRes, struct InstrSeq* seq );

void Finish( struct InstrSeq* Code );

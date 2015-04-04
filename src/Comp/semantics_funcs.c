#include "semantics.h"

char *curFuncName = NULL;

struct InstrSeq*
doReturn( struct ExprRes* Expr )
{
    struct FuncType *fType = GetAttr(FindName( funcTab, curFuncName ));
    if( fType->Type != Expr->Type )
    {
        typeMismatch();
    }
    struct InstrSeq* code = Expr->Instrs;
    AppendSeq( code, GenInstr( NULL, "move",
                         "$v0",
                         TmpRegName( Expr->Reg ), NULL ) );
    AppendSeq( code, GenInstr( NULL, "jr", "$ra", NULL, NULL ) );
    ReleaseTmpReg( Expr->Reg );
    free( Expr );
    return code;
}

void doDecArg( struct ExprRes *res )
{
    struct ArgList *nEntry = malloc(sizeof(struct ArgList));
    nEntry->Next = argList;
    nEntry->ArgPos = argPos;
    argPos++;
    nEntry->Res = res;
    argList = nEntry;
}

void freeArgList()
{
    struct ArgList *aEnt = argList;
    while(aEnt)
    {
        struct ArgList *temp = aEnt->Next;
        free(aEnt);
        aEnt = temp;
    }
    argPos = 0;
    argList = NULL;
}

char *findArgAt(int pos)
{
    struct SymTab *tab = tabList->Tab;
    struct SymEntry *ent = FirstEntry(tab);
    while( ent )
    {
        struct VarType *vType = (struct VarType *)GetAttr(ent);
        if(vType->Arg && vType->ArgPos == pos)
        {
            return (char *)GetName(ent);
        }
        ent = NextEntry(tab, ent);
    }
    WriteIndicator( GetCurrentColumn() );
    WriteMessage( "Too Many Arguments" );
    exit( 1 );
}

struct ExprRes*
doCall( char* name )
{
    char buf[ 1024 ];
    snprintf( buf, 1024, "_%s", name );
    struct SymEntry* entry = FindName( funcTab, buf );
    if ( !entry )
    {
        WriteIndicator( GetCurrentColumn() );
        WriteMessage( "Function not declared" );
        exit( 1 );
    }
    struct ExprRes* ret = malloc( sizeof( struct ExprRes ) );
    struct FuncType* fType = (struct FuncType*)GetAttr( entry );
    
    // Swap variable context and store args
    struct InstrSeq *argInstrs = NULL;
    struct SymTab *tempTab = tabList->Tab;
    tabList->Tab = fType->Tab;
    struct ArgList *aEnt = argList;
    while(aEnt)
    {
        argInstrs = AppendSeq( argInstrs, doAssign(findArgAt(aEnt->ArgPos), aEnt->Res, 1));
        aEnt = aEnt->Next;
    }
    tabList->Tab = tempTab;
   
    ret->Type = fType->Type;
    ret->Reg = AvailTmpReg();
    ret->Instrs = SaveSeq();
    AppendSeq( ret->Instrs, GenInstr( NULL, "subu", "$s0",
                                "$s0", Imm( fType->VarRsrv ) ) );
    AppendSeq( ret->Instrs, argInstrs);
    AppendSeq( ret->Instrs, GenInstr( NULL, "move", "$sp",
                                "$s0", NULL ) );
    AppendSeq( ret->Instrs, GenInstr( NULL, "jal", buf, NULL, NULL ) );
    AppendSeq( ret->Instrs, GenInstr( NULL, "addu", "$sp",
                                "$sp", Imm( fType->VarRsrv ) ) );
    AppendSeq( ret->Instrs, RestoreSeq() );
    AppendSeq( ret->Instrs, GenInstr( NULL, "move",
                                TmpRegName( ret->Reg ),
                                "$v0", NULL ) );
    freeArgList();
    return ret;
}

void
doFuncInit( char *name, int type )
{
    char buf[ 1024 ];
    snprintf( buf, 1024, "_%s", name );
    struct SymEntry* entry;
    struct FuncType* fType;
    if ( FindName( funcTab, buf ) )
    {
        WriteIndicator( GetCurrentColumn() );
        WriteMessage( "Function Redefinition Illegal" );
        exit( 1 );
    }
    EnterName( funcTab, buf, &entry );
    curFuncName = (char *)GetName(entry);
    fType = malloc( sizeof( struct FuncType ) );
    fType->Type = type;
    fType->VarRsrv = sPos;
    fType->Tab = tabList->Tab;
    SetAttr( entry, (void*)fType );
}

struct InstrSeq*
doDecFunc( char* name, struct InstrSeq* code)
{
    char buf[ 1024 ];
    snprintf( buf, 1024, "_%s", name );
    struct InstrSeq* nCode;
    nCode = GenInstr( buf, NULL, NULL, NULL, NULL );
    AppendSeq( nCode, code );
    AppendSeq( nCode, GenInstr( NULL, "jr", "$ra", NULL, NULL ) );
    doPopDecs();
    return nCode;
}

struct InstrSeq* doFuncInstrs( struct ExprRes* res )
{
    struct InstrSeq* code = res->Instrs;
    ReleaseTmpReg( res->Reg );
    free( res );
    return code;
}

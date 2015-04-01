/* Semantics.c
   Support and semantic action routines.
*/

#include "semantics.h"

/* Semantics support routines */

void typeMismatch()
{
    WriteIndicator( GetCurrentColumn() );
    WriteMessage( "Type Mismatch" );
    exit( 1 );
}

void doDeclare( char* name, int type, int size )
{
    struct SymEntry* ent;
    struct VarType *vType = malloc(sizeof(struct VarType));
    vType->Type = type;
    vType->Size = size;
    EnterName( table, name, &ent );
    SetAttr( ent, (void*)vType );
}

struct ExprRes *
doArrVal( char *name, struct ExprRes *Pos )
{
    char buf[50];
    int reg_addr = AvailTmpReg();
    struct VarType *vType;
    struct SymEntry *ent = FindName( table, name );
    if (!ent)
    {
        WriteIndicator( GetCurrentColumn() );
        WriteMessage( "Undeclared Variable" );
        exit( 1 );
    }
    if(Pos->Type != T_INT){
        typeMismatch();
    }
    snprintf(buf, 50, "0(%s)", TmpRegName(reg_addr));
    vType = (struct VarType *)GetAttr( ent );
    AppendSeq(Pos->Instrs, GenInstr(NULL, "la",
                TmpRegName(reg_addr),
                name, NULL));
    AppendSeq(Pos->Instrs, GenInstr(NULL, "mul",
                TmpRegName(Pos->Reg),
                TmpRegName(Pos->Reg),
                "4"));
    AppendSeq(Pos->Instrs, GenInstr(NULL, "add",
                TmpRegName(reg_addr),
                TmpRegName(reg_addr),
                TmpRegName(Pos->Reg)));
    AppendSeq(Pos->Instrs, GenInstr(NULL, "lw",
                TmpRegName(Pos->Reg),
                buf, NULL));

    if(vType->Type == T_BOOL_ARR)
    {
        Pos->Type = T_BOOL;
    }
    else 
    {
        Pos->Type = T_INT;
    }
    ReleaseTmpReg(reg_addr);
    return Pos;
}

struct ExprRes*
doRval( char* name )
{
    struct ExprRes* res;
    struct VarType *vType;
    struct SymEntry* ent = FindName( table, name );
    if ( !ent )
    {
        WriteIndicator( GetCurrentColumn() );
        WriteMessage( "Undeclared Variable" );
        exit( 1 );
    }
    vType = (struct VarType *)GetAttr( ent );
    res = (struct ExprRes*)malloc( sizeof( struct ExprRes ) );
    res->Reg = AvailTmpReg();
    res->Instrs = GenInstr( NULL, "lw", TmpRegName( res->Reg ), name, NULL );
    res->Type = vType->Type;
    return res;
}

struct InstrSeq*
doPrintSpaceHlp()
{
    struct InstrSeq* code;
    code = GenInstr( NULL, "li", "$v0", "4", NULL );
    AppendSeq( code, GenInstr( NULL, "la", "$a0", "_sp", NULL ) );
    AppendSeq( code, GenInstr( NULL, "syscall", NULL, NULL, NULL ) );
    return code;
}

struct InstrSeq*
doPrintLn()
{
    struct InstrSeq* code;
    code = GenInstr( NULL, "li", "$v0", "4", NULL );
    AppendSeq( code, GenInstr( NULL, "la", "$a0", "_nl", NULL ) );
    AppendSeq( code, GenInstr( NULL, "syscall", NULL, NULL, NULL ) );
    return code;
}

struct InstrSeq*
doPrintList( struct ExprRes* Res1, struct InstrSeq* instrs )
{
    struct InstrSeq* res;
    res = doPrint( Res1 );
    AppendSeq( res, doPrintSpaceHlp() );
    AppendSeq( res, instrs );
    return res;
}

struct InstrSeq*
doPrint( struct ExprRes* Expr )
{
    if ( Expr->Type == T_BOOL )
        return doPrintBool( Expr );
    if ( Expr->Type == T_INT )
        return doPrintInt( Expr );
    if ( Expr->Type == T_STR )
        return doPrintStr( Expr );

    typeMismatch();
    return NULL;
}

struct InstrSeq*
doPrintSp( struct ExprRes* Expr )
{
    if ( Expr->Type != T_INT )
    {
        typeMismatch();
    }

    struct InstrSeq* code;
    int reg_count = AvailTmpReg();
    int reg_one = AvailTmpReg();
    char* s_label = GenLabel();
    char* e_label = GenLabel();
    code = Expr->Instrs;
    AppendSeq( code, GenInstr( NULL, "addi",
                         TmpRegName( reg_one ),
                         "$0", "1" ) );
    AppendSeq( code, GenInstr( NULL, "blt",
                         TmpRegName( Expr->Reg ),
                         TmpRegName( reg_one ),
                         e_label ) );
    AppendSeq( code, GenInstr( NULL, "move",
                         TmpRegName( reg_count ),
                         "$0", NULL ) );
    AppendSeq( code, GenInstr( s_label, NULL, NULL, NULL, NULL ) );
    AppendSeq( code, doPrintSpaceHlp() );
    AppendSeq( code, GenInstr( NULL, "add",
                         TmpRegName( reg_count ),
                         TmpRegName( reg_count ),
                         TmpRegName( reg_one ) ) );
    AppendSeq( code, GenInstr( NULL, "blt",
                         TmpRegName( reg_count ),
                         TmpRegName( Expr->Reg ),
                         s_label ) );
    AppendSeq( code, GenInstr( e_label, NULL, NULL, NULL, NULL ) );
    ReleaseTmpReg( Expr->Reg );
    ReleaseTmpReg( reg_count );
    ReleaseTmpReg( reg_one );
    free( Expr );
    free( s_label );
    free( e_label );
    return code;
}

struct InstrSeq *
doAssignArr( char *name, struct ExprRes *Expr, struct ExprRes *Pos)
{
    char buf[50];
    int reg_addr = AvailTmpReg();
    struct InstrSeq *code;
    struct VarType *vType;
    struct SymEntry *ent = FindName( table, name );
    if ( !ent ) 
    {
        WriteIndicator( GetCurrentColumn() );
        WriteMessage( "Undeclared variable" );
        exit( 1 );
    }
    vType = (struct VarType *)GetAttr( ent );
    if( (Expr->Type == T_BOOL && vType->Type != T_BOOL_ARR) ||
            (Expr->Type == T_INT && vType->Type != T_INT_ARR) ||
                Pos->Type != T_INT) 
    {
        typeMismatch();
    }
    snprintf(buf, 50, "0(%s)", TmpRegName(reg_addr));
    code = Expr->Instrs;
    AppendSeq( code, Pos->Instrs );
    AppendSeq( code, GenInstr( NULL, "la", 
                TmpRegName( reg_addr ),
                name, NULL ) );
    AppendSeq( code, GenInstr( NULL, "mul",
                TmpRegName( Pos->Reg ),
                TmpRegName( Pos->Reg ),
                "4" ) );
    AppendSeq( code, GenInstr( NULL, "add",
                TmpRegName( reg_addr ),
                TmpRegName( reg_addr ),
                TmpRegName( Pos->Reg ) ) );
    AppendSeq( code, GenInstr( NULL, "sw", 
                TmpRegName( Expr->Reg), 
                buf, NULL ) );
    ReleaseTmpReg( Expr-> Reg );
    ReleaseTmpReg(Pos->Reg);
    ReleaseTmpReg(reg_addr);
    free(Expr);
    free(Pos);
    return code;
}

struct InstrSeq*
doAssign( char* name, struct ExprRes* Expr )
{
    struct InstrSeq* code;
    struct SymEntry* ent = FindName( table, name );
    if ( !ent )
    {
        WriteIndicator( GetCurrentColumn() );
        WriteMessage( "Undeclared variable" );
        exit( 1 );
    }
    if ( ((struct VarType *)GetAttr( ent ))->Type != Expr->Type )
    {
        typeMismatch();
    }
    code = Expr->Instrs;
    AppendSeq( code, GenInstr( NULL, "sw", TmpRegName( Expr->Reg ), name, NULL ) );
    ReleaseTmpReg( Expr->Reg );
    free( Expr );
    return code;
}

struct InstrSeq*
doIf( struct ExprRes* Expr, struct InstrSeq* code )
{
    if ( Expr->Type != T_BOOL )
    {
        typeMismatch();
    }

    struct InstrSeq* nCode = Expr->Instrs;
    char* end_label = GenLabel();
    AppendSeq( nCode, GenInstr( NULL, "beq",
                          TmpRegName( Expr->Reg ),
                          "$0",
                          end_label ) );
    AppendSeq( nCode, code );
    AppendSeq( nCode, GenInstr( end_label, NULL, NULL, NULL, NULL ) );
    ReleaseTmpReg( Expr->Reg );
    free( end_label );
    free( Expr );
    return nCode;
}

struct InstrSeq*
doIfElse( struct ExprRes* Expr, struct InstrSeq* iCode, struct InstrSeq* eCode )
{
    if ( Expr->Type != T_BOOL )
    {
        typeMismatch();
    }

    struct InstrSeq* nCode = Expr->Instrs;
    char* end_label = GenLabel();
    char* else_label = GenLabel();
    AppendSeq( nCode, GenInstr( NULL, "beq",
                          TmpRegName( Expr->Reg ),
                          "$0",
                          else_label ) );
    AppendSeq( nCode, iCode );
    AppendSeq( nCode, GenInstr( NULL, "b", end_label, NULL, NULL ) );
    AppendSeq( nCode, GenInstr( else_label, NULL, NULL, NULL, NULL ) );
    AppendSeq( nCode, eCode );
    AppendSeq( nCode, GenInstr( end_label, NULL, NULL, NULL, NULL ) );
    ReleaseTmpReg( Expr->Reg );
    free( end_label );
    free( else_label );
    return nCode;
}

struct InstrSeq*
doWhile( struct ExprRes* Expr, struct InstrSeq* code )
{
    if ( Expr->Type != T_BOOL )
    {
        typeMismatch();
    }

    char* s_label = GenLabel();
    char* e_label = GenLabel();
    struct InstrSeq* nCode;
    nCode = GenInstr( s_label, NULL, NULL, NULL, NULL );
    AppendSeq( nCode, Expr->Instrs );
    AppendSeq( nCode, GenInstr( NULL, "beq",
                          TmpRegName( Expr->Reg ),
                          "$0",
                          e_label ) );
    AppendSeq( nCode, code );
    AppendSeq( nCode, GenInstr( NULL, "b", s_label, NULL, NULL ) );
    AppendSeq( nCode, GenInstr( e_label, NULL, NULL, NULL, NULL ) );
    ReleaseTmpReg( Expr->Reg );
    free( s_label );
    free( e_label );
    free( Expr );
    return nCode;
}

struct InstrSeq*
doReadList( char* var, struct InstrSeq* code )
{
    struct InstrSeq* instrs;
    instrs = doRead( var );
    AppendSeq( instrs, code );
    return instrs;
}

struct InstrSeq*
doRead( char* var )
{
    if ( !FindName( table, var ) )
    {
        WriteIndicator( GetCurrentColumn() );
        WriteMessage( "Undeclared variable" );
        exit( 1 );
    }

    struct InstrSeq* code;
    code = GenInstr( NULL, "li", "$v0", "5", NULL );
    AppendSeq( code, GenInstr( NULL, "syscall", NULL, NULL, NULL ) );
    AppendSeq( code, GenInstr( NULL, "sw", "$v0", var, NULL ) );
    return code;
}

void Finish( struct InstrSeq* Code )
{
    struct InstrSeq* code;
    struct SymEntry* entry;
    struct Attr* attr;
    struct StrLitList* strEnt;

    code = GenInstr( NULL, ".text", NULL, NULL, NULL );
    AppendSeq( code, GenInstr( NULL, ".globl", "main", NULL, NULL ) );
    AppendSeq( code, GenInstr( "main", NULL, NULL, NULL, NULL ) );
    AppendSeq( code, Code );
    AppendSeq( code, GenInstr( NULL, "li", "$v0", "10", NULL ) );
    AppendSeq( code, GenInstr( NULL, "syscall", NULL, NULL, NULL ) );
    AppendSeq( code, GenInstr( NULL, ".data", NULL, NULL, NULL ) );
    AppendSeq( code, GenInstr( NULL, ".align", "4", NULL, NULL ) );
    AppendSeq( code, GenInstr( "_nl", ".asciiz", "\"\\n\"", NULL, NULL ) );
    AppendSeq( code, GenInstr( "_true", ".asciiz", "\"true\"", NULL, NULL ) );
    AppendSeq( code, GenInstr( "_false", ".asciiz", "\"false\"", NULL, NULL ) );
    AppendSeq( code, GenInstr( "_sp", ".asciiz", "\" \"", NULL, NULL ) );

    entry = FirstEntry( table );
    while ( entry )
    {
        AppendSeq(code, GenInstr( NULL, ".align", "4", NULL, NULL) );
        struct VarType *vType;
        vType = GetAttr(entry);
        if(vType->Type == T_BOOL_ARR || vType->Type == T_INT_ARR)
        {
            char buf[50];
            snprintf(buf, 50, "%d", vType->Size * 4);
            AppendSeq( code, GenInstr( (char*)GetName( entry ), ".space", buf, NULL, NULL ) );
        }
        else
        {
            AppendSeq( code, GenInstr( (char*)GetName( entry ), ".word", "0", NULL, NULL ) );
        }
        entry = NextEntry( table, entry );
    }

    strEnt = strList;
    while ( strEnt )
    {
        AppendSeq(code, GenInstr( NULL, ".align", "4", NULL, NULL) );
        AppendSeq( code, GenInstr( strEnt->label, ".asciiz", strEnt->val, NULL, NULL ) );
        strEnt = strEnt->next;
    }

    WriteSeq( code );

    return;
}


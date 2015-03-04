#include "semantics.h"

extern struct SymTab *table;

void
printSymTab()
{
    struct SymEntry *entry = FirstEntry(table);
    printf("%20s\t%10s\n", "Variable", "Value");
    while(entry)
    {
        printf("%20s\t%10d\n", GetName(entry), getVal(GetName(entry)));
        entry = NextEntry(table, entry);
    }
}

void
storeVar(char *name, int32_t v)
{
    int32_t *nVal = malloc(sizeof(int32_t));
    *nVal = v;
    struct SymEntry *nEntry = NULL;
    EnterName(table, name, &nEntry);
    SetAttr(nEntry, (void *)nVal);
}

int32_t
getVal(const char *name)
{
    struct SymEntry *entry = FindName(table, name);
    if(entry)
    {
        return *((int32_t *)GetAttr(entry));
    }
    printf("Runtime Error: Variable %s Not Declared\n", name);
    exit(1);
}

int32_t
doMult(int32_t x, int32_t y)
{
    return x * y;
}

int32_t
doPlus(int32_t x, int32_t y)
{
    return x + y;
}

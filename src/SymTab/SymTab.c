// SymTab.c
// Author: Nicholas Sullivan
#include "SymTab.h"

#define PRIME1 314606869
#define PRIME2 553105253
#define PRIME3 674506111
#define PRIME4 141650963
#define rotl32(x, r) ((x << r) | (x >> (32 - r)))
#define IDX(hash, pSize) hash & ((1 << pSize) - 1)

static uint32_t
hash(const char *name)
{
    uint32_t hash = 982451653;
    uint32_t len = strlen(name);
    while (len > 0)
    {
        uint8_t a = *name;
        name++;
        len--;
        hash ^= PRIME4 * a;
        hash = rotl32(hash, 11) * PRIME1;
    }
    hash ^= hash >> 17;
    hash *= PRIME2;
    hash ^= hash >> 11;
    hash *= PRIME3;
    hash ^= hash >> 16;
    return hash;
}

struct SymTab *
CreateSymTab(int Size)
{
    struct SymTab *nTab = malloc(sizeof(struct SymTab));
    uint32_t pSize = 1;
    while (pow(2, pSize) < Size)
    {
        pSize++;
    }
    nTab->Size = pSize;
    nTab->Contents = calloc(pow(2, pSize), sizeof(struct SymEntry *));
    return nTab;
}

void
DestroySymTab(struct SymTab *ATable)
{
    for (int i = 0; i < pow(2, ATable->Size); i++)
    {
        struct SymEntry *p = ATable->Contents[i];
        while (p)
        {
            struct SymEntry *n = p->Next;
            free(p->Name);
            free(p);
            p = n;
        }
    }
    free(ATable->Contents);
    free(ATable);
}

bool
EnterName(struct SymTab *ATable,
          const char *Name,
          struct SymEntry **AnEntry)
{
    struct SymEntry *nEntry = FindName(ATable, Name);
    if (nEntry)
    {
        if (AnEntry)
        {
            *AnEntry = nEntry;
        }
        return 0;
    }
    nEntry = malloc(sizeof(struct SymEntry));
    if (AnEntry)
    {
        *AnEntry = nEntry;
    }
    nEntry->Name = malloc(strlen(Name) + 1);
    strcpy(nEntry->Name, Name);
    nEntry->Attributes = NULL;
    uint32_t loc = IDX(hash(nEntry->Name), ATable->Size);
    nEntry->Next = ATable->Contents[loc];
    ATable->Contents[loc] = nEntry;
    return 1;
}

struct SymEntry *
FindName(struct SymTab *ATable, const char *Name)
{
    struct SymEntry *p;
    uint32_t loc = IDX(hash(Name), ATable->Size);
    p = ATable->Contents[loc];
    while (p && strcmp(Name, p->Name) != 0)
    {
        p = p->Next;
    }
    return p;
}

void
SetAttr(struct SymEntry *AnEntry, void *Attributes)
{
    AnEntry->Attributes = Attributes;
}

void *
GetAttr(struct SymEntry *AnEntry)
{
    return AnEntry->Attributes;
}

const char *
GetName(struct SymEntry *AnEntry)
{
    return AnEntry->Name;
}

struct SymEntry *
FirstEntry(struct SymTab *ATable)
{
    for (int i = 0; i < pow(2, ATable->Size); i++)
    {
        if (ATable->Contents[i])
        {
            return ATable->Contents[i];
        }
    }
    return NULL;
}

struct SymEntry *
NextEntry(struct SymTab *ATable, struct SymEntry *AnEntry)
{
    if (AnEntry->Next)
    {
        return AnEntry->Next;
    }
    uint32_t loc = IDX(hash(AnEntry->Name), ATable->Size);
    for (int i = loc + 1; i < pow(2, ATable->Size); i++)
    {
        if (ATable->Contents[i])
        {
            return ATable->Contents[i];
        }
    }
    return NULL;
}

// driver.c
// SymTab Test Driver
// Author: Nicholas Sullivan
#include <stdio.h>
#include <time.h>
#include "SymTab.h"

int
main()
{
    srand(time(NULL));
    uint32_t ins = 0;
    struct SymTab *tab = CreateSymTab(100000);
    struct SymEntry *ent = FindName(tab, "test");
    if(!ent)
    {
        puts("empty find passed");
    }
    else
    {
        puts("empty find failed");
    }

    char **names = malloc(sizeof(char *) * 10000);
    for(int i = 0; i < 1000000; i++)
    {
        names[i] = malloc(10);
        for(int j = 0; j < 9; j++)
        {
            names[i][j] = rand() % 40 + 48;
        }
        names[i][9] = '\0';
        if(EnterName(tab, names[i], NULL))
        {
            ins++;
        }
    }

    for(int i = 0; i < 1000000; i++)
    {
        ent = FindName(tab, names[i]);
        if(strcmp(names[i], ent->Name) != 0)
        {
            printf("%s\n",ent->Name);
            puts("Name Mismatch");
            break;
        }
    }

    ent = FindName(tab, "aaa");
    if (ent)
    {
        puts("full find failed");
    }
    else 
    {
        puts("full find passed");
    }

    uint32_t freq = 0;
    for(int i = 0; i < 10000; i++)
    {
        int count = 0;
        struct SymEntry *p = tab->Contents[i];
        while(p)
        {
            count++;
            p = p->Next;
        }
        if (count > freq)
        {
            freq = count;
        }
    }

    uint32_t enumd = 0;
    ent = FirstEntry(tab);
    while(ent)
    {
        ent = NextEntry(tab, ent);
        enumd++;
    }

    DestroySymTab(tab);
    printf("Enumd: %u\n", enumd);
    printf("Inserted: %u\n", ins);
    printf("Max Chain: %u\n", freq);
    return 0;
}

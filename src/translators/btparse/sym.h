#include <ctype.h>

/* 
 * Declarations for symbol table in sym.c 
 */

/* define some hash function */
#ifndef HASH_FUN
#define HASH_FUN(p, h) while ( *p != '\0' ) h = (h<<1) + tolower (*p++);
#endif

/* minimum symbol table record */
typedef struct _sym 
{
   char        *symbol;         /* the macro name */
   char        *text;           /* its expansion */
   struct _sym *next, *prev, **head, *scope;
   unsigned int hash;
} Sym, *SymPtr;

void zzs_init(int, int);
void zzs_free(void);
void zzs_done(void);
void zzs_add(char *, Sym *);
Sym *zzs_get(char *);
void zzs_del(Sym *);
void zzs_keydel(char *);
Sym **zzs_scope(Sym **);
Sym *zzs_rmscope(Sym **);
void zzs_stat(void);
Sym *zzs_new(char *);
Sym *zzs_newadd(char *);
char *zzs_strdup(char *);

/* Symbol table manager for Bison,
   Copyright (C) 1984, 1989 Free Software Foundation, Inc.

This file is part of Bison, the GNU Compiler Compiler.

Bison is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

Bison is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Bison; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */


#include <stdio.h>
#include "system.h"
#include "new.h"
#include "symtab.h"
#include "gram.h"


bucket **symtab;
bucket *firstsymbol;
bucket *lastsymbol;



int
hash(key)
char *key;
{
  register char *cp;
  register int k;

  cp = key;
  k = 0;
  while (*cp)
    k = ((k << 1) ^ (*cp++)) & 0x3fff;

  return (k % TABSIZE);
}



char *
copys(s)
char *s;
{
  return strcpy(xmalloc(strlen(s) + 1), s);
}


void
tabinit()
{
/*   register int i; JF unused */

  symtab = NEW2(TABSIZE, bucket *);

  firstsymbol = NULL;
  lastsymbol = NULL;
  nsyms = 0;
}


bucket *
getsym(key)
char *key;
{
  register int hashval;
  register bucket *bp;

  hashval = hash(key);

  for (bp = symtab[hashval]; bp != NULL; bp = bp->link)
    if (strcmp(key, bp->tag) == 0)
      return (bp);

  bp = NEW(bucket);
  bp->link = symtab[hashval];
  symtab[hashval] = bp;
  bp->next = NULL;
  bp->tag = copys(key);
  bp->value = nsyms++;

  if (firstsymbol == NULL)
    {
      firstsymbol = bp;
      lastsymbol = bp;
    }
  else
    {
      lastsymbol->next = bp;
      lastsymbol = bp;
    }

  return (bp);
}


void
free_symtab()
{
  register int i;
  register bucket *bp,*bptmp;/* JF don't use ptr after free */

  for (i = 0; i < TABSIZE; i++)
    {
      bp = symtab[i];
      while (bp)
	{
	  bptmp = bp->link;
#if 0 /* This causes crashes because one string can appear more than once.  */
	  if (bp->type_name)
	    FREE(bp->type_name);
#endif
	  FREE(bp);
	  bp = bptmp;
	}
    }
  FREE(symtab);
}

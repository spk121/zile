#ifndef __LISTS_H__
#define __LISTS_H__

#include <stdio.h>

typedef struct le{
  /* either data or a branch */
  struct le * branch;
  char * data;
  int quoted;
  int tag;

  /* for the next in the list in the current parenlevel */
  struct le * list_prev;
  struct le * list_next;
} le;

le * leNew(char * text);
void leDelete(le * element);
void leWipe(le * list);

le * leAddHead(le * list, le * element);
le * leAddTail(le * list, le * element);

le * leAddBranchElement( le * list, le * branch, int quoted );
le * leAddDataElement( le * list, char * data, int quoted );
le * leDup( le * list );

void leClearTag( le * list );
void leTagData(le * list, char * data, int tagval);
void leTagReplace(le * list, int tagval, le * newinfo);

void leDump( le * list, int indent );
void leDumpEval( le * list, int indent );
void leDumpEvalTree( le * list, int indent );
void leDumpReformat(FILE * of, le * tree);
#endif

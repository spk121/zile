#include <stdio.h>
#include "lists.h"

enum tokenname { 
  T_NONE, 
  T_CLOSEPAREN, 
  T_OPENPAREN, 
  T_NEWLINE, 
  T_QUOTE,
  T_WORD, 
  T_EOF
};

typedef int 
(*getcCallback)
  (
   void
   );
typedef void
(*ungetcCallback)
  (
   int c
   );

char *
snagAToken(
           getcCallback getachar,
           ungetcCallback ungetachar,
           enum tokenname * tokenid
           );
struct le *
parseInFile(
            getcCallback getachar,
            ungetcCallback ungetachar,
            struct le * list,
            int * line
            );

#include <stdio.h>
#include "zile.h"
#include "extern.h"
#include "parser.h"
#include "vars.h"

FILE * fp = NULL;
    
static void myungetc(int c)
{
  ungetc(c, fp);
}
    
static int mygetc(void)
{
  return getc(fp);
}

void lithp(char* file)
{
  int lineno;
  struct le *list = NULL;

  /* parse in the file */
  printf("==== File %s\n", file);
  fp = fopen(file, "r");

  if (!fp)
    fprintf(stderr, "ERROR: Couldn't open \"%s\".\n", file);
  else {
    lineno = 0;
    list = parseInFile(mygetc, myungetc, list, &lineno);
    fclose(fp);
    fp = NULL;
  }

  /* evaluate the read-in lists and free */
  leDumpEval(list, 0); 
  leWipe(list);

  /* display the variables and free */
  printf("Variables:\n");
  variableDump(mainVarList);
  variableFree(mainVarList);

  /* display the user-defined functions and free */
  printf("defun's:\n");
  variableDump(defunList);
  variableFree(defunList);
}

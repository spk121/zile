/* C code produced by gperf version 2.5 (GNU C++ version) */
/* Command-line: gperf -ac -N is_c_keyword fontlock_c.gperf  */

#define TOTAL_KEYWORDS 53
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 10
#define MIN_HASH_VALUE 2
#define MAX_HASH_VALUE 93
/* maximum key range = 92, duplicates = 0 */

static unsigned int
hash (register const char *str, register int len)
{
  static unsigned char asso_values[] =
    {
     94, 94, 94, 94, 94, 94, 94, 94, 94, 94,
     94, 94, 94, 94, 94, 94, 94, 94, 94, 94,
     94, 94, 94, 94, 94, 94, 94, 94, 94, 94,
     94, 94, 94, 94, 94, 94, 94, 94, 94, 94,
     94, 94, 94, 94, 94, 94, 94, 94, 94, 94,
     94, 94, 94, 94, 94, 94, 94, 94, 94, 94,
     94, 94, 94, 94, 94, 94, 94, 94, 94, 94,
     94, 94, 94, 94, 94, 94, 94, 94, 94, 94,
     94, 94, 94, 94, 94, 94, 94, 94, 94, 94,
     94, 94, 94, 94, 94, 20, 94, 50, 55, 40,
      0,  0, 31, 15, 30, 60, 94,  5, 10, 10,
      5,  0, 94, 20, 15, 10,  5,  0, 26,  0,
     45, 15, 94, 94, 94, 94, 94, 94,
    };
  return len + asso_values[str[len - 1]] + asso_values[str[0]];
}

char *
is_c_keyword (register const char *str, register int len)
{
  static char *wordlist[] =
    {
      "", "", 
      "do", 
      "", 
      "else", 
      "while", 
      "double", 
      "", 
      "unsigned", 
      "true", 
      "union", 
      "extern", 
      "default", 
      "not", 
      "enum", 
      "", 
      "signed", 
      "or", 
      "", 
      "goto", 
      "short", 
      "struct", 
      "", "", "", 
      "or_eq", 
      "return", 
      "", 
      "restrict", 
      "long", 
      "void", 
      "not_eq", 
      "", "", 
      "volatile", 
      "_Bool", 
      "false", 
      "", 
      "register", 
      "", "", 
      "float", 
      "", 
      "typedef", 
      "case", 
      "_Imaginary", 
      "switch", 
      "sizeof", 
      "continue", 
      "for", 
      "const", 
      "", "", 
      "and", 
      "auto", 
      "compl", 
      "static", 
      "", "", 
      "char", 
      "", 
      "bitand", 
      "", 
      "xor", 
      "", 
      "break", 
      "inline", 
      "", 
      "int", 
      "bool", 
      "", 
      "xor_eq", 
      "", 
      "_Complex", 
      "", 
      "bitor", 
      "and_eq", 
      "", "", "", "", "", "", "", 
      "imaginary", 
      "", "", "", "", "", "", "", 
      "complex", 
      "if", 
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          register char *s = wordlist[key];

          if (*s == *str && !strncmp (str + 1, s + 1, len - 1))
            return s;
        }
    }
  return 0;
}

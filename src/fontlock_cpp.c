/* C code produced by gperf version 2.5 (GNU C++ version) */
/* Command-line: gperf -ac -D -N is_cpp_keyword fontlock_cpp.gperf  */

#define TOTAL_KEYWORDS 74
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 16
#define MIN_HASH_VALUE 3
#define MAX_HASH_VALUE 175
/* maximum key range = 173, duplicates = 3 */

static unsigned int
hash (register const char *str, register int len)
{
  static unsigned char asso_values[] =
    {
     176, 176, 176, 176, 176, 176, 176, 176, 176, 176,
     176, 176, 176, 176, 176, 176, 176, 176, 176, 176,
     176, 176, 176, 176, 176, 176, 176, 176, 176, 176,
     176, 176, 176, 176, 176, 176, 176, 176, 176, 176,
     176, 176, 176, 176, 176, 176, 176, 176, 176, 176,
     176, 176, 176, 176, 176, 176, 176, 176, 176, 176,
     176, 176, 176, 176, 176, 176, 176, 176, 176, 176,
     176, 176, 176, 176, 176, 176, 176, 176, 176, 176,
     176, 176, 176, 176, 176, 176, 176, 176, 176, 176,
     176, 176, 176, 176, 176, 176, 176,  40,  10,  99,
      80,  45,  75,  65,  35,  40, 176,  10,  10,   5,
      95,  55,   0,   0,  15,  70,   0,   0,  50,  15,
       0,   0, 176, 176, 176, 176, 176, 176,
    };
  return len + asso_values[str[len - 1]] + asso_values[str[0]];
}

char *
is_cpp_keyword (register const char *str, register int len)
{
  static char *wordlist[] =
    {
      "", "", "", 
      "try", 
      "xor_eq", 
      "xor", 
      "throw", 
      "wchar_t", 
      "bool", 
      "break", 
      "bitor", 
      "reinterpret_cast", 
      "register", 
      "int", 
      "hasm", 
      "and_eq", 
      "true", 
      "export", 
      "private", 
      "explicit", 
      "typename", 
      "template", 
      "enum", 
      "mutable", 
      "or_eq", 
      "while", 
      "virtual", 
      "using", 
      "or", 
      "this", 
      "short", 
      "struct", 
      "operator", 
      "long", 
      "float", 
      "static_cast", 
      "typedef", 
      "typeid", 
      "default", 
      "unsigned", 
      "protected", 
      "inline", 
      "dynamic_cast", 
      "for", 
      "else", 
      "bitand", 
      "not", 
      "auto", 
      "union", 
      "not_eq", 
      "volatile", 
      "const", 
      "public", 
      "const_cast", 
      "switch", 
      "new", 
      "compl", 
      "return", 
      "if", 
      "char", 
      "and", 
      "goto", 
      "false", 
      "delete", 
      "double", 
      "void", 
      "do", 
      "catch", 
      "extern", 
      "case", 
      "namespace", 
      "sizeof", 
      "continue", 
      "signed", 
      "friend", 
      "class", 
      "static", 
    };

  static short lookup[] =
    {
        -1,  -1,  -1,   3,  -1,  -1,   4,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        -1,  -1,  -1,  -1,   5,  -1,   6,  -1,   7,  -1,   8,   9,  -1,  -1,
        -1,  -1,  10,  11,  -1,  -1,  -1,  -1,  -1,  -1,  12,  -1,  -1,  -1,
        -1,  13,  14,  -1,  15,  -1,  -1,  16,  -1,  17,  18, 177,  22, -19,
        -3,  23,  -1,  -1,  24,  -1,  -1,  -1,  -1,  25,  -1,  26,  -1,  -1,
        27,  -1,  28,  -1,  29,  30,  31,  -1,  32,  33,  34,  35,  36,  -1,
        -1,  -1,  37,  38,  39,  40,  -1,  41,  42,  43,  44,  -1,  45,  -1,
        46,  47,  48,  49,  -1,  50,  51,  52,  -1,  -1,  -1,  53,  -1,  54,
        -1,  55,  56,  -1,  57,  58,  59,  -1,  -1,  -1,  -1,  60,  61,  62,
        -1,  -1,  -1,  -1,  -1, 176, -63,  -2,  65,  -1,  -1,  66,  -1,  67,
        -1,  -1,  -1,  -1,  -1,  -1,  68,  -1,  69,  70,  -1,  71,  72,  -1,
        -1,  -1,  73,  -1,  -1,  -1,  -1,  74,  -1,  -1,  -1,  -1,  -1,  -1,
        -1,  -1,  -1,  -1,  -1,  -1,  75,  76,
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          register int index = lookup[key];

          if (index >= 0 && index < MAX_HASH_VALUE)
            {
              register char *s = wordlist[index];

              if (*s == *str && !strncmp (str + 1, s + 1, len - 1))
                return s;
            }
          else if (index < 0 && index >= -MAX_HASH_VALUE)
            return 0;
          else
            {
              register int offset = key + index + (index > 0 ? -MAX_HASH_VALUE : MAX_HASH_VALUE);
              register char **base = &wordlist[-lookup[offset]];
              register char **ptr = base + -lookup[offset + 1];

              while (--ptr >= base)
                if (*str == **ptr && !strcmp (str + 1, *ptr + 1))
                  return *ptr;
            }
        }
    }
  return 0;
}

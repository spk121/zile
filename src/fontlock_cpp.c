#include "config.h"
#if ENABLE_CPP_MODE
/* ANSI-C code produced by gperf version 2.7.2 */
/* Command-line: gperf -LANSI-C -ac -k 1,2,3 -N is_cpp_keyword fontlock_cpp.gperf  */

#define TOTAL_KEYWORDS 74
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 16
#define MIN_HASH_VALUE 2
#define MAX_HASH_VALUE 226
/* maximum key range = 225, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
hash (register const char *str, register unsigned int len)
{
  static unsigned char asso_values[] =
    {
      227, 227, 227, 227, 227, 227, 227, 227, 227, 227,
      227, 227, 227, 227, 227, 227, 227, 227, 227, 227,
      227, 227, 227, 227, 227, 227, 227, 227, 227, 227,
      227, 227, 227, 227, 227, 227, 227, 227, 227, 227,
      227, 227, 227, 227, 227, 227, 227, 227, 227, 227,
      227, 227, 227, 227, 227, 227, 227, 227, 227, 227,
      227, 227, 227, 227, 227, 227, 227, 227, 227, 227,
      227, 227, 227, 227, 227, 227, 227, 227, 227, 227,
      227, 227, 227, 227, 227, 227, 227, 227, 227, 227,
      227, 227, 227, 227, 227,   0, 227,  25,  75,  40,
       55,   5,  30,  80,  20,  35, 227, 227,  65,  30,
        0,   0,  10, 227,   0, 105,   0,   0,   0,  50,
       57,  10,   5, 227, 227, 227, 227, 227, 227, 227,
      227, 227, 227, 227, 227, 227, 227, 227, 227, 227,
      227, 227, 227, 227, 227, 227, 227, 227, 227, 227,
      227, 227, 227, 227, 227, 227, 227, 227, 227, 227,
      227, 227, 227, 227, 227, 227, 227, 227, 227, 227,
      227, 227, 227, 227, 227, 227, 227, 227, 227, 227,
      227, 227, 227, 227, 227, 227, 227, 227, 227, 227,
      227, 227, 227, 227, 227, 227, 227, 227, 227, 227,
      227, 227, 227, 227, 227, 227, 227, 227, 227, 227,
      227, 227, 227, 227, 227, 227, 227, 227, 227, 227,
      227, 227, 227, 227, 227, 227, 227, 227, 227, 227,
      227, 227, 227, 227, 227, 227, 227, 227, 227, 227,
      227, 227, 227, 227, 227, 227, 227, 227, 227, 227,
      227, 227, 227, 227, 227, 227
    };
  register int hval = len;

  switch (hval)
    {
      default:
      case 3:
        hval += asso_values[(unsigned char)str[2]];
      case 2:
        hval += asso_values[(unsigned char)str[1]];
      case 1:
        hval += asso_values[(unsigned char)str[0]];
        break;
    }
  return hval;
}

#ifdef __GNUC__
__inline
#endif
const char *
is_cpp_keyword (register const char *str, register unsigned int len)
{
  static const char * wordlist[] =
    {
      "", "",
      "or",
      "not",
      "true",
      "or_eq",
      "not_eq",
      "", "",
      "enum",
      "",
      "return",
      "",
      "try",
      "", "", "", "", "",
      "protected",
      "", "", "",
      "operator",
      "",
      "throw",
      "typeid",
      "typedef",
      "typename",
      "auto",
      "", "", "",
      "for",
      "", "", "",
      "mutable",
      "int",
      "void",
      "union",
      "",
      "virtual",
      "template",
      "",
      "const",
      "", "",
      "continue",
      "",
      "const_cast",
      "",
      "private",
      "", "", "",
      "reinterpret_cast",
      "do",
      "new",
      "this",
      "xor",
      "double",
      "",
      "xor_eq",
      "namespace",
      "", "",
      "if",
      "extern",
      "long",
      "catch",
      "friend",
      "",
      "volatile",
      "",
      "compl",
      "",
      "dynamic_cast",
      "export",
      "bool",
      "explicit",
      "", "",
      "and",
      "goto",
      "break",
      "and_eq",
      "", "",
      "char",
      "",
      "public",
      "",
      "register",
      "", "", "",
      "default",
      "", "",
      "float",
      "", "", "", "", "",
      "inline",
      "", "", "",
      "while",
      "struct",
      "",
      "unsigned",
      "",
      "bitor",
      "bitand",
      "wchar_t",
      "", "", "", "", "", "", "",
      "false",
      "", "", "", "",
      "short",
      "delete",
      "", "", "",
      "class",
      "static",
      "", "", "", "",
      "static_cast",
      "", "", "",
      "using",
      "", "", "", "", "",
      "sizeof",
      "", "",
      "hasm",
      "", "", "", "", "", "", "", "", "",
      "", "", "", "", "", "", "", "", "",
      "",
      "case",
      "", "", "", "",
      "else",
      "", "", "", "", "", "", "", "", "",
      "", "", "", "", "", "", "",
      "switch",
      "", "", "", "", "", "", "", "", "",
      "", "", "", "", "", "", "", "", "",
      "", "", "", "", "", "", "", "", "",
      "", "",
      "signed"
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          register const char *s = wordlist[key];

          if (*str == *s && !strncmp (str + 1, s + 1, len - 1) && s[len] == '\0')
            return s;
        }
    }
  return 0;
}
#endif

#include "config.h"
#if ENABLE_CSHARP_MODE
/* ANSI-C code produced by gperf version 2.7.2 */
/* Command-line: gperf -LANSI-C -ac -k 1,2,4 -N is_csharp_keyword fontlock_csharp.gperf  */

#define TOTAL_KEYWORDS 79
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 10
#define MIN_HASH_VALUE 2
#define MAX_HASH_VALUE 262
/* maximum key range = 261, duplicates = 0 */

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
  static unsigned short asso_values[] =
    {
      263, 263, 263, 263, 263, 263, 263, 263, 263, 263,
      263, 263, 263, 263, 263, 263, 263, 263, 263, 263,
      263, 263, 263, 263, 263, 263, 263, 263, 263, 263,
      263, 263, 263, 263, 263, 263, 263, 263, 263, 263,
      263, 263, 263, 263, 263, 263, 263, 263, 263, 263,
      263, 263, 263, 263, 263, 263, 263, 263, 263, 263,
      263, 263, 263, 263, 263, 263, 263, 263, 263, 263,
      263, 263, 263, 263, 263, 263, 263, 263, 263, 263,
      263, 263, 263, 263, 263, 263, 263, 263, 263, 263,
      263, 263, 263, 263, 263, 263, 263,   0, 110,  80,
       15,   0, 115,  60,  95,  35, 263,  10,  30,   0,
       65,   0,  55, 263,  75,   0,  80,   5,  10,  15,
       10,  20, 263, 263, 263, 263, 263, 263, 263, 263,
      263, 263, 263, 263, 263, 263, 263, 263, 263, 263,
      263, 263, 263, 263, 263, 263, 263, 263, 263, 263,
      263, 263, 263, 263, 263, 263, 263, 263, 263, 263,
      263, 263, 263, 263, 263, 263, 263, 263, 263, 263,
      263, 263, 263, 263, 263, 263, 263, 263, 263, 263,
      263, 263, 263, 263, 263, 263, 263, 263, 263, 263,
      263, 263, 263, 263, 263, 263, 263, 263, 263, 263,
      263, 263, 263, 263, 263, 263, 263, 263, 263, 263,
      263, 263, 263, 263, 263, 263, 263, 263, 263, 263,
      263, 263, 263, 263, 263, 263, 263, 263, 263, 263,
      263, 263, 263, 263, 263, 263, 263, 263, 263, 263,
      263, 263, 263, 263, 263, 263, 263, 263, 263, 263,
      263, 263, 263, 263, 263, 263
    };
  register int hval = len;

  switch (hval)
    {
      default:
      case 4:
        hval += asso_values[(unsigned char)str[3]];
      case 3:
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
is_csharp_keyword (register const char *str, register unsigned int len)
{
  static const char * wordlist[] =
    {
      "", "",
      "as",
      "set",
      "", "", "", "",
      "out",
      "", "",
      "ushort",
      "", "", "", "",
      "extern",
      "do",
      "", "",
      "value",
      "",
      "default",
      "delegate",
      "", "", "", "", "",
      "void",
      "", "", "", "",
      "else",
      "",
      "sealed",
      "is",
      "", "", "",
      "sizeof",
      "", "",
      "lock",
      "", "", "",
      "explicit",
      "", "", "", "", "", "", "", "",
      "decimal",
      "", "", "",
      "params",
      "",
      "get",
      "goto",
      "", "", "",
      "new",
      "enum",
      "", "", "",
      "implicit",
      "namespace",
      "using",
      "unsafe",
      "",
      "ref",
      "",
      "event",
      "", "", "",
      "case",
      "const",
      "return",
      "", "", "", "",
      "struct",
      "",
      "override",
      "long",
      "",
      "public",
      "",
      "readonly",
      "", "",
      "switch",
      "in",
      "int",
      "null",
      "ulong",
      "typeof",
      "",
      "internal",
      "interface",
      "", "", "", "",
      "base",
      "class",
      "object",
      "",
      "for",
      "",
      "false",
      "string",
      "foreach",
      "",
      "uint",
      "", "", "", "", "", "",
      "double",
      "virtual",
      "",
      "byte",
      "", "", "",
      "operator",
      "", "", "", "", "",
      "bool",
      "while",
      "",
      "private",
      "", "",
      "float",
      "",
      "if",
      "", "",
      "fixed",
      "",
      "finally",
      "try",
      "true",
      "", "", "", "", "",
      "catch",
      "static",
      "",
      "continue",
      "",
      "stackalloc",
      "", "", "",
      "unchecked",
      "short",
      "", "", "",
      "this",
      "throw",
      "", "", "", "", "", "", "", "", "",
      "break",
      "", "", "", "",
      "sbyte",
      "", "",
      "abstract",
      "", "", "", "", "", "", "", "", "",
      "", "", "", "", "", "", "", "", "",
      "", "",
      "protected",
      "", "", "", "", "", "", "", "", "",
      "", "", "", "", "", "", "", "", "",
      "", "", "", "", "", "", "", "", "",
      "", "", "", "", "", "", "",
      "char",
      "", "", "", "", "", "", "",
      "checked"
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

#include "config.h"
#if ENABLE_JAVA_MODE
/* ANSI-C code produced by gperf version 2.7.2 */
/* Command-line: gperf -LANSI-C -ac -k 1,2 -N is_java_keyword fontlock_java.gperf  */

#define TOTAL_KEYWORDS 48
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 12
#define MIN_HASH_VALUE 3
#define MAX_HASH_VALUE 97
/* maximum key range = 95, duplicates = 0 */

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
      98, 98, 98, 98, 98, 98, 98, 98, 98, 98,
      98, 98, 98, 98, 98, 98, 98, 98, 98, 98,
      98, 98, 98, 98, 98, 98, 98, 98, 98, 98,
      98, 98, 98, 98, 98, 98, 98, 98, 98, 98,
      98, 98, 98, 98, 98, 98, 98, 98, 98, 98,
      98, 98, 98, 98, 98, 98, 98, 98, 98, 98,
      98, 98, 98, 98, 98, 98, 98, 98, 98, 98,
      98, 98, 98, 98, 98, 98, 98, 98, 98, 98,
      98, 98, 98, 98, 98, 98, 98, 98, 98, 98,
      98, 98, 98, 98, 98, 98, 98, 15, 35, 45,
      60, 30, 20, 20,  0,  0, 98, 98, 10, 20,
      60,  0, 50, 98,  0, 26,  0, 15, 25,  5,
      10, 40, 98, 98, 98, 98, 98, 98, 98, 98,
      98, 98, 98, 98, 98, 98, 98, 98, 98, 98,
      98, 98, 98, 98, 98, 98, 98, 98, 98, 98,
      98, 98, 98, 98, 98, 98, 98, 98, 98, 98,
      98, 98, 98, 98, 98, 98, 98, 98, 98, 98,
      98, 98, 98, 98, 98, 98, 98, 98, 98, 98,
      98, 98, 98, 98, 98, 98, 98, 98, 98, 98,
      98, 98, 98, 98, 98, 98, 98, 98, 98, 98,
      98, 98, 98, 98, 98, 98, 98, 98, 98, 98,
      98, 98, 98, 98, 98, 98, 98, 98, 98, 98,
      98, 98, 98, 98, 98, 98, 98, 98, 98, 98,
      98, 98, 98, 98, 98, 98, 98, 98, 98, 98,
      98, 98, 98, 98, 98, 98, 98, 98, 98, 98,
      98, 98, 98, 98, 98, 98
    };
  return len + asso_values[(unsigned char)str[1]] + asso_values[(unsigned char)str[0]];
}

#ifdef __GNUC__
__inline
#endif
const char *
is_java_keyword (register const char *str, register unsigned int len)
{
  static const char * wordlist[] =
    {
      "", "", "",
      "try",
      "this",
      "throw",
      "throws",
      "", "",
      "transient",
      "while",
      "", "", "",
      "long",
      "", "", "", "", "", "", "",
      "if",
      "for",
      "goto",
      "final",
      "import",
      "finally",
      "",
      "void",
      "implements",
      "short",
      "static",
      "volatile",
      "strictfp",
      "float",
      "return",
      "switch",
      "", "",
      "break",
      "",
      "boolean",
      "",
      "else",
      "",
      "super",
      "extends",
      "",
      "char",
      "const",
      "", "",
      "continue",
      "", "", "",
      "private",
      "abstract",
      "protected",
      "class",
      "",
      "do",
      "int",
      "case",
      "catch",
      "double",
      "", "",
      "interface",
      "instanceof",
      "public",
      "package",
      "", "", "", "", "",
      "synchronized",
      "byte",
      "",
      "native",
      "", "", "", "", "", "", "", "", "",
      "", "",
      "new",
      "", "", "",
      "default"
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

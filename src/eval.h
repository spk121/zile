#include "lists.h"

typedef
le *
(*eval_cb)
  (
   const int argc,
   le * branch
   );
typedef struct evalLookupNode {
  char    * word;
  eval_cb   callback;
} evalLookupNode;

le * evaluateBranch(le * trybranch);
le * evaluateNode(le * node);
le * evaluateDefun( le * fcn, le * params );

int countNodes(le * branch);

int evalCastLeToInt( const le * levalue );
le * evalCastIntToLe( int intvalue );

le * eval_cb_nothing( const int argc, le * branch );

enum cumefcn { C_NONE, C_ADD, C_SUBTRACT, C_MULTIPLY, C_DIVIDE };
int
eval_cume_helper(
                 enum cumefcn function,
                 int value,
                 le * branch
                 ) ;
le * eval_cb_add( const int argc, le * branch );
le * eval_cb_subtract( const int argc, le * branch );
le * eval_cb_multiply( const int argc, le * branch );
le * eval_cb_divide( const int argc, le * branch );
le * eval_cb_oneplus( const int argc, le * branch );
le * eval_cb_oneminus( const int argc, le * branch );
le * eval_cb_modulus( const int argc, le * branch );

le * eval_cb_lt( const int argc, le * branch );
le * eval_cb_lt_eq( const int argc, le * branch );
le * eval_cb_gt( const int argc, le * branch );
le * eval_cb_gt_eq( const int argc, le * branch );
le * eval_cb_eqsign( const int argc, le * branch );

le * eval_cb_and( const int argc, le * branch );
le * eval_cb_or( const int argc, le * branch );
le * eval_cb_not( const int argc, le * branch );

le * eval_cb_atom( const int argc, le * branch );
le * eval_cb_car( const int argc, le * branch );
le * eval_cb_cdr( const int argc, le * branch );
le * eval_cb_cons( const int argc, le * branch );
le * eval_cb_list( const int argc, le * branch );
int eval_cb_lists_same( le * list1, le * list2 );
le * eval_cb_equal( const int argc, le * branch );


le * eval_cb_if( const int argc, le * branch );
enum whenunless { WU_WHEN, WU_UNLESS };
le *
eval_cb_whenunless_helper(
                          enum whenunless which,
                          const int argc, 
                          le * branch
                          );
le * eval_cb_unless( const int argc, le * branch );
le * eval_cb_when( const int argc, le * branch );
le * eval_cb_cond( const int argc, le * branch );
le * eval_cb_select( const int argc, le * branch );

le * eval_cb_princ( const int argc, le * branch );
le * eval_cb_terpri( const int argc, le * branch );

le * eval_cb_eval( const int argc, le * branch );
le * eval_cb_prog( const int argc, le * branch, int returnit );
le * eval_cb_prog1( const int argc, le * branch );
le * eval_cb_prog2( const int argc, le * branch );
le * eval_cb_progn( const int argc, le * branch );

enum setfcn { S_SET, S_SETQ };
le * eval_cb_set_helper( 
                        enum setfcn function,
                        const int argc, 
                        le * branch
                        );
le * eval_cb_set( const int argc, le * branch );
le * eval_cb_setq( const int argc, le * branch );
le * eval_cb_enum( const int argc, le * branch );

le * eval_cb_defun( const int argc, le * branch );

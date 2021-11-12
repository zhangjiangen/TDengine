/*
** 2000-05-29
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** Driver template for the LEMON parser generator.
**
** The "lemon" program processes an LALR(1) input grammar file, then uses
** this template to construct a parser.  The "lemon" program inserts text
** at each "%%" line.  Also, any "P-a-r-s-e" identifer prefix (without the
** interstitial "-" characters) contained in this template is changed into
** the value of the %name directive from the grammar.  Otherwise, the content
** of this template is copied straight through into the generate parser
** source file.
**
** The following is the concatenation of all %include directives from the
** input grammar file:
*/
#include <stdio.h>
#include <assert.h>
/************ Begin %include sections from the grammar ************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include "qSqlparser.h"
#include "tcmdtype.h"
#include "ttoken.h"
#include "ttokendef.h"
#include "tutil.h"
#include "tvariant.h"
/**************** End of %include directives **********************************/
/* These constants specify the various numeric values for terminal symbols
** in a format understandable to "makeheaders".  This section is blank unless
** "lemon" is run with the "-m" command-line option.
***************** Begin makeheaders token definitions *************************/
/**************** End makeheaders token definitions ***************************/

/* The next sections is a series of control #defines.
** various aspects of the generated parser.
**    YYCODETYPE         is the data type used to store the integer codes
**                       that represent terminal and non-terminal symbols.
**                       "unsigned char" is used if there are fewer than
**                       256 symbols.  Larger types otherwise.
**    YYNOCODE           is a number of type YYCODETYPE that is not used for
**                       any terminal or nonterminal symbol.
**    YYFALLBACK         If defined, this indicates that one or more tokens
**                       (also known as: "terminal symbols") have fall-back
**                       values which should be used if the original symbol
**                       would not parse.  This permits keywords to sometimes
**                       be used as identifiers, for example.
**    YYACTIONTYPE       is the data type used for "action codes" - numbers
**                       that indicate what to do in response to the next
**                       token.
**    ParseTOKENTYPE     is the data type used for minor type for terminal
**                       symbols.  Background: A "minor type" is a semantic
**                       value associated with a terminal or non-terminal
**                       symbols.  For example, for an "ID" terminal symbol,
**                       the minor type might be the name of the identifier.
**                       Each non-terminal can have a different minor type.
**                       Terminal symbols all have the same minor type, though.
**                       This macros defines the minor type for terminal 
**                       symbols.
**    YYMINORTYPE        is the data type used for all minor types.
**                       This is typically a union of many types, one of
**                       which is ParseTOKENTYPE.  The entry in the union
**                       for terminal symbols is called "yy0".
**    YYSTACKDEPTH       is the maximum depth of the parser's stack.  If
**                       zero the stack is dynamically sized using realloc()
**    ParseARG_SDECL     A static variable declaration for the %extra_argument
**    ParseARG_PDECL     A parameter declaration for the %extra_argument
**    ParseARG_PARAM     Code to pass %extra_argument as a subroutine parameter
**    ParseARG_STORE     Code to store %extra_argument into yypParser
**    ParseARG_FETCH     Code to extract %extra_argument from yypParser
**    ParseCTX_*         As ParseARG_ except for %extra_context
**    YYERRORSYMBOL      is the code number of the error symbol.  If not
**                       defined, then do no error processing.
**    YYNSTATE           the combined number of states.
**    YYNRULE            the number of rules in the grammar
**    YYNTOKEN           Number of terminal symbols
**    YY_MAX_SHIFT       Maximum value for shift actions
**    YY_MIN_SHIFTREDUCE Minimum value for shift-reduce actions
**    YY_MAX_SHIFTREDUCE Maximum value for shift-reduce actions
**    YY_ERROR_ACTION    The yy_action[] code for syntax error
**    YY_ACCEPT_ACTION   The yy_action[] code for accept
**    YY_NO_ACTION       The yy_action[] code for no-op
**    YY_MIN_REDUCE      Minimum value for reduce actions
**    YY_MAX_REDUCE      Maximum value for reduce actions
*/
#ifndef INTERFACE
# define INTERFACE 1
#endif
/************* Begin control #defines *****************************************/
#define YYCODETYPE unsigned short int
#define YYNOCODE 282
#define YYACTIONTYPE unsigned short int
#define ParseTOKENTYPE SStrToken
typedef union {
  int yyinit;
  ParseTOKENTYPE yy0;
  tVariant yy2;
  SCreateDbInfo yy10;
  int32_t yy40;
  SSqlNode* yy68;
  SCreatedTableInfo yy72;
  SLimitVal yy114;
  SCreateTableSql* yy170;
  SIntervalVal yy280;
  int yy281;
  SSessionWindowVal yy295;
  SArray* yy345;
  tSqlExpr* yy418;
  SCreateAcctInfo yy427;
  SWindowStateVal yy432;
  SRelationInfo* yy484;
  TAOS_FIELD yy487;
  int64_t yy525;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#define ParseARG_SDECL SSqlInfo* pInfo;
#define ParseARG_PDECL ,SSqlInfo* pInfo
#define ParseARG_PARAM ,pInfo
#define ParseARG_FETCH SSqlInfo* pInfo=yypParser->pInfo;
#define ParseARG_STORE yypParser->pInfo=pInfo;
#define ParseCTX_SDECL
#define ParseCTX_PDECL
#define ParseCTX_PARAM
#define ParseCTX_FETCH
#define ParseCTX_STORE
#define YYFALLBACK 1
#define YYNSTATE             369
#define YYNRULE              296
#define YYNTOKEN             198
#define YY_MAX_SHIFT         368
#define YY_MIN_SHIFTREDUCE   579
#define YY_MAX_SHIFTREDUCE   874
#define YY_ERROR_ACTION      875
#define YY_ACCEPT_ACTION     876
#define YY_NO_ACTION         877
#define YY_MIN_REDUCE        878
#define YY_MAX_REDUCE        1173
/************* End control #defines *******************************************/
#define YY_NLOOKAHEAD ((int)(sizeof(yy_lookahead)/sizeof(yy_lookahead[0])))

/* Define the yytestcase() macro to be a no-op if is not already defined
** otherwise.
**
** Applications can choose to define yytestcase() in the %include section
** to a macro that can assist in verifying code coverage.  For production
** code the yytestcase() macro should be turned off.  But it is useful
** for testing.
*/
#ifndef yytestcase
# define yytestcase(X)
#endif


/* Next are the tables used to determine what action to take based on the
** current state and lookahead token.  These tables are used to implement
** functions that take a state number and lookahead value and return an
** action integer.  
**
** Suppose the action integer is N.  Then the action is determined as
** follows
**
**   0 <= N <= YY_MAX_SHIFT             Shift N.  That is, push the lookahead
**                                      token onto the stack and goto state N.
**
**   N between YY_MIN_SHIFTREDUCE       Shift to an arbitrary state then
**     and YY_MAX_SHIFTREDUCE           reduce by rule N-YY_MIN_SHIFTREDUCE.
**
**   N == YY_ERROR_ACTION               A syntax error has occurred.
**
**   N == YY_ACCEPT_ACTION              The parser accepts its input.
**
**   N == YY_NO_ACTION                  No such action.  Denotes unused
**                                      slots in the yy_action[] table.
**
**   N between YY_MIN_REDUCE            Reduce by rule N-YY_MIN_REDUCE
**     and YY_MAX_REDUCE
**
** The action table is constructed as a single large table named yy_action[].
** Given state S and lookahead X, the action is computed as either:
**
**    (A)   N = yy_action[ yy_shift_ofst[S] + X ]
**    (B)   N = yy_default[S]
**
** The (A) formula is preferred.  The B formula is used instead if
** yy_lookahead[yy_shift_ofst[S]+X] is not equal to X.
**
** The formulas above are for computing the action when the lookahead is
** a terminal symbol.  If the lookahead is a non-terminal (as occurs after
** a reduce action) then the yy_reduce_ofst[] array is used in place of
** the yy_shift_ofst[] array.
**
** The following are the tables generated in this section:
**
**  yy_action[]        A single table containing all actions.
**  yy_lookahead[]     A table containing the lookahead for each entry in
**                     yy_action.  Used to detect hash collisions.
**  yy_shift_ofst[]    For each state, the offset into yy_action for
**                     shifting terminals.
**  yy_reduce_ofst[]   For each state, the offset into yy_action for
**                     shifting non-terminals after a reduce.
**  yy_default[]       Default action for each state.
**
*********** Begin parsing tables **********************************************/
#define YY_ACTTAB_COUNT (764)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */   251,  631,  631,  631,  631,  101,   23,  250,  255,  632,
 /*    10 */   632,  632,  632,   57,   58,  240,   61,   62, 1060,  213,
 /*    20 */   254,   51, 1038,   60,  324,   65,   63,   66,   64, 1004,
 /*    30 */  1149, 1002, 1003,   56,   55,  210, 1005,   54,   53,   52,
 /*    40 */  1006, 1022, 1007, 1008,  165,  876,  368,  580,  581,  582,
 /*    50 */   583,  584,  585,  586,  587,  588,  589,  590,  591,  592,
 /*    60 */   593,  367,  815,  213,  235,   57,   58,  667,   61,   62,
 /*    70 */   213,  213,  254,   51, 1150,   60,  324,   65,   63,   66,
 /*    80 */    64, 1150, 1150,  795,   29,   56,   55,   83, 1057,   54,
 /*    90 */    53,   52,   57,   58,  246,   61,   62, 1051,  816,  254,
 /*   100 */    51, 1038,   60,  324,   65,   63,   66,   64,   54,   53,
 /*   110 */    52,   89,   56,   55,  276,  354,   54,   53,   52,   57,
 /*   120 */    59, 1098,   61,   62,  234,  366,  254,   51,   98,   60,
 /*   130 */   324,   65,   63,   66,   64,  819,  813,  822,  165,   56,
 /*   140 */    55,  165,  794,   54,   53,   52,   58,  248,   61,   62,
 /*   150 */    45,  322,  254,   51, 1038,   60,  324,   65,   63,   66,
 /*   160 */    64, 1017, 1018,   35, 1021,   56,   55,  778,  779,   54,
 /*   170 */    53,   52,   44,  320,  361,  360,  319,  318,  317,  359,
 /*   180 */   316,  315,  314,  358,  313,  357,  356,  996,  984,  985,
 /*   190 */   986,  987,  988,  989,  990,  991,  992,  993,  994,  995,
 /*   200 */   997,  998,   61,   62,   24,   38,  254,   51,   86,   60,
 /*   210 */   324,   65,   63,   66,   64, 1099,  297,  295,   94,   56,
 /*   220 */    55,  174,  216,   54,   53,   52,  253,  828, 1051,  222,
 /*   230 */   817,  158,  820, 1028,  823,  140,  139,  138,  221,  285,
 /*   240 */   253,  828,  329,   89,  817,  237,  820,  279,  823,  236,
 /*   250 */    38,  126,   65,   63,   66,   64, 1035,  826,  232,  233,
 /*   260 */    56,   55,  325,  354,   54,   53,   52,    5,   41,  184,
 /*   270 */    38,  257,  232,  233,  183,  107,  112,  103,  111,  247,
 /*   280 */   743,  715,   45,  740,   38,  741,  818,  742,  821,  124,
 /*   290 */   118,  129,  262,  309,  244, 1020,  128,   38,  134,  137,
 /*   300 */   127, 1035,  283,  282,  275,   96,   79,  131,   67,  344,
 /*   310 */   343,  259,  260,  229,  245,  165,  204,  202,  200,   84,
 /*   320 */    38, 1035,   67,  199,  144,  143,  142,  141,  333,   44,
 /*   330 */  1051,  361,  360,   56,   55, 1035,  359,   54,   53,   52,
 /*   340 */   358,  334,  357,  356,   14,  829,  824,  238, 1035,  258,
 /*   350 */    38,  256,  825,  332,  331,  365,  364,  149,   38,  829,
 /*   360 */   824,  927,   38,   38,  335,   38,  825,   38,  194,  759,
 /*   370 */   264, 1035,  261,  268,  339,  338,   80,  263,  100,   81,
 /*   380 */   322,  263,  272,  271,  155,  153,  152,  180,  937,  928,
 /*   390 */   263,  181,   95,   87,  336,  194,  194,  756,  744,  745,
 /*   400 */  1036, 1035,  340,  362,  965,   71,  341,  342,  827, 1035,
 /*   410 */  1019,  346,  277, 1035, 1035,  775, 1034,  785, 1035,    1,
 /*   420 */   182,    3,  195,  786,   74,    9,   39,  725,  301,  160,
 /*   430 */   727,   68,  279,  303,  726,   34,  252,   26,  849,  830,
 /*   440 */   326,   39,   39,  211,   68,  630,   78,   99,   68,   72,
 /*   450 */   136,  135,   25,   25,   16,  217,   15,  763,    6,   25,
 /*   460 */   117,   18,  116,   17, 1146,  748,   75,  749,  304,  746,
 /*   470 */    20,  747,   19,  123,  273,  122, 1145,   22,  714,   21,
 /*   480 */   176, 1144,  230,  231,  214,  215,  218, 1037,  212,  219,
 /*   490 */   220,  224,  225, 1169,  226, 1161, 1109, 1108,  242,  223,
 /*   500 */  1105,  209, 1104,  243,  345,  156,  157,   48,  154, 1033,
 /*   510 */   175, 1059, 1070, 1049, 1091, 1067, 1068, 1052,  280, 1072,
 /*   520 */   159,  164, 1090,  284,  291, 1029,  278,  310,  177,  173,
 /*   530 */   166, 1027,  168,  774,  169,  178,  179,  942,  306,  307,
 /*   540 */   239,  308,  311,  312,  286,  832,   46,  298,  207,   42,
 /*   550 */    76,  288,  323,   73,  936,  167,   50,  330, 1168,  114,
 /*   560 */  1167, 1164,  294,  185,  296,  337, 1160,  120, 1159, 1156,
 /*   570 */   186,  962,  292,   43,   40,   47,  171,  208,  924,  130,
 /*   580 */   922,  132,  133,  920,  919,  265,  197,  198,  916,  915,
 /*   590 */   914,  913,  912,  911,  910,  201,  203,  907,  905,  903,
 /*   600 */   901,  205,  898,  206,  290, 1031,   85,   90,  287,  289,
 /*   610 */  1092,   49,   82,  355,  125,  347,  348,  349,   77,  249,
 /*   620 */   350,  305,  351,  352,  353,  363,  227,  874,  266,  267,
 /*   630 */   228,  108,  941,  940,  109,  873,  269,  270,  872,  855,
 /*   640 */   274,  854,  918,  279,  300,  917,  909,  189,  193,  145,
 /*   650 */   963,  187,  188,  190,  191,  146,  192,  147,  908,    2,
 /*   660 */   148, 1000,    4,  299,  900,  964,   10,  899,   88,  172,
 /*   670 */   170,   33,  751,   30,  281, 1010,   91,  776,  161,  163,
 /*   680 */   787,  162,  241,  781,   92,   31,  783,   93,  293,   11,
 /*   690 */    32,   12,   13,   97,   27,  302,   28,  100,  102,  105,
 /*   700 */    36,  104,  645,   37,  106,  680,  678,  677,  676,  674,
 /*   710 */   673,  672,  669,  635,  321,  110,  327,    7,  328,  831,
 /*   720 */   833,    8,  113,  115,   69,   70,  717,   39,  716,  713,
 /*   730 */   661,  659,  119,  651,  121,  657,  653,  655,  649,  647,
 /*   740 */   683,  682,  681,  679,  675,  671,  670,  196,  633,  597,
 /*   750 */   150,  595,  878,  877,  877,  877,  877,  877,  877,  877,
 /*   760 */   877,  877,  877,  151,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */   207,    1,    1,    1,    1,  208,  270,  207,  207,    9,
 /*    10 */     9,    9,    9,   13,   14,  246,   16,   17,  201,  270,
 /*    20 */    20,   21,  253,   23,   24,   25,   26,   27,   28,  224,
 /*    30 */   281,  226,  227,   33,   34,  270,  231,   37,   38,   39,
 /*    40 */   235,  244,  237,  238,  201,  198,  199,   45,   46,   47,
 /*    50 */    48,   49,   50,   51,   52,   53,   54,   55,   56,   57,
 /*    60 */    58,   59,    1,  270,   62,   13,   14,    5,   16,   17,
 /*    70 */   270,  270,   20,   21,  281,   23,   24,   25,   26,   27,
 /*    80 */    28,  281,  281,   77,   83,   33,   34,   87,  271,   37,
 /*    90 */    38,   39,   13,   14,  246,   16,   17,  250,   37,   20,
 /*   100 */    21,  253,   23,   24,   25,   26,   27,   28,   37,   38,
 /*   110 */    39,   83,   33,   34,  267,   91,   37,   38,   39,   13,
 /*   120 */    14,  278,   16,   17,  200,  201,   20,   21,  208,   23,
 /*   130 */    24,   25,   26,   27,   28,    5,   84,    7,  201,   33,
 /*   140 */    34,  201,  136,   37,   38,   39,   14,  246,   16,   17,
 /*   150 */   122,   85,   20,   21,  253,   23,   24,   25,   26,   27,
 /*   160 */    28,  241,  242,  243,  244,   33,   34,  128,  129,   37,
 /*   170 */    38,   39,   99,  100,  101,  102,  103,  104,  105,  106,
 /*   180 */   107,  108,  109,  110,  111,  112,  113,  224,  225,  226,
 /*   190 */   227,  228,  229,  230,  231,  232,  233,  234,  235,  236,
 /*   200 */   237,  238,   16,   17,   44,  201,   20,   21,   84,   23,
 /*   210 */    24,   25,   26,   27,   28,  278,  276,  280,  278,   33,
 /*   220 */    34,  257,   62,   37,   38,   39,    1,    2,  250,   69,
 /*   230 */     5,  201,    7,  201,    9,   75,   76,   77,   78,  275,
 /*   240 */     1,    2,   82,   83,    5,  267,    7,  123,    9,  245,
 /*   250 */   201,   79,   25,   26,   27,   28,  252,  127,   33,   34,
 /*   260 */    33,   34,   37,   91,   37,   38,   39,   63,   64,   65,
 /*   270 */   201,   69,   33,   34,   70,   71,   72,   73,   74,  247,
 /*   280 */     2,    5,  122,    5,  201,    7,    5,    9,    7,   63,
 /*   290 */    64,   65,   69,   89,  245,    0,   70,  201,   72,   73,
 /*   300 */    74,  252,  272,  273,  144,  254,  146,   81,   83,   33,
 /*   310 */    34,   33,   34,  153,  245,  201,   63,   64,   65,  268,
 /*   320 */   201,  252,   83,   70,   71,   72,   73,   74,  245,   99,
 /*   330 */   250,  101,  102,   33,   34,  252,  106,   37,   38,   39,
 /*   340 */   110,  245,  112,  113,   83,  120,  121,  267,  252,  147,
 /*   350 */   201,  149,  127,  151,  152,   66,   67,   68,  201,  120,
 /*   360 */   121,  206,  201,  201,  245,  201,  127,  201,  213,   37,
 /*   370 */   147,  252,  149,  145,  151,  152,  208,  201,  117,  118,
 /*   380 */    85,  201,  154,  155,   63,   64,   65,  211,  206,  206,
 /*   390 */   201,  211,  278,   84,  245,  213,  213,   98,  120,  121,
 /*   400 */   211,  252,  245,  222,  223,   98,  245,  245,  127,  252,
 /*   410 */   242,  245,   84,  252,  252,   84,  252,   84,  252,  209,
 /*   420 */   210,  204,  205,   84,   98,  126,   98,   84,   84,   98,
 /*   430 */    84,   98,  123,   84,   84,   83,   61,   98,   84,   84,
 /*   440 */    15,   98,   98,  270,   98,   84,   83,   98,   98,  142,
 /*   450 */    79,   80,   98,   98,  148,  270,  150,  125,   83,   98,
 /*   460 */   148,  148,  150,  150,  270,    5,  140,    7,  116,    5,
 /*   470 */   148,    7,  150,  148,  201,  150,  270,  148,  115,  150,
 /*   480 */   248,  270,  270,  270,  270,  270,  270,  253,  270,  270,
 /*   490 */   270,  270,  270,  253,  270,  253,  240,  240,  240,  270,
 /*   500 */   240,  270,  240,  240,  240,  201,  201,  269,   61,  201,
 /*   510 */   255,  201,  201,  266,  279,  201,  201,  250,  250,  201,
 /*   520 */   201,  201,  279,  274,  201,  250,  202,   90,  201,  258,
 /*   530 */   265,  201,  263,  127,  262,  201,  201,  201,  201,  201,
 /*   540 */   274,  201,  201,  201,  274,  120,  201,  134,  201,  201,
 /*   550 */   139,  274,  201,  141,  201,  264,  138,  201,  201,  201,
 /*   560 */   201,  201,  132,  201,  137,  201,  201,  201,  201,  201,
 /*   570 */   201,  201,  131,  201,  201,  201,  260,  201,  201,  201,
 /*   580 */   201,  201,  201,  201,  201,  201,  201,  201,  201,  201,
 /*   590 */   201,  201,  201,  201,  201,  201,  201,  201,  201,  201,
 /*   600 */   201,  201,  201,  201,  130,  202,  202,  202,  133,  202,
 /*   610 */   202,  143,  119,  114,   97,   96,   51,   93,  202,  202,
 /*   620 */    95,  202,   55,   94,   92,   85,  202,    5,  156,    5,
 /*   630 */   202,  208,  212,  212,  208,    5,  156,    5,    5,  101,
 /*   640 */   145,  100,  202,  123,  116,  202,  202,  215,  214,  203,
 /*   650 */   221,  220,  219,  218,  216,  203,  217,  203,  202,  209,
 /*   660 */   203,  239,  204,  249,  202,  223,   83,  202,  124,  259,
 /*   670 */   261,  256,   84,   83,   98,  239,   98,   84,   83,   98,
 /*   680 */    84,   83,    1,   84,   83,   98,   84,   83,   83,  135,
 /*   690 */    98,  135,   83,   87,   83,  116,   83,  117,   79,   71,
 /*   700 */    88,   87,    5,   88,   87,    9,    5,    5,    5,    5,
 /*   710 */     5,    5,    5,   86,   15,   79,   24,   83,   59,   84,
 /*   720 */   120,   83,  150,  150,   16,   16,    5,   98,    5,   84,
 /*   730 */     5,    5,  150,    5,  150,    5,    5,    5,    5,    5,
 /*   740 */     5,    5,    5,    5,    5,    5,    5,   98,   86,   61,
 /*   750 */    21,   60,    0,  282,  282,  282,  282,  282,  282,  282,
 /*   760 */   282,  282,  282,   21,  282,  282,  282,  282,  282,  282,
 /*   770 */   282,  282,  282,  282,  282,  282,  282,  282,  282,  282,
 /*   780 */   282,  282,  282,  282,  282,  282,  282,  282,  282,  282,
 /*   790 */   282,  282,  282,  282,  282,  282,  282,  282,  282,  282,
 /*   800 */   282,  282,  282,  282,  282,  282,  282,  282,  282,  282,
 /*   810 */   282,  282,  282,  282,  282,  282,  282,  282,  282,  282,
 /*   820 */   282,  282,  282,  282,  282,  282,  282,  282,  282,  282,
 /*   830 */   282,  282,  282,  282,  282,  282,  282,  282,  282,  282,
 /*   840 */   282,  282,  282,  282,  282,  282,  282,  282,  282,  282,
 /*   850 */   282,  282,  282,  282,  282,  282,  282,  282,  282,  282,
 /*   860 */   282,  282,  282,  282,  282,  282,  282,  282,  282,  282,
 /*   870 */   282,  282,  282,  282,  282,  282,  282,  282,  282,  282,
 /*   880 */   282,  282,  282,  282,  282,  282,  282,  282,  282,  282,
 /*   890 */   282,  282,  282,  282,  282,  282,  282,  282,  282,  282,
 /*   900 */   282,  282,  282,  282,  282,  282,  282,  282,  282,  282,
 /*   910 */   282,  282,  282,  282,  282,  282,  282,  282,  282,  282,
 /*   920 */   282,  282,  282,  282,  282,  282,  282,  282,  282,  282,
 /*   930 */   282,  282,  282,  282,  282,  282,  282,  282,  282,  282,
 /*   940 */   282,  282,  282,  282,  282,  282,  282,  282,  282,  282,
 /*   950 */   282,  282,  282,  282,  282,  282,  282,  282,  282,  282,
 /*   960 */   282,  282,
};
#define YY_SHIFT_COUNT    (368)
#define YY_SHIFT_MIN      (0)
#define YY_SHIFT_MAX      (752)
static const unsigned short int yy_shift_ofst[] = {
 /*     0 */   160,   73,   73,  230,  230,   66,  225,  239,  239,    1,
 /*    10 */     3,    3,    3,    3,    3,    3,    3,    3,    3,    3,
 /*    20 */     3,    3,    3,    0,    2,  239,  278,  278,  278,   28,
 /*    30 */    28,    3,    3,   39,    3,  295,    3,    3,    3,    3,
 /*    40 */   172,   66,   24,   24,   62,  764,  764,  764,  239,  239,
 /*    50 */   239,  239,  239,  239,  239,  239,  239,  239,  239,  239,
 /*    60 */   239,  239,  239,  239,  239,  239,  239,  239,  278,  278,
 /*    70 */   278,  276,  276,  276,  276,  276,  276,  261,  276,    3,
 /*    80 */     3,    3,    3,    3,  332,    3,    3,    3,   28,   28,
 /*    90 */     3,    3,    3,    3,    6,    6,  299,   28,    3,    3,
 /*   100 */     3,    3,    3,    3,    3,    3,    3,    3,    3,    3,
 /*   110 */     3,    3,    3,    3,    3,    3,    3,    3,    3,    3,
 /*   120 */     3,    3,    3,    3,    3,    3,    3,    3,    3,    3,
 /*   130 */     3,    3,    3,    3,    3,    3,    3,    3,    3,    3,
 /*   140 */     3,    3,    3,    3,    3,    3,    3,    3,    3,    3,
 /*   150 */     3,    3,    3,    3,    3,    3,  447,  447,  447,  447,
 /*   160 */   406,  406,  406,  406,  447,  447,  411,  412,  413,  418,
 /*   170 */   427,  430,  441,  474,  475,  468,  493,  447,  447,  447,
 /*   180 */   437,  437,  499,   66,   66,  447,  447,  517,  519,  565,
 /*   190 */   524,  525,  567,  529,  532,  499,   62,  447,  447,  540,
 /*   200 */   540,  447,  540,  447,  540,  447,  447,  764,  764,   52,
 /*   210 */    79,   79,  106,   79,  132,  186,  204,  227,  227,  227,
 /*   220 */   227,  226,  253,  300,  300,  300,  300,  202,  223,  228,
 /*   230 */    71,   71,  130,  281,  289,  321,  328,  124,  309,  331,
 /*   240 */   333,  339,  307,  326,  343,  344,  346,  349,  350,  352,
 /*   250 */   354,  355,   61,  375,  425,  361,  306,  312,  313,  460,
 /*   260 */   464,  322,  325,  363,  329,  371,  622,  472,  624,  630,
 /*   270 */   480,  632,  633,  538,  541,  495,  520,  528,  583,  544,
 /*   280 */   588,  590,  576,  578,  593,  595,  596,  598,  599,  581,
 /*   290 */   601,  602,  604,  681,  605,  587,  554,  592,  556,  606,
 /*   300 */   609,  528,  611,  579,  613,  580,  619,  612,  614,  628,
 /*   310 */   697,  615,  617,  696,  701,  702,  703,  704,  705,  706,
 /*   320 */   707,  627,  699,  636,  634,  635,  600,  638,  692,  659,
 /*   330 */   708,  572,  573,  629,  629,  629,  629,  709,  582,  584,
 /*   340 */   629,  629,  629,  721,  723,  645,  629,  725,  726,  728,
 /*   350 */   730,  731,  732,  733,  734,  735,  736,  737,  738,  739,
 /*   360 */   740,  741,  649,  662,  729,  742,  688,  691,  752,
};
#define YY_REDUCE_COUNT (208)
#define YY_REDUCE_MIN   (-264)
#define YY_REDUCE_MAX   (465)
static const short yy_reduce_ofst[] = {
 /*     0 */  -153,  -37,  -37, -195, -195,  -80, -207, -200, -199,   30,
 /*    10 */     4,  -63,  -60,   49,   69,   83,   96,  119,  149,  157,
 /*    20 */   161,  162,  166, -183,  -76, -251, -231, -152,  -99,  -22,
 /*    30 */    80, -157,  114,  -36,   32, -203,  176,  180,  189,  164,
 /*    40 */   155,  168,  182,  183,  181,   51,  210,  217, -264, -235,
 /*    50 */   173,  185,  194,  206,  211,  212,  213,  214,  215,  216,
 /*    60 */   218,  219,  220,  221,  222,  224,  229,  231,  234,  240,
 /*    70 */   242,  256,  257,  258,  260,  262,  263,  232,  264,  273,
 /*    80 */   304,  305,  308,  310,  238,  311,  314,  315,  267,  268,
 /*    90 */   318,  319,  320,  323,  235,  243,  255,  275,  327,  330,
 /*   100 */   334,  335,  336,  337,  338,  340,  341,  342,  345,  347,
 /*   110 */   348,  351,  353,  356,  357,  358,  359,  360,  362,  364,
 /*   120 */   365,  366,  367,  368,  369,  370,  372,  373,  374,  376,
 /*   130 */   377,  378,  379,  380,  381,  382,  383,  384,  385,  386,
 /*   140 */   387,  388,  389,  390,  391,  392,  393,  394,  395,  396,
 /*   150 */   397,  398,  399,  400,  401,  402,  324,  403,  404,  405,
 /*   160 */   249,  266,  270,  277,  407,  408,  247,  265,  291,  269,
 /*   170 */   272,  409,  316,  410,  271,  415,  414,  416,  417,  419,
 /*   180 */   420,  421,  422,  423,  426,  424,  428,  429,  431,  433,
 /*   190 */   432,  435,  438,  439,  434,  436,  442,  440,  443,  446,
 /*   200 */   452,  444,  454,  456,  457,  462,  465,  450,  458,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   875,  999,  938, 1009,  925,  935, 1152, 1152, 1152,  875,
 /*    10 */   875,  875,  875,  875,  875,  875,  875,  875,  875,  875,
 /*    20 */   875,  875,  875, 1061,  895, 1152,  875,  875,  875,  875,
 /*    30 */   875,  875,  875, 1076,  875,  935,  875,  875,  875,  875,
 /*    40 */   945,  935,  945,  945,  875, 1056,  983, 1001,  875,  875,
 /*    50 */   875,  875,  875,  875,  875,  875,  875,  875,  875,  875,
 /*    60 */   875,  875,  875,  875,  875,  875,  875,  875,  875,  875,
 /*    70 */   875,  875,  875,  875,  875,  875,  875, 1030,  875,  875,
 /*    80 */   875,  875,  875,  875, 1063, 1069, 1066,  875,  875,  875,
 /*    90 */  1071,  875,  875,  875, 1095, 1095, 1054,  875,  875,  875,
 /*   100 */   875,  875,  875,  875,  875,  875,  875,  875,  875,  875,
 /*   110 */   875,  875,  875,  875,  875,  875,  875,  875,  875,  875,
 /*   120 */   875,  875,  875,  875,  875,  875,  875,  875,  875,  875,
 /*   130 */   923,  875,  921,  875,  875,  875,  875,  875,  875,  875,
 /*   140 */   875,  875,  875,  875,  875,  875,  875,  875,  875,  906,
 /*   150 */   875,  875,  875,  875,  875,  875,  897,  897,  897,  897,
 /*   160 */   875,  875,  875,  875,  897,  897, 1102, 1106, 1088, 1100,
 /*   170 */  1096, 1083, 1081, 1079, 1087, 1110, 1032,  897,  897,  897,
 /*   180 */   943,  943,  939,  935,  935,  897,  897,  961,  959,  957,
 /*   190 */   949,  955,  951,  953,  947,  926,  875,  897,  897,  933,
 /*   200 */   933,  897,  933,  897,  933,  897,  897,  983, 1001,  875,
 /*   210 */  1111, 1101,  875, 1151, 1141, 1140,  875, 1147, 1139, 1138,
 /*   220 */  1137,  875,  875, 1133, 1136, 1135, 1134,  875,  875,  875,
 /*   230 */  1143, 1142,  875,  875,  875,  875,  875,  875,  875,  875,
 /*   240 */   875,  875, 1107, 1103,  875,  875,  875,  875,  875,  875,
 /*   250 */   875,  875,  875, 1113,  875,  875,  875,  875,  875,  875,
 /*   260 */   875,  875,  875, 1011,  875,  875,  875,  875,  875,  875,
 /*   270 */   875,  875,  875,  875,  875,  875, 1053,  875,  875,  875,
 /*   280 */   875,  875, 1065, 1064,  875,  875,  875,  875,  875,  875,
 /*   290 */   875,  875,  875,  875,  875, 1097,  875, 1089,  875,  875,
 /*   300 */   875, 1023,  875,  875,  875,  875,  875,  875,  875,  875,
 /*   310 */   875,  875,  875,  875,  875,  875,  875,  875,  875,  875,
 /*   320 */   875,  875,  875,  875,  875,  875,  875,  875,  875,  875,
 /*   330 */   875,  875,  875, 1170, 1165, 1166, 1163,  875,  875,  875,
 /*   340 */  1162, 1157, 1158,  875,  875,  875, 1155,  875,  875,  875,
 /*   350 */   875,  875,  875,  875,  875,  875,  875,  875,  875,  875,
 /*   360 */   875,  875,  967,  875,  904,  902,  875,  893,  875,
};
/********** End of lemon-generated parsing tables *****************************/

/* The next table maps tokens (terminal symbols) into fallback tokens.  
** If a construct like the following:
** 
**      %fallback ID X Y Z.
**
** appears in the grammar, then ID becomes a fallback token for X, Y,
** and Z.  Whenever one of the tokens X, Y, or Z is input to the parser
** but it does not parse, the type of the token is changed to ID and
** the parse is retried before an error is thrown.
**
** This feature can be used, for example, to cause some keywords in a language
** to revert to identifiers if they keyword does not apply in the context where
** it appears.
*/
#ifdef YYFALLBACK
static const YYCODETYPE yyFallback[] = {
    0,  /*          $ => nothing */
    0,  /*         ID => nothing */
    1,  /*       BOOL => ID */
    1,  /*    TINYINT => ID */
    1,  /*   SMALLINT => ID */
    1,  /*    INTEGER => ID */
    1,  /*     BIGINT => ID */
    1,  /*      FLOAT => ID */
    1,  /*     DOUBLE => ID */
    1,  /*     STRING => ID */
    1,  /*  TIMESTAMP => ID */
    1,  /*     BINARY => ID */
    1,  /*      NCHAR => ID */
    0,  /*         OR => nothing */
    0,  /*        AND => nothing */
    0,  /*        NOT => nothing */
    0,  /*         EQ => nothing */
    0,  /*         NE => nothing */
    0,  /*     ISNULL => nothing */
    0,  /*    NOTNULL => nothing */
    0,  /*         IS => nothing */
    1,  /*       LIKE => ID */
    1,  /*       GLOB => ID */
    0,  /*    BETWEEN => nothing */
    0,  /*         IN => nothing */
    0,  /*         GT => nothing */
    0,  /*         GE => nothing */
    0,  /*         LT => nothing */
    0,  /*         LE => nothing */
    0,  /*     BITAND => nothing */
    0,  /*      BITOR => nothing */
    0,  /*     LSHIFT => nothing */
    0,  /*     RSHIFT => nothing */
    0,  /*       PLUS => nothing */
    0,  /*      MINUS => nothing */
    0,  /*     DIVIDE => nothing */
    0,  /*      TIMES => nothing */
    0,  /*       STAR => nothing */
    0,  /*      SLASH => nothing */
    0,  /*        REM => nothing */
    0,  /*     CONCAT => nothing */
    0,  /*     UMINUS => nothing */
    0,  /*      UPLUS => nothing */
    0,  /*     BITNOT => nothing */
    0,  /*       SHOW => nothing */
    0,  /*  DATABASES => nothing */
    0,  /*     TOPICS => nothing */
    0,  /*  FUNCTIONS => nothing */
    0,  /*     MNODES => nothing */
    0,  /*     DNODES => nothing */
    0,  /*   ACCOUNTS => nothing */
    0,  /*      USERS => nothing */
    0,  /*    MODULES => nothing */
    0,  /*    QUERIES => nothing */
    0,  /* CONNECTIONS => nothing */
    0,  /*    STREAMS => nothing */
    0,  /*  VARIABLES => nothing */
    0,  /*     SCORES => nothing */
    0,  /*     GRANTS => nothing */
    0,  /*     VNODES => nothing */
    1,  /*    IPTOKEN => ID */
    0,  /*        DOT => nothing */
    0,  /*     CREATE => nothing */
    0,  /*      TABLE => nothing */
    1,  /*     STABLE => ID */
    1,  /*   DATABASE => ID */
    0,  /*     TABLES => nothing */
    0,  /*    STABLES => nothing */
    0,  /*    VGROUPS => nothing */
    0,  /*       DROP => nothing */
    0,  /*      TOPIC => nothing */
    0,  /*   FUNCTION => nothing */
    0,  /*      DNODE => nothing */
    0,  /*       USER => nothing */
    0,  /*    ACCOUNT => nothing */
    0,  /*        USE => nothing */
    0,  /*   DESCRIBE => nothing */
    1,  /*       DESC => ID */
    0,  /*      ALTER => nothing */
    0,  /*       PASS => nothing */
    0,  /*  PRIVILEGE => nothing */
    0,  /*      LOCAL => nothing */
    0,  /*    COMPACT => nothing */
    0,  /*         LP => nothing */
    0,  /*         RP => nothing */
    0,  /*         IF => nothing */
    0,  /*     EXISTS => nothing */
    0,  /*         AS => nothing */
    0,  /* OUTPUTTYPE => nothing */
    0,  /*  AGGREGATE => nothing */
    0,  /*    BUFSIZE => nothing */
    0,  /*        PPS => nothing */
    0,  /*    TSERIES => nothing */
    0,  /*        DBS => nothing */
    0,  /*    STORAGE => nothing */
    0,  /*      QTIME => nothing */
    0,  /*      CONNS => nothing */
    0,  /*      STATE => nothing */
    0,  /*      COMMA => nothing */
    0,  /*       KEEP => nothing */
    0,  /*      CACHE => nothing */
    0,  /*    REPLICA => nothing */
    0,  /*     QUORUM => nothing */
    0,  /*       DAYS => nothing */
    0,  /*    MINROWS => nothing */
    0,  /*    MAXROWS => nothing */
    0,  /*     BLOCKS => nothing */
    0,  /*      CTIME => nothing */
    0,  /*        WAL => nothing */
    0,  /*      FSYNC => nothing */
    0,  /*       COMP => nothing */
    0,  /*  PRECISION => nothing */
    0,  /*     UPDATE => nothing */
    0,  /*  CACHELAST => nothing */
    0,  /* PARTITIONS => nothing */
    0,  /*   UNSIGNED => nothing */
    0,  /*       TAGS => nothing */
    0,  /*      USING => nothing */
    0,  /*         TO => nothing */
    0,  /*      SPLIT => nothing */
    1,  /*       NULL => ID */
    1,  /*        NOW => ID */
    0,  /*     SELECT => nothing */
    0,  /*      UNION => nothing */
    1,  /*        ALL => ID */
    0,  /*   DISTINCT => nothing */
    0,  /*       FROM => nothing */
    0,  /*   VARIABLE => nothing */
    0,  /*   INTERVAL => nothing */
    0,  /*      EVERY => nothing */
    0,  /*    SESSION => nothing */
    0,  /* STATE_WINDOW => nothing */
    0,  /*       FILL => nothing */
    0,  /*    SLIDING => nothing */
    0,  /*      ORDER => nothing */
    0,  /*         BY => nothing */
    1,  /*        ASC => ID */
    0,  /*      GROUP => nothing */
    0,  /*     HAVING => nothing */
    0,  /*      LIMIT => nothing */
    1,  /*     OFFSET => ID */
    0,  /*     SLIMIT => nothing */
    0,  /*    SOFFSET => nothing */
    0,  /*      WHERE => nothing */
    0,  /*      RESET => nothing */
    0,  /*      QUERY => nothing */
    0,  /*     SYNCDB => nothing */
    0,  /*        ADD => nothing */
    0,  /*     COLUMN => nothing */
    0,  /*     MODIFY => nothing */
    0,  /*        TAG => nothing */
    0,  /*     CHANGE => nothing */
    0,  /*        SET => nothing */
    0,  /*       KILL => nothing */
    0,  /* CONNECTION => nothing */
    0,  /*     STREAM => nothing */
    0,  /*      COLON => nothing */
    1,  /*      ABORT => ID */
    1,  /*      AFTER => ID */
    1,  /*     ATTACH => ID */
    1,  /*     BEFORE => ID */
    1,  /*      BEGIN => ID */
    1,  /*    CASCADE => ID */
    1,  /*    CLUSTER => ID */
    1,  /*   CONFLICT => ID */
    1,  /*       COPY => ID */
    1,  /*   DEFERRED => ID */
    1,  /* DELIMITERS => ID */
    1,  /*     DETACH => ID */
    1,  /*       EACH => ID */
    1,  /*        END => ID */
    1,  /*    EXPLAIN => ID */
    1,  /*       FAIL => ID */
    1,  /*        FOR => ID */
    1,  /*     IGNORE => ID */
    1,  /*  IMMEDIATE => ID */
    1,  /*  INITIALLY => ID */
    1,  /*    INSTEAD => ID */
    1,  /*      MATCH => ID */
    1,  /*        KEY => ID */
    1,  /*         OF => ID */
    1,  /*      RAISE => ID */
    1,  /*    REPLACE => ID */
    1,  /*   RESTRICT => ID */
    1,  /*        ROW => ID */
    1,  /*  STATEMENT => ID */
    1,  /*    TRIGGER => ID */
    1,  /*       VIEW => ID */
    1,  /*       SEMI => ID */
    1,  /*       NONE => ID */
    1,  /*       PREV => ID */
    1,  /*     LINEAR => ID */
    1,  /*     IMPORT => ID */
    1,  /*     TBNAME => ID */
    1,  /*       JOIN => ID */
    1,  /*     INSERT => ID */
    1,  /*       INTO => ID */
    1,  /*     VALUES => ID */
};
#endif /* YYFALLBACK */

/* The following structure represents a single element of the
** parser's stack.  Information stored includes:
**
**   +  The state number for the parser at this level of the stack.
**
**   +  The value of the token stored at this level of the stack.
**      (In other words, the "major" token.)
**
**   +  The semantic value stored at this level of the stack.  This is
**      the information used by the action routines in the grammar.
**      It is sometimes called the "minor" token.
**
** After the "shift" half of a SHIFTREDUCE action, the stateno field
** actually contains the reduce action for the second half of the
** SHIFTREDUCE.
*/
struct yyStackEntry {
  YYACTIONTYPE stateno;  /* The state-number, or reduce action in SHIFTREDUCE */
  YYCODETYPE major;      /* The major token value.  This is the code
                         ** number for the token at this stack level */
  YYMINORTYPE minor;     /* The user-supplied minor token value.  This
                         ** is the value of the token  */
};
typedef struct yyStackEntry yyStackEntry;

/* The state of the parser is completely contained in an instance of
** the following structure */
struct yyParser {
  yyStackEntry *yytos;          /* Pointer to top element of the stack */
#ifdef YYTRACKMAXSTACKDEPTH
  int yyhwm;                    /* High-water mark of the stack */
#endif
#ifndef YYNOERRORRECOVERY
  int yyerrcnt;                 /* Shifts left before out of the error */
#endif
  ParseARG_SDECL                /* A place to hold %extra_argument */
  ParseCTX_SDECL                /* A place to hold %extra_context */
#if YYSTACKDEPTH<=0
  int yystksz;                  /* Current side of the stack */
  yyStackEntry *yystack;        /* The parser's stack */
  yyStackEntry yystk0;          /* First stack entry */
#else
  yyStackEntry yystack[YYSTACKDEPTH];  /* The parser's stack */
  yyStackEntry *yystackEnd;            /* Last entry in the stack */
#endif
};
typedef struct yyParser yyParser;

#ifndef NDEBUG
#include <stdio.h>
static FILE *yyTraceFILE = 0;
static char *yyTracePrompt = 0;
#endif /* NDEBUG */

#ifndef NDEBUG
/* 
** Turn parser tracing on by giving a stream to which to write the trace
** and a prompt to preface each trace message.  Tracing is turned off
** by making either argument NULL 
**
** Inputs:
** <ul>
** <li> A FILE* to which trace output should be written.
**      If NULL, then tracing is turned off.
** <li> A prefix string written at the beginning of every
**      line of trace output.  If NULL, then tracing is
**      turned off.
** </ul>
**
** Outputs:
** None.
*/
void ParseTrace(FILE *TraceFILE, char *zTracePrompt){
  yyTraceFILE = TraceFILE;
  yyTracePrompt = zTracePrompt;
  if( yyTraceFILE==0 ) yyTracePrompt = 0;
  else if( yyTracePrompt==0 ) yyTraceFILE = 0;
}
#endif /* NDEBUG */

#if defined(YYCOVERAGE) || !defined(NDEBUG)
/* For tracing shifts, the names of all terminals and nonterminals
** are required.  The following table supplies these names */
static const char *const yyTokenName[] = { 
  /*    0 */ "$",
  /*    1 */ "ID",
  /*    2 */ "BOOL",
  /*    3 */ "TINYINT",
  /*    4 */ "SMALLINT",
  /*    5 */ "INTEGER",
  /*    6 */ "BIGINT",
  /*    7 */ "FLOAT",
  /*    8 */ "DOUBLE",
  /*    9 */ "STRING",
  /*   10 */ "TIMESTAMP",
  /*   11 */ "BINARY",
  /*   12 */ "NCHAR",
  /*   13 */ "OR",
  /*   14 */ "AND",
  /*   15 */ "NOT",
  /*   16 */ "EQ",
  /*   17 */ "NE",
  /*   18 */ "ISNULL",
  /*   19 */ "NOTNULL",
  /*   20 */ "IS",
  /*   21 */ "LIKE",
  /*   22 */ "GLOB",
  /*   23 */ "BETWEEN",
  /*   24 */ "IN",
  /*   25 */ "GT",
  /*   26 */ "GE",
  /*   27 */ "LT",
  /*   28 */ "LE",
  /*   29 */ "BITAND",
  /*   30 */ "BITOR",
  /*   31 */ "LSHIFT",
  /*   32 */ "RSHIFT",
  /*   33 */ "PLUS",
  /*   34 */ "MINUS",
  /*   35 */ "DIVIDE",
  /*   36 */ "TIMES",
  /*   37 */ "STAR",
  /*   38 */ "SLASH",
  /*   39 */ "REM",
  /*   40 */ "CONCAT",
  /*   41 */ "UMINUS",
  /*   42 */ "UPLUS",
  /*   43 */ "BITNOT",
  /*   44 */ "SHOW",
  /*   45 */ "DATABASES",
  /*   46 */ "TOPICS",
  /*   47 */ "FUNCTIONS",
  /*   48 */ "MNODES",
  /*   49 */ "DNODES",
  /*   50 */ "ACCOUNTS",
  /*   51 */ "USERS",
  /*   52 */ "MODULES",
  /*   53 */ "QUERIES",
  /*   54 */ "CONNECTIONS",
  /*   55 */ "STREAMS",
  /*   56 */ "VARIABLES",
  /*   57 */ "SCORES",
  /*   58 */ "GRANTS",
  /*   59 */ "VNODES",
  /*   60 */ "IPTOKEN",
  /*   61 */ "DOT",
  /*   62 */ "CREATE",
  /*   63 */ "TABLE",
  /*   64 */ "STABLE",
  /*   65 */ "DATABASE",
  /*   66 */ "TABLES",
  /*   67 */ "STABLES",
  /*   68 */ "VGROUPS",
  /*   69 */ "DROP",
  /*   70 */ "TOPIC",
  /*   71 */ "FUNCTION",
  /*   72 */ "DNODE",
  /*   73 */ "USER",
  /*   74 */ "ACCOUNT",
  /*   75 */ "USE",
  /*   76 */ "DESCRIBE",
  /*   77 */ "DESC",
  /*   78 */ "ALTER",
  /*   79 */ "PASS",
  /*   80 */ "PRIVILEGE",
  /*   81 */ "LOCAL",
  /*   82 */ "COMPACT",
  /*   83 */ "LP",
  /*   84 */ "RP",
  /*   85 */ "IF",
  /*   86 */ "EXISTS",
  /*   87 */ "AS",
  /*   88 */ "OUTPUTTYPE",
  /*   89 */ "AGGREGATE",
  /*   90 */ "BUFSIZE",
  /*   91 */ "PPS",
  /*   92 */ "TSERIES",
  /*   93 */ "DBS",
  /*   94 */ "STORAGE",
  /*   95 */ "QTIME",
  /*   96 */ "CONNS",
  /*   97 */ "STATE",
  /*   98 */ "COMMA",
  /*   99 */ "KEEP",
  /*  100 */ "CACHE",
  /*  101 */ "REPLICA",
  /*  102 */ "QUORUM",
  /*  103 */ "DAYS",
  /*  104 */ "MINROWS",
  /*  105 */ "MAXROWS",
  /*  106 */ "BLOCKS",
  /*  107 */ "CTIME",
  /*  108 */ "WAL",
  /*  109 */ "FSYNC",
  /*  110 */ "COMP",
  /*  111 */ "PRECISION",
  /*  112 */ "UPDATE",
  /*  113 */ "CACHELAST",
  /*  114 */ "PARTITIONS",
  /*  115 */ "UNSIGNED",
  /*  116 */ "TAGS",
  /*  117 */ "USING",
  /*  118 */ "TO",
  /*  119 */ "SPLIT",
  /*  120 */ "NULL",
  /*  121 */ "NOW",
  /*  122 */ "SELECT",
  /*  123 */ "UNION",
  /*  124 */ "ALL",
  /*  125 */ "DISTINCT",
  /*  126 */ "FROM",
  /*  127 */ "VARIABLE",
  /*  128 */ "INTERVAL",
  /*  129 */ "EVERY",
  /*  130 */ "SESSION",
  /*  131 */ "STATE_WINDOW",
  /*  132 */ "FILL",
  /*  133 */ "SLIDING",
  /*  134 */ "ORDER",
  /*  135 */ "BY",
  /*  136 */ "ASC",
  /*  137 */ "GROUP",
  /*  138 */ "HAVING",
  /*  139 */ "LIMIT",
  /*  140 */ "OFFSET",
  /*  141 */ "SLIMIT",
  /*  142 */ "SOFFSET",
  /*  143 */ "WHERE",
  /*  144 */ "RESET",
  /*  145 */ "QUERY",
  /*  146 */ "SYNCDB",
  /*  147 */ "ADD",
  /*  148 */ "COLUMN",
  /*  149 */ "MODIFY",
  /*  150 */ "TAG",
  /*  151 */ "CHANGE",
  /*  152 */ "SET",
  /*  153 */ "KILL",
  /*  154 */ "CONNECTION",
  /*  155 */ "STREAM",
  /*  156 */ "COLON",
  /*  157 */ "ABORT",
  /*  158 */ "AFTER",
  /*  159 */ "ATTACH",
  /*  160 */ "BEFORE",
  /*  161 */ "BEGIN",
  /*  162 */ "CASCADE",
  /*  163 */ "CLUSTER",
  /*  164 */ "CONFLICT",
  /*  165 */ "COPY",
  /*  166 */ "DEFERRED",
  /*  167 */ "DELIMITERS",
  /*  168 */ "DETACH",
  /*  169 */ "EACH",
  /*  170 */ "END",
  /*  171 */ "EXPLAIN",
  /*  172 */ "FAIL",
  /*  173 */ "FOR",
  /*  174 */ "IGNORE",
  /*  175 */ "IMMEDIATE",
  /*  176 */ "INITIALLY",
  /*  177 */ "INSTEAD",
  /*  178 */ "MATCH",
  /*  179 */ "KEY",
  /*  180 */ "OF",
  /*  181 */ "RAISE",
  /*  182 */ "REPLACE",
  /*  183 */ "RESTRICT",
  /*  184 */ "ROW",
  /*  185 */ "STATEMENT",
  /*  186 */ "TRIGGER",
  /*  187 */ "VIEW",
  /*  188 */ "SEMI",
  /*  189 */ "NONE",
  /*  190 */ "PREV",
  /*  191 */ "LINEAR",
  /*  192 */ "IMPORT",
  /*  193 */ "TBNAME",
  /*  194 */ "JOIN",
  /*  195 */ "INSERT",
  /*  196 */ "INTO",
  /*  197 */ "VALUES",
  /*  198 */ "program",
  /*  199 */ "cmd",
  /*  200 */ "dbPrefix",
  /*  201 */ "ids",
  /*  202 */ "cpxName",
  /*  203 */ "ifexists",
  /*  204 */ "alter_db_optr",
  /*  205 */ "alter_topic_optr",
  /*  206 */ "acct_optr",
  /*  207 */ "exprlist",
  /*  208 */ "ifnotexists",
  /*  209 */ "db_optr",
  /*  210 */ "topic_optr",
  /*  211 */ "typename",
  /*  212 */ "bufsize",
  /*  213 */ "pps",
  /*  214 */ "tseries",
  /*  215 */ "dbs",
  /*  216 */ "streams",
  /*  217 */ "storage",
  /*  218 */ "qtime",
  /*  219 */ "users",
  /*  220 */ "conns",
  /*  221 */ "state",
  /*  222 */ "intitemlist",
  /*  223 */ "intitem",
  /*  224 */ "keep",
  /*  225 */ "cache",
  /*  226 */ "replica",
  /*  227 */ "quorum",
  /*  228 */ "days",
  /*  229 */ "minrows",
  /*  230 */ "maxrows",
  /*  231 */ "blocks",
  /*  232 */ "ctime",
  /*  233 */ "wal",
  /*  234 */ "fsync",
  /*  235 */ "comp",
  /*  236 */ "prec",
  /*  237 */ "update",
  /*  238 */ "cachelast",
  /*  239 */ "partitions",
  /*  240 */ "signed",
  /*  241 */ "create_table_args",
  /*  242 */ "create_stable_args",
  /*  243 */ "create_table_list",
  /*  244 */ "create_from_stable",
  /*  245 */ "columnlist",
  /*  246 */ "tagitemlist",
  /*  247 */ "tagNamelist",
  /*  248 */ "to_opt",
  /*  249 */ "split_opt",
  /*  250 */ "select",
  /*  251 */ "to_split",
  /*  252 */ "column",
  /*  253 */ "tagitem",
  /*  254 */ "selcollist",
  /*  255 */ "from",
  /*  256 */ "where_opt",
  /*  257 */ "interval_option",
  /*  258 */ "sliding_opt",
  /*  259 */ "session_option",
  /*  260 */ "windowstate_option",
  /*  261 */ "fill_opt",
  /*  262 */ "groupby_opt",
  /*  263 */ "having_opt",
  /*  264 */ "orderby_opt",
  /*  265 */ "slimit_opt",
  /*  266 */ "limit_opt",
  /*  267 */ "union",
  /*  268 */ "sclp",
  /*  269 */ "distinct",
  /*  270 */ "expr",
  /*  271 */ "as",
  /*  272 */ "tablelist",
  /*  273 */ "sub",
  /*  274 */ "tmvar",
  /*  275 */ "intervalKey",
  /*  276 */ "sortlist",
  /*  277 */ "sortitem",
  /*  278 */ "item",
  /*  279 */ "sortorder",
  /*  280 */ "grouplist",
  /*  281 */ "expritem",
};
#endif /* defined(YYCOVERAGE) || !defined(NDEBUG) */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *const yyRuleName[] = {
 /*   0 */ "program ::= cmd",
 /*   1 */ "cmd ::= SHOW DATABASES",
 /*   2 */ "cmd ::= SHOW TOPICS",
 /*   3 */ "cmd ::= SHOW FUNCTIONS",
 /*   4 */ "cmd ::= SHOW MNODES",
 /*   5 */ "cmd ::= SHOW DNODES",
 /*   6 */ "cmd ::= SHOW ACCOUNTS",
 /*   7 */ "cmd ::= SHOW USERS",
 /*   8 */ "cmd ::= SHOW MODULES",
 /*   9 */ "cmd ::= SHOW QUERIES",
 /*  10 */ "cmd ::= SHOW CONNECTIONS",
 /*  11 */ "cmd ::= SHOW STREAMS",
 /*  12 */ "cmd ::= SHOW VARIABLES",
 /*  13 */ "cmd ::= SHOW SCORES",
 /*  14 */ "cmd ::= SHOW GRANTS",
 /*  15 */ "cmd ::= SHOW VNODES",
 /*  16 */ "cmd ::= SHOW VNODES IPTOKEN",
 /*  17 */ "dbPrefix ::=",
 /*  18 */ "dbPrefix ::= ids DOT",
 /*  19 */ "cpxName ::=",
 /*  20 */ "cpxName ::= DOT ids",
 /*  21 */ "cmd ::= SHOW CREATE TABLE ids cpxName",
 /*  22 */ "cmd ::= SHOW CREATE STABLE ids cpxName",
 /*  23 */ "cmd ::= SHOW CREATE DATABASE ids",
 /*  24 */ "cmd ::= SHOW dbPrefix TABLES",
 /*  25 */ "cmd ::= SHOW dbPrefix TABLES LIKE ids",
 /*  26 */ "cmd ::= SHOW dbPrefix STABLES",
 /*  27 */ "cmd ::= SHOW dbPrefix STABLES LIKE ids",
 /*  28 */ "cmd ::= SHOW dbPrefix VGROUPS",
 /*  29 */ "cmd ::= SHOW dbPrefix VGROUPS ids",
 /*  30 */ "cmd ::= DROP TABLE ifexists ids cpxName",
 /*  31 */ "cmd ::= DROP STABLE ifexists ids cpxName",
 /*  32 */ "cmd ::= DROP DATABASE ifexists ids",
 /*  33 */ "cmd ::= DROP TOPIC ifexists ids",
 /*  34 */ "cmd ::= DROP FUNCTION ids",
 /*  35 */ "cmd ::= DROP DNODE ids",
 /*  36 */ "cmd ::= DROP USER ids",
 /*  37 */ "cmd ::= DROP ACCOUNT ids",
 /*  38 */ "cmd ::= USE ids",
 /*  39 */ "cmd ::= DESCRIBE ids cpxName",
 /*  40 */ "cmd ::= DESC ids cpxName",
 /*  41 */ "cmd ::= ALTER USER ids PASS ids",
 /*  42 */ "cmd ::= ALTER USER ids PRIVILEGE ids",
 /*  43 */ "cmd ::= ALTER DNODE ids ids",
 /*  44 */ "cmd ::= ALTER DNODE ids ids ids",
 /*  45 */ "cmd ::= ALTER LOCAL ids",
 /*  46 */ "cmd ::= ALTER LOCAL ids ids",
 /*  47 */ "cmd ::= ALTER DATABASE ids alter_db_optr",
 /*  48 */ "cmd ::= ALTER TOPIC ids alter_topic_optr",
 /*  49 */ "cmd ::= ALTER ACCOUNT ids acct_optr",
 /*  50 */ "cmd ::= ALTER ACCOUNT ids PASS ids acct_optr",
 /*  51 */ "cmd ::= COMPACT VNODES IN LP exprlist RP",
 /*  52 */ "ids ::= ID",
 /*  53 */ "ids ::= STRING",
 /*  54 */ "ifexists ::= IF EXISTS",
 /*  55 */ "ifexists ::=",
 /*  56 */ "ifnotexists ::= IF NOT EXISTS",
 /*  57 */ "ifnotexists ::=",
 /*  58 */ "cmd ::= CREATE DNODE ids",
 /*  59 */ "cmd ::= CREATE ACCOUNT ids PASS ids acct_optr",
 /*  60 */ "cmd ::= CREATE DATABASE ifnotexists ids db_optr",
 /*  61 */ "cmd ::= CREATE TOPIC ifnotexists ids topic_optr",
 /*  62 */ "cmd ::= CREATE FUNCTION ids AS ids OUTPUTTYPE typename bufsize",
 /*  63 */ "cmd ::= CREATE AGGREGATE FUNCTION ids AS ids OUTPUTTYPE typename bufsize",
 /*  64 */ "cmd ::= CREATE USER ids PASS ids",
 /*  65 */ "bufsize ::=",
 /*  66 */ "bufsize ::= BUFSIZE INTEGER",
 /*  67 */ "pps ::=",
 /*  68 */ "pps ::= PPS INTEGER",
 /*  69 */ "tseries ::=",
 /*  70 */ "tseries ::= TSERIES INTEGER",
 /*  71 */ "dbs ::=",
 /*  72 */ "dbs ::= DBS INTEGER",
 /*  73 */ "streams ::=",
 /*  74 */ "streams ::= STREAMS INTEGER",
 /*  75 */ "storage ::=",
 /*  76 */ "storage ::= STORAGE INTEGER",
 /*  77 */ "qtime ::=",
 /*  78 */ "qtime ::= QTIME INTEGER",
 /*  79 */ "users ::=",
 /*  80 */ "users ::= USERS INTEGER",
 /*  81 */ "conns ::=",
 /*  82 */ "conns ::= CONNS INTEGER",
 /*  83 */ "state ::=",
 /*  84 */ "state ::= STATE ids",
 /*  85 */ "acct_optr ::= pps tseries storage streams qtime dbs users conns state",
 /*  86 */ "intitemlist ::= intitemlist COMMA intitem",
 /*  87 */ "intitemlist ::= intitem",
 /*  88 */ "intitem ::= INTEGER",
 /*  89 */ "keep ::= KEEP intitemlist",
 /*  90 */ "cache ::= CACHE INTEGER",
 /*  91 */ "replica ::= REPLICA INTEGER",
 /*  92 */ "quorum ::= QUORUM INTEGER",
 /*  93 */ "days ::= DAYS INTEGER",
 /*  94 */ "minrows ::= MINROWS INTEGER",
 /*  95 */ "maxrows ::= MAXROWS INTEGER",
 /*  96 */ "blocks ::= BLOCKS INTEGER",
 /*  97 */ "ctime ::= CTIME INTEGER",
 /*  98 */ "wal ::= WAL INTEGER",
 /*  99 */ "fsync ::= FSYNC INTEGER",
 /* 100 */ "comp ::= COMP INTEGER",
 /* 101 */ "prec ::= PRECISION STRING",
 /* 102 */ "update ::= UPDATE INTEGER",
 /* 103 */ "cachelast ::= CACHELAST INTEGER",
 /* 104 */ "partitions ::= PARTITIONS INTEGER",
 /* 105 */ "db_optr ::=",
 /* 106 */ "db_optr ::= db_optr cache",
 /* 107 */ "db_optr ::= db_optr replica",
 /* 108 */ "db_optr ::= db_optr quorum",
 /* 109 */ "db_optr ::= db_optr days",
 /* 110 */ "db_optr ::= db_optr minrows",
 /* 111 */ "db_optr ::= db_optr maxrows",
 /* 112 */ "db_optr ::= db_optr blocks",
 /* 113 */ "db_optr ::= db_optr ctime",
 /* 114 */ "db_optr ::= db_optr wal",
 /* 115 */ "db_optr ::= db_optr fsync",
 /* 116 */ "db_optr ::= db_optr comp",
 /* 117 */ "db_optr ::= db_optr prec",
 /* 118 */ "db_optr ::= db_optr keep",
 /* 119 */ "db_optr ::= db_optr update",
 /* 120 */ "db_optr ::= db_optr cachelast",
 /* 121 */ "topic_optr ::= db_optr",
 /* 122 */ "topic_optr ::= topic_optr partitions",
 /* 123 */ "alter_db_optr ::=",
 /* 124 */ "alter_db_optr ::= alter_db_optr replica",
 /* 125 */ "alter_db_optr ::= alter_db_optr quorum",
 /* 126 */ "alter_db_optr ::= alter_db_optr keep",
 /* 127 */ "alter_db_optr ::= alter_db_optr blocks",
 /* 128 */ "alter_db_optr ::= alter_db_optr comp",
 /* 129 */ "alter_db_optr ::= alter_db_optr update",
 /* 130 */ "alter_db_optr ::= alter_db_optr cachelast",
 /* 131 */ "alter_topic_optr ::= alter_db_optr",
 /* 132 */ "alter_topic_optr ::= alter_topic_optr partitions",
 /* 133 */ "typename ::= ids",
 /* 134 */ "typename ::= ids LP signed RP",
 /* 135 */ "typename ::= ids UNSIGNED",
 /* 136 */ "signed ::= INTEGER",
 /* 137 */ "signed ::= PLUS INTEGER",
 /* 138 */ "signed ::= MINUS INTEGER",
 /* 139 */ "cmd ::= CREATE TABLE create_table_args",
 /* 140 */ "cmd ::= CREATE TABLE create_stable_args",
 /* 141 */ "cmd ::= CREATE STABLE create_stable_args",
 /* 142 */ "cmd ::= CREATE TABLE create_table_list",
 /* 143 */ "create_table_list ::= create_from_stable",
 /* 144 */ "create_table_list ::= create_table_list create_from_stable",
 /* 145 */ "create_table_args ::= ifnotexists ids cpxName LP columnlist RP",
 /* 146 */ "create_stable_args ::= ifnotexists ids cpxName LP columnlist RP TAGS LP columnlist RP",
 /* 147 */ "create_from_stable ::= ifnotexists ids cpxName USING ids cpxName TAGS LP tagitemlist RP",
 /* 148 */ "create_from_stable ::= ifnotexists ids cpxName USING ids cpxName LP tagNamelist RP TAGS LP tagitemlist RP",
 /* 149 */ "tagNamelist ::= tagNamelist COMMA ids",
 /* 150 */ "tagNamelist ::= ids",
 /* 151 */ "create_table_args ::= ifnotexists ids cpxName to_opt split_opt AS select",
 /* 152 */ "to_opt ::=",
 /* 153 */ "to_opt ::= TO ids cpxName",
 /* 154 */ "split_opt ::=",
 /* 155 */ "split_opt ::= SPLIT ids",
 /* 156 */ "columnlist ::= columnlist COMMA column",
 /* 157 */ "columnlist ::= column",
 /* 158 */ "column ::= ids typename",
 /* 159 */ "tagitemlist ::= tagitemlist COMMA tagitem",
 /* 160 */ "tagitemlist ::= tagitem",
 /* 161 */ "tagitem ::= INTEGER",
 /* 162 */ "tagitem ::= FLOAT",
 /* 163 */ "tagitem ::= STRING",
 /* 164 */ "tagitem ::= BOOL",
 /* 165 */ "tagitem ::= NULL",
 /* 166 */ "tagitem ::= NOW",
 /* 167 */ "tagitem ::= MINUS INTEGER",
 /* 168 */ "tagitem ::= MINUS FLOAT",
 /* 169 */ "tagitem ::= PLUS INTEGER",
 /* 170 */ "tagitem ::= PLUS FLOAT",
 /* 171 */ "select ::= SELECT selcollist from where_opt interval_option sliding_opt session_option windowstate_option fill_opt groupby_opt having_opt orderby_opt slimit_opt limit_opt",
 /* 172 */ "select ::= LP select RP",
 /* 173 */ "union ::= select",
 /* 174 */ "union ::= union UNION ALL select",
 /* 175 */ "cmd ::= union",
 /* 176 */ "select ::= SELECT selcollist",
 /* 177 */ "sclp ::= selcollist COMMA",
 /* 178 */ "sclp ::=",
 /* 179 */ "selcollist ::= sclp distinct expr as",
 /* 180 */ "selcollist ::= sclp STAR",
 /* 181 */ "as ::= AS ids",
 /* 182 */ "as ::= ids",
 /* 183 */ "as ::=",
 /* 184 */ "distinct ::= DISTINCT",
 /* 185 */ "distinct ::=",
 /* 186 */ "from ::= FROM tablelist",
 /* 187 */ "from ::= FROM sub",
 /* 188 */ "sub ::= LP union RP",
 /* 189 */ "sub ::= LP union RP ids",
 /* 190 */ "sub ::= sub COMMA LP union RP ids",
 /* 191 */ "tablelist ::= ids cpxName",
 /* 192 */ "tablelist ::= ids cpxName ids",
 /* 193 */ "tablelist ::= tablelist COMMA ids cpxName",
 /* 194 */ "tablelist ::= tablelist COMMA ids cpxName ids",
 /* 195 */ "tmvar ::= VARIABLE",
 /* 196 */ "interval_option ::= intervalKey LP tmvar RP",
 /* 197 */ "interval_option ::= intervalKey LP tmvar COMMA tmvar RP",
 /* 198 */ "interval_option ::=",
 /* 199 */ "intervalKey ::= INTERVAL",
 /* 200 */ "intervalKey ::= EVERY",
 /* 201 */ "session_option ::=",
 /* 202 */ "session_option ::= SESSION LP ids cpxName COMMA tmvar RP",
 /* 203 */ "windowstate_option ::=",
 /* 204 */ "windowstate_option ::= STATE_WINDOW LP ids RP",
 /* 205 */ "fill_opt ::=",
 /* 206 */ "fill_opt ::= FILL LP ID COMMA tagitemlist RP",
 /* 207 */ "fill_opt ::= FILL LP ID RP",
 /* 208 */ "sliding_opt ::= SLIDING LP tmvar RP",
 /* 209 */ "sliding_opt ::=",
 /* 210 */ "orderby_opt ::=",
 /* 211 */ "orderby_opt ::= ORDER BY sortlist",
 /* 212 */ "sortlist ::= sortlist COMMA item sortorder",
 /* 213 */ "sortlist ::= item sortorder",
 /* 214 */ "item ::= ids cpxName",
 /* 215 */ "sortorder ::= ASC",
 /* 216 */ "sortorder ::= DESC",
 /* 217 */ "sortorder ::=",
 /* 218 */ "groupby_opt ::=",
 /* 219 */ "groupby_opt ::= GROUP BY grouplist",
 /* 220 */ "grouplist ::= grouplist COMMA item",
 /* 221 */ "grouplist ::= item",
 /* 222 */ "having_opt ::=",
 /* 223 */ "having_opt ::= HAVING expr",
 /* 224 */ "limit_opt ::=",
 /* 225 */ "limit_opt ::= LIMIT signed",
 /* 226 */ "limit_opt ::= LIMIT signed OFFSET signed",
 /* 227 */ "limit_opt ::= LIMIT signed COMMA signed",
 /* 228 */ "slimit_opt ::=",
 /* 229 */ "slimit_opt ::= SLIMIT signed",
 /* 230 */ "slimit_opt ::= SLIMIT signed SOFFSET signed",
 /* 231 */ "slimit_opt ::= SLIMIT signed COMMA signed",
 /* 232 */ "where_opt ::=",
 /* 233 */ "where_opt ::= WHERE expr",
 /* 234 */ "expr ::= LP expr RP",
 /* 235 */ "expr ::= ID",
 /* 236 */ "expr ::= ID DOT ID",
 /* 237 */ "expr ::= ID DOT STAR",
 /* 238 */ "expr ::= INTEGER",
 /* 239 */ "expr ::= MINUS INTEGER",
 /* 240 */ "expr ::= PLUS INTEGER",
 /* 241 */ "expr ::= FLOAT",
 /* 242 */ "expr ::= MINUS FLOAT",
 /* 243 */ "expr ::= PLUS FLOAT",
 /* 244 */ "expr ::= STRING",
 /* 245 */ "expr ::= NOW",
 /* 246 */ "expr ::= VARIABLE",
 /* 247 */ "expr ::= PLUS VARIABLE",
 /* 248 */ "expr ::= MINUS VARIABLE",
 /* 249 */ "expr ::= BOOL",
 /* 250 */ "expr ::= NULL",
 /* 251 */ "expr ::= ID LP exprlist RP",
 /* 252 */ "expr ::= ID LP STAR RP",
 /* 253 */ "expr ::= expr IS NULL",
 /* 254 */ "expr ::= expr IS NOT NULL",
 /* 255 */ "expr ::= expr LT expr",
 /* 256 */ "expr ::= expr GT expr",
 /* 257 */ "expr ::= expr LE expr",
 /* 258 */ "expr ::= expr GE expr",
 /* 259 */ "expr ::= expr NE expr",
 /* 260 */ "expr ::= expr EQ expr",
 /* 261 */ "expr ::= expr BETWEEN expr AND expr",
 /* 262 */ "expr ::= expr AND expr",
 /* 263 */ "expr ::= expr OR expr",
 /* 264 */ "expr ::= expr PLUS expr",
 /* 265 */ "expr ::= expr MINUS expr",
 /* 266 */ "expr ::= expr STAR expr",
 /* 267 */ "expr ::= expr SLASH expr",
 /* 268 */ "expr ::= expr REM expr",
 /* 269 */ "expr ::= expr LIKE expr",
 /* 270 */ "expr ::= expr IN LP exprlist RP",
 /* 271 */ "exprlist ::= exprlist COMMA expritem",
 /* 272 */ "exprlist ::= expritem",
 /* 273 */ "expritem ::= expr",
 /* 274 */ "expritem ::=",
 /* 275 */ "cmd ::= RESET QUERY CACHE",
 /* 276 */ "cmd ::= SYNCDB ids REPLICA",
 /* 277 */ "cmd ::= ALTER TABLE ids cpxName ADD COLUMN columnlist",
 /* 278 */ "cmd ::= ALTER TABLE ids cpxName DROP COLUMN ids",
 /* 279 */ "cmd ::= ALTER TABLE ids cpxName MODIFY COLUMN columnlist",
 /* 280 */ "cmd ::= ALTER TABLE ids cpxName ADD TAG columnlist",
 /* 281 */ "cmd ::= ALTER TABLE ids cpxName DROP TAG ids",
 /* 282 */ "cmd ::= ALTER TABLE ids cpxName CHANGE TAG ids ids",
 /* 283 */ "cmd ::= ALTER TABLE ids cpxName SET TAG ids EQ tagitem",
 /* 284 */ "cmd ::= ALTER TABLE ids cpxName MODIFY TAG columnlist",
 /* 285 */ "cmd ::= ALTER STABLE ids cpxName ADD COLUMN columnlist",
 /* 286 */ "cmd ::= ALTER STABLE ids cpxName DROP COLUMN ids",
 /* 287 */ "cmd ::= ALTER STABLE ids cpxName MODIFY COLUMN columnlist",
 /* 288 */ "cmd ::= ALTER STABLE ids cpxName ADD TAG columnlist",
 /* 289 */ "cmd ::= ALTER STABLE ids cpxName DROP TAG ids",
 /* 290 */ "cmd ::= ALTER STABLE ids cpxName CHANGE TAG ids ids",
 /* 291 */ "cmd ::= ALTER STABLE ids cpxName SET TAG ids EQ tagitem",
 /* 292 */ "cmd ::= ALTER STABLE ids cpxName MODIFY TAG columnlist",
 /* 293 */ "cmd ::= KILL CONNECTION INTEGER",
 /* 294 */ "cmd ::= KILL STREAM INTEGER COLON INTEGER",
 /* 295 */ "cmd ::= KILL QUERY INTEGER COLON INTEGER",
};
#endif /* NDEBUG */


#if YYSTACKDEPTH<=0
/*
** Try to increase the size of the parser stack.  Return the number
** of errors.  Return 0 on success.
*/
static int yyGrowStack(yyParser *p){
  int newSize;
  int idx;
  yyStackEntry *pNew;

  newSize = p->yystksz*2 + 100;
  idx = p->yytos ? (int)(p->yytos - p->yystack) : 0;
  if( p->yystack==&p->yystk0 ){
    pNew = malloc(newSize*sizeof(pNew[0]));
    if( pNew ) pNew[0] = p->yystk0;
  }else{
    pNew = realloc(p->yystack, newSize*sizeof(pNew[0]));
  }
  if( pNew ){
    p->yystack = pNew;
    p->yytos = &p->yystack[idx];
#ifndef NDEBUG
    if( yyTraceFILE ){
      fprintf(yyTraceFILE,"%sStack grows from %d to %d entries.\n",
              yyTracePrompt, p->yystksz, newSize);
    }
#endif
    p->yystksz = newSize;
  }
  return pNew==0; 
}
#endif

/* Datatype of the argument to the memory allocated passed as the
** second argument to ParseAlloc() below.  This can be changed by
** putting an appropriate #define in the %include section of the input
** grammar.
*/
#ifndef YYMALLOCARGTYPE
# define YYMALLOCARGTYPE size_t
#endif

/* Initialize a new parser that has already been allocated.
*/
void ParseInit(void *yypRawParser ParseCTX_PDECL){
  yyParser *yypParser = (yyParser*)yypRawParser;
  ParseCTX_STORE
#ifdef YYTRACKMAXSTACKDEPTH
  yypParser->yyhwm = 0;
#endif
#if YYSTACKDEPTH<=0
  yypParser->yytos = NULL;
  yypParser->yystack = NULL;
  yypParser->yystksz = 0;
  if( yyGrowStack(yypParser) ){
    yypParser->yystack = &yypParser->yystk0;
    yypParser->yystksz = 1;
  }
#endif
#ifndef YYNOERRORRECOVERY
  yypParser->yyerrcnt = -1;
#endif
  yypParser->yytos = yypParser->yystack;
  yypParser->yystack[0].stateno = 0;
  yypParser->yystack[0].major = 0;
#if YYSTACKDEPTH>0
  yypParser->yystackEnd = &yypParser->yystack[YYSTACKDEPTH-1];
#endif
}

#ifndef Parse_ENGINEALWAYSONSTACK
/* 
** This function allocates a new parser.
** The only argument is a pointer to a function which works like
** malloc.
**
** Inputs:
** A pointer to the function used to allocate memory.
**
** Outputs:
** A pointer to a parser.  This pointer is used in subsequent calls
** to Parse and ParseFree.
*/
void *ParseAlloc(void *(*mallocProc)(YYMALLOCARGTYPE) ParseCTX_PDECL){
  yyParser *yypParser;
  yypParser = (yyParser*)(*mallocProc)( (YYMALLOCARGTYPE)sizeof(yyParser) );
  if( yypParser ){
    ParseCTX_STORE
    ParseInit(yypParser ParseCTX_PARAM);
  }
  return (void*)yypParser;
}
#endif /* Parse_ENGINEALWAYSONSTACK */


/* The following function deletes the "minor type" or semantic value
** associated with a symbol.  The symbol can be either a terminal
** or nonterminal. "yymajor" is the symbol code, and "yypminor" is
** a pointer to the value to be deleted.  The code used to do the 
** deletions is derived from the %destructor and/or %token_destructor
** directives of the input grammar.
*/
static void yy_destructor(
  yyParser *yypParser,    /* The parser */
  YYCODETYPE yymajor,     /* Type code for object to destroy */
  YYMINORTYPE *yypminor   /* The object to be destroyed */
){
  ParseARG_FETCH
  ParseCTX_FETCH
  switch( yymajor ){
    /* Here is inserted the actions which take place when a
    ** terminal or non-terminal is destroyed.  This can happen
    ** when the symbol is popped from the stack during a
    ** reduce or during error processing or when a parser is 
    ** being destroyed before it is finished parsing.
    **
    ** Note: during a reduce, the only symbols destroyed are those
    ** which appear on the RHS of the rule, but which are *not* used
    ** inside the C code.
    */
/********* Begin destructor definitions ***************************************/
    case 207: /* exprlist */
    case 254: /* selcollist */
    case 268: /* sclp */
{
tSqlExprListDestroy((yypminor->yy345));
}
      break;
    case 222: /* intitemlist */
    case 224: /* keep */
    case 245: /* columnlist */
    case 246: /* tagitemlist */
    case 247: /* tagNamelist */
    case 261: /* fill_opt */
    case 262: /* groupby_opt */
    case 264: /* orderby_opt */
    case 276: /* sortlist */
    case 280: /* grouplist */
{
taosArrayDestroy((yypminor->yy345));
}
      break;
    case 243: /* create_table_list */
{
destroyCreateTableSql((yypminor->yy170));
}
      break;
    case 250: /* select */
{
destroySqlNode((yypminor->yy68));
}
      break;
    case 255: /* from */
    case 272: /* tablelist */
    case 273: /* sub */
{
destroyRelationInfo((yypminor->yy484));
}
      break;
    case 256: /* where_opt */
    case 263: /* having_opt */
    case 270: /* expr */
    case 281: /* expritem */
{
tSqlExprDestroy((yypminor->yy418));
}
      break;
    case 267: /* union */
{
destroyAllSqlNode((yypminor->yy345));
}
      break;
    case 277: /* sortitem */
{
tVariantDestroy(&(yypminor->yy2));
}
      break;
/********* End destructor definitions *****************************************/
    default:  break;   /* If no destructor action specified: do nothing */
  }
}

/*
** Pop the parser's stack once.
**
** If there is a destructor routine associated with the token which
** is popped from the stack, then call it.
*/
static void yy_pop_parser_stack(yyParser *pParser){
  yyStackEntry *yytos;
  assert( pParser->yytos!=0 );
  assert( pParser->yytos > pParser->yystack );
  yytos = pParser->yytos--;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sPopping %s\n",
      yyTracePrompt,
      yyTokenName[yytos->major]);
  }
#endif
  yy_destructor(pParser, yytos->major, &yytos->minor);
}

/*
** Clear all secondary memory allocations from the parser
*/
void ParseFinalize(void *p){
  yyParser *pParser = (yyParser*)p;
  while( pParser->yytos>pParser->yystack ) yy_pop_parser_stack(pParser);
#if YYSTACKDEPTH<=0
  if( pParser->yystack!=&pParser->yystk0 ) free(pParser->yystack);
#endif
}

#ifndef Parse_ENGINEALWAYSONSTACK
/* 
** Deallocate and destroy a parser.  Destructors are called for
** all stack elements before shutting the parser down.
**
** If the YYPARSEFREENEVERNULL macro exists (for example because it
** is defined in a %include section of the input grammar) then it is
** assumed that the input pointer is never NULL.
*/
void ParseFree(
  void *p,                    /* The parser to be deleted */
  void (*freeProc)(void*)     /* Function used to reclaim memory */
){
#ifndef YYPARSEFREENEVERNULL
  if( p==0 ) return;
#endif
  ParseFinalize(p);
  (*freeProc)(p);
}
#endif /* Parse_ENGINEALWAYSONSTACK */

/*
** Return the peak depth of the stack for a parser.
*/
#ifdef YYTRACKMAXSTACKDEPTH
int ParseStackPeak(void *p){
  yyParser *pParser = (yyParser*)p;
  return pParser->yyhwm;
}
#endif

/* This array of booleans keeps track of the parser statement
** coverage.  The element yycoverage[X][Y] is set when the parser
** is in state X and has a lookahead token Y.  In a well-tested
** systems, every element of this matrix should end up being set.
*/
#if defined(YYCOVERAGE)
static unsigned char yycoverage[YYNSTATE][YYNTOKEN];
#endif

/*
** Write into out a description of every state/lookahead combination that
**
**   (1)  has not been used by the parser, and
**   (2)  is not a syntax error.
**
** Return the number of missed state/lookahead combinations.
*/
#if defined(YYCOVERAGE)
int ParseCoverage(FILE *out){
  int stateno, iLookAhead, i;
  int nMissed = 0;
  for(stateno=0; stateno<YYNSTATE; stateno++){
    i = yy_shift_ofst[stateno];
    for(iLookAhead=0; iLookAhead<YYNTOKEN; iLookAhead++){
      if( yy_lookahead[i+iLookAhead]!=iLookAhead ) continue;
      if( yycoverage[stateno][iLookAhead]==0 ) nMissed++;
      if( out ){
        fprintf(out,"State %d lookahead %s %s\n", stateno,
                yyTokenName[iLookAhead],
                yycoverage[stateno][iLookAhead] ? "ok" : "missed");
      }
    }
  }
  return nMissed;
}
#endif

/*
** Find the appropriate action for a parser given the terminal
** look-ahead token iLookAhead.
*/
static YYACTIONTYPE yy_find_shift_action(
  YYCODETYPE iLookAhead,    /* The look-ahead token */
  YYACTIONTYPE stateno      /* Current state number */
){
  int i;

  if( stateno>YY_MAX_SHIFT ) return stateno;
  assert( stateno <= YY_SHIFT_COUNT );
#if defined(YYCOVERAGE)
  yycoverage[stateno][iLookAhead] = 1;
#endif
  do{
    i = yy_shift_ofst[stateno];
    assert( i>=0 );
    /* assert( i+YYNTOKEN<=(int)YY_NLOOKAHEAD ); */
    assert( iLookAhead!=YYNOCODE );
    assert( iLookAhead < YYNTOKEN );
    i += iLookAhead;
    if( i>=YY_NLOOKAHEAD || yy_lookahead[i]!=iLookAhead ){
#ifdef YYFALLBACK
      YYCODETYPE iFallback;            /* Fallback token */
      if( iLookAhead<sizeof(yyFallback)/sizeof(yyFallback[0])
             && (iFallback = yyFallback[iLookAhead])!=0 ){
#ifndef NDEBUG
        if( yyTraceFILE ){
          fprintf(yyTraceFILE, "%sFALLBACK %s => %s\n",
             yyTracePrompt, yyTokenName[iLookAhead], yyTokenName[iFallback]);
        }
#endif
        assert( yyFallback[iFallback]==0 ); /* Fallback loop must terminate */
        iLookAhead = iFallback;
        continue;
      }
#endif
#ifdef YYWILDCARD
      {
        int j = i - iLookAhead + YYWILDCARD;
        if( 
#if YY_SHIFT_MIN+YYWILDCARD<0
          j>=0 &&
#endif
#if YY_SHIFT_MAX+YYWILDCARD>=YY_ACTTAB_COUNT
          j<YY_ACTTAB_COUNT &&
#endif
          j<(int)(sizeof(yy_lookahead)/sizeof(yy_lookahead[0])) &&
          yy_lookahead[j]==YYWILDCARD && iLookAhead>0
        ){
#ifndef NDEBUG
          if( yyTraceFILE ){
            fprintf(yyTraceFILE, "%sWILDCARD %s => %s\n",
               yyTracePrompt, yyTokenName[iLookAhead],
               yyTokenName[YYWILDCARD]);
          }
#endif /* NDEBUG */
          return yy_action[j];
        }
      }
#endif /* YYWILDCARD */
      return yy_default[stateno];
    }else{
      return yy_action[i];
    }
  }while(1);
}

/*
** Find the appropriate action for a parser given the non-terminal
** look-ahead token iLookAhead.
*/
static YYACTIONTYPE yy_find_reduce_action(
  YYACTIONTYPE stateno,     /* Current state number */
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
#ifdef YYERRORSYMBOL
  if( stateno>YY_REDUCE_COUNT ){
    return yy_default[stateno];
  }
#else
  assert( stateno<=YY_REDUCE_COUNT );
#endif
  i = yy_reduce_ofst[stateno];
  assert( iLookAhead!=YYNOCODE );
  i += iLookAhead;
#ifdef YYERRORSYMBOL
  if( i<0 || i>=YY_ACTTAB_COUNT || yy_lookahead[i]!=iLookAhead ){
    return yy_default[stateno];
  }
#else
  assert( i>=0 && i<YY_ACTTAB_COUNT );
  assert( yy_lookahead[i]==iLookAhead );
#endif
  return yy_action[i];
}

/*
** The following routine is called if the stack overflows.
*/
static void yyStackOverflow(yyParser *yypParser){
   ParseARG_FETCH
   ParseCTX_FETCH
#ifndef NDEBUG
   if( yyTraceFILE ){
     fprintf(yyTraceFILE,"%sStack Overflow!\n",yyTracePrompt);
   }
#endif
   while( yypParser->yytos>yypParser->yystack ) yy_pop_parser_stack(yypParser);
   /* Here code is inserted which will execute if the parser
   ** stack every overflows */
/******** Begin %stack_overflow code ******************************************/
/******** End %stack_overflow code ********************************************/
   ParseARG_STORE /* Suppress warning about unused %extra_argument var */
   ParseCTX_STORE
}

/*
** Print tracing information for a SHIFT action
*/
#ifndef NDEBUG
static void yyTraceShift(yyParser *yypParser, int yyNewState, const char *zTag){
  if( yyTraceFILE ){
    if( yyNewState<YYNSTATE ){
      fprintf(yyTraceFILE,"%s%s '%s', go to state %d\n",
         yyTracePrompt, zTag, yyTokenName[yypParser->yytos->major],
         yyNewState);
    }else{
      fprintf(yyTraceFILE,"%s%s '%s', pending reduce %d\n",
         yyTracePrompt, zTag, yyTokenName[yypParser->yytos->major],
         yyNewState - YY_MIN_REDUCE);
    }
  }
}
#else
# define yyTraceShift(X,Y,Z)
#endif

/*
** Perform a shift action.
*/
static void yy_shift(
  yyParser *yypParser,          /* The parser to be shifted */
  YYACTIONTYPE yyNewState,      /* The new state to shift in */
  YYCODETYPE yyMajor,           /* The major token to shift in */
  ParseTOKENTYPE yyMinor        /* The minor token to shift in */
){
  yyStackEntry *yytos;
  yypParser->yytos++;
#ifdef YYTRACKMAXSTACKDEPTH
  if( (int)(yypParser->yytos - yypParser->yystack)>yypParser->yyhwm ){
    yypParser->yyhwm++;
    assert( yypParser->yyhwm == (int)(yypParser->yytos - yypParser->yystack) );
  }
#endif
#if YYSTACKDEPTH>0 
  if( yypParser->yytos>yypParser->yystackEnd ){
    yypParser->yytos--;
    yyStackOverflow(yypParser);
    return;
  }
#else
  if( yypParser->yytos>=&yypParser->yystack[yypParser->yystksz] ){
    if( yyGrowStack(yypParser) ){
      yypParser->yytos--;
      yyStackOverflow(yypParser);
      return;
    }
  }
#endif
  if( yyNewState > YY_MAX_SHIFT ){
    yyNewState += YY_MIN_REDUCE - YY_MIN_SHIFTREDUCE;
  }
  yytos = yypParser->yytos;
  yytos->stateno = yyNewState;
  yytos->major = yyMajor;
  yytos->minor.yy0 = yyMinor;
  yyTraceShift(yypParser, yyNewState, "Shift");
}

/* The following table contains information about every rule that
** is used during the reduce.
*/
static const struct {
  YYCODETYPE lhs;       /* Symbol on the left-hand side of the rule */
  signed char nrhs;     /* Negative of the number of RHS symbols in the rule */
} yyRuleInfo[] = {
  {  198,   -1 }, /* (0) program ::= cmd */
  {  199,   -2 }, /* (1) cmd ::= SHOW DATABASES */
  {  199,   -2 }, /* (2) cmd ::= SHOW TOPICS */
  {  199,   -2 }, /* (3) cmd ::= SHOW FUNCTIONS */
  {  199,   -2 }, /* (4) cmd ::= SHOW MNODES */
  {  199,   -2 }, /* (5) cmd ::= SHOW DNODES */
  {  199,   -2 }, /* (6) cmd ::= SHOW ACCOUNTS */
  {  199,   -2 }, /* (7) cmd ::= SHOW USERS */
  {  199,   -2 }, /* (8) cmd ::= SHOW MODULES */
  {  199,   -2 }, /* (9) cmd ::= SHOW QUERIES */
  {  199,   -2 }, /* (10) cmd ::= SHOW CONNECTIONS */
  {  199,   -2 }, /* (11) cmd ::= SHOW STREAMS */
  {  199,   -2 }, /* (12) cmd ::= SHOW VARIABLES */
  {  199,   -2 }, /* (13) cmd ::= SHOW SCORES */
  {  199,   -2 }, /* (14) cmd ::= SHOW GRANTS */
  {  199,   -2 }, /* (15) cmd ::= SHOW VNODES */
  {  199,   -3 }, /* (16) cmd ::= SHOW VNODES IPTOKEN */
  {  200,    0 }, /* (17) dbPrefix ::= */
  {  200,   -2 }, /* (18) dbPrefix ::= ids DOT */
  {  202,    0 }, /* (19) cpxName ::= */
  {  202,   -2 }, /* (20) cpxName ::= DOT ids */
  {  199,   -5 }, /* (21) cmd ::= SHOW CREATE TABLE ids cpxName */
  {  199,   -5 }, /* (22) cmd ::= SHOW CREATE STABLE ids cpxName */
  {  199,   -4 }, /* (23) cmd ::= SHOW CREATE DATABASE ids */
  {  199,   -3 }, /* (24) cmd ::= SHOW dbPrefix TABLES */
  {  199,   -5 }, /* (25) cmd ::= SHOW dbPrefix TABLES LIKE ids */
  {  199,   -3 }, /* (26) cmd ::= SHOW dbPrefix STABLES */
  {  199,   -5 }, /* (27) cmd ::= SHOW dbPrefix STABLES LIKE ids */
  {  199,   -3 }, /* (28) cmd ::= SHOW dbPrefix VGROUPS */
  {  199,   -4 }, /* (29) cmd ::= SHOW dbPrefix VGROUPS ids */
  {  199,   -5 }, /* (30) cmd ::= DROP TABLE ifexists ids cpxName */
  {  199,   -5 }, /* (31) cmd ::= DROP STABLE ifexists ids cpxName */
  {  199,   -4 }, /* (32) cmd ::= DROP DATABASE ifexists ids */
  {  199,   -4 }, /* (33) cmd ::= DROP TOPIC ifexists ids */
  {  199,   -3 }, /* (34) cmd ::= DROP FUNCTION ids */
  {  199,   -3 }, /* (35) cmd ::= DROP DNODE ids */
  {  199,   -3 }, /* (36) cmd ::= DROP USER ids */
  {  199,   -3 }, /* (37) cmd ::= DROP ACCOUNT ids */
  {  199,   -2 }, /* (38) cmd ::= USE ids */
  {  199,   -3 }, /* (39) cmd ::= DESCRIBE ids cpxName */
  {  199,   -3 }, /* (40) cmd ::= DESC ids cpxName */
  {  199,   -5 }, /* (41) cmd ::= ALTER USER ids PASS ids */
  {  199,   -5 }, /* (42) cmd ::= ALTER USER ids PRIVILEGE ids */
  {  199,   -4 }, /* (43) cmd ::= ALTER DNODE ids ids */
  {  199,   -5 }, /* (44) cmd ::= ALTER DNODE ids ids ids */
  {  199,   -3 }, /* (45) cmd ::= ALTER LOCAL ids */
  {  199,   -4 }, /* (46) cmd ::= ALTER LOCAL ids ids */
  {  199,   -4 }, /* (47) cmd ::= ALTER DATABASE ids alter_db_optr */
  {  199,   -4 }, /* (48) cmd ::= ALTER TOPIC ids alter_topic_optr */
  {  199,   -4 }, /* (49) cmd ::= ALTER ACCOUNT ids acct_optr */
  {  199,   -6 }, /* (50) cmd ::= ALTER ACCOUNT ids PASS ids acct_optr */
  {  199,   -6 }, /* (51) cmd ::= COMPACT VNODES IN LP exprlist RP */
  {  201,   -1 }, /* (52) ids ::= ID */
  {  201,   -1 }, /* (53) ids ::= STRING */
  {  203,   -2 }, /* (54) ifexists ::= IF EXISTS */
  {  203,    0 }, /* (55) ifexists ::= */
  {  208,   -3 }, /* (56) ifnotexists ::= IF NOT EXISTS */
  {  208,    0 }, /* (57) ifnotexists ::= */
  {  199,   -3 }, /* (58) cmd ::= CREATE DNODE ids */
  {  199,   -6 }, /* (59) cmd ::= CREATE ACCOUNT ids PASS ids acct_optr */
  {  199,   -5 }, /* (60) cmd ::= CREATE DATABASE ifnotexists ids db_optr */
  {  199,   -5 }, /* (61) cmd ::= CREATE TOPIC ifnotexists ids topic_optr */
  {  199,   -8 }, /* (62) cmd ::= CREATE FUNCTION ids AS ids OUTPUTTYPE typename bufsize */
  {  199,   -9 }, /* (63) cmd ::= CREATE AGGREGATE FUNCTION ids AS ids OUTPUTTYPE typename bufsize */
  {  199,   -5 }, /* (64) cmd ::= CREATE USER ids PASS ids */
  {  212,    0 }, /* (65) bufsize ::= */
  {  212,   -2 }, /* (66) bufsize ::= BUFSIZE INTEGER */
  {  213,    0 }, /* (67) pps ::= */
  {  213,   -2 }, /* (68) pps ::= PPS INTEGER */
  {  214,    0 }, /* (69) tseries ::= */
  {  214,   -2 }, /* (70) tseries ::= TSERIES INTEGER */
  {  215,    0 }, /* (71) dbs ::= */
  {  215,   -2 }, /* (72) dbs ::= DBS INTEGER */
  {  216,    0 }, /* (73) streams ::= */
  {  216,   -2 }, /* (74) streams ::= STREAMS INTEGER */
  {  217,    0 }, /* (75) storage ::= */
  {  217,   -2 }, /* (76) storage ::= STORAGE INTEGER */
  {  218,    0 }, /* (77) qtime ::= */
  {  218,   -2 }, /* (78) qtime ::= QTIME INTEGER */
  {  219,    0 }, /* (79) users ::= */
  {  219,   -2 }, /* (80) users ::= USERS INTEGER */
  {  220,    0 }, /* (81) conns ::= */
  {  220,   -2 }, /* (82) conns ::= CONNS INTEGER */
  {  221,    0 }, /* (83) state ::= */
  {  221,   -2 }, /* (84) state ::= STATE ids */
  {  206,   -9 }, /* (85) acct_optr ::= pps tseries storage streams qtime dbs users conns state */
  {  222,   -3 }, /* (86) intitemlist ::= intitemlist COMMA intitem */
  {  222,   -1 }, /* (87) intitemlist ::= intitem */
  {  223,   -1 }, /* (88) intitem ::= INTEGER */
  {  224,   -2 }, /* (89) keep ::= KEEP intitemlist */
  {  225,   -2 }, /* (90) cache ::= CACHE INTEGER */
  {  226,   -2 }, /* (91) replica ::= REPLICA INTEGER */
  {  227,   -2 }, /* (92) quorum ::= QUORUM INTEGER */
  {  228,   -2 }, /* (93) days ::= DAYS INTEGER */
  {  229,   -2 }, /* (94) minrows ::= MINROWS INTEGER */
  {  230,   -2 }, /* (95) maxrows ::= MAXROWS INTEGER */
  {  231,   -2 }, /* (96) blocks ::= BLOCKS INTEGER */
  {  232,   -2 }, /* (97) ctime ::= CTIME INTEGER */
  {  233,   -2 }, /* (98) wal ::= WAL INTEGER */
  {  234,   -2 }, /* (99) fsync ::= FSYNC INTEGER */
  {  235,   -2 }, /* (100) comp ::= COMP INTEGER */
  {  236,   -2 }, /* (101) prec ::= PRECISION STRING */
  {  237,   -2 }, /* (102) update ::= UPDATE INTEGER */
  {  238,   -2 }, /* (103) cachelast ::= CACHELAST INTEGER */
  {  239,   -2 }, /* (104) partitions ::= PARTITIONS INTEGER */
  {  209,    0 }, /* (105) db_optr ::= */
  {  209,   -2 }, /* (106) db_optr ::= db_optr cache */
  {  209,   -2 }, /* (107) db_optr ::= db_optr replica */
  {  209,   -2 }, /* (108) db_optr ::= db_optr quorum */
  {  209,   -2 }, /* (109) db_optr ::= db_optr days */
  {  209,   -2 }, /* (110) db_optr ::= db_optr minrows */
  {  209,   -2 }, /* (111) db_optr ::= db_optr maxrows */
  {  209,   -2 }, /* (112) db_optr ::= db_optr blocks */
  {  209,   -2 }, /* (113) db_optr ::= db_optr ctime */
  {  209,   -2 }, /* (114) db_optr ::= db_optr wal */
  {  209,   -2 }, /* (115) db_optr ::= db_optr fsync */
  {  209,   -2 }, /* (116) db_optr ::= db_optr comp */
  {  209,   -2 }, /* (117) db_optr ::= db_optr prec */
  {  209,   -2 }, /* (118) db_optr ::= db_optr keep */
  {  209,   -2 }, /* (119) db_optr ::= db_optr update */
  {  209,   -2 }, /* (120) db_optr ::= db_optr cachelast */
  {  210,   -1 }, /* (121) topic_optr ::= db_optr */
  {  210,   -2 }, /* (122) topic_optr ::= topic_optr partitions */
  {  204,    0 }, /* (123) alter_db_optr ::= */
  {  204,   -2 }, /* (124) alter_db_optr ::= alter_db_optr replica */
  {  204,   -2 }, /* (125) alter_db_optr ::= alter_db_optr quorum */
  {  204,   -2 }, /* (126) alter_db_optr ::= alter_db_optr keep */
  {  204,   -2 }, /* (127) alter_db_optr ::= alter_db_optr blocks */
  {  204,   -2 }, /* (128) alter_db_optr ::= alter_db_optr comp */
  {  204,   -2 }, /* (129) alter_db_optr ::= alter_db_optr update */
  {  204,   -2 }, /* (130) alter_db_optr ::= alter_db_optr cachelast */
  {  205,   -1 }, /* (131) alter_topic_optr ::= alter_db_optr */
  {  205,   -2 }, /* (132) alter_topic_optr ::= alter_topic_optr partitions */
  {  211,   -1 }, /* (133) typename ::= ids */
  {  211,   -4 }, /* (134) typename ::= ids LP signed RP */
  {  211,   -2 }, /* (135) typename ::= ids UNSIGNED */
  {  240,   -1 }, /* (136) signed ::= INTEGER */
  {  240,   -2 }, /* (137) signed ::= PLUS INTEGER */
  {  240,   -2 }, /* (138) signed ::= MINUS INTEGER */
  {  199,   -3 }, /* (139) cmd ::= CREATE TABLE create_table_args */
  {  199,   -3 }, /* (140) cmd ::= CREATE TABLE create_stable_args */
  {  199,   -3 }, /* (141) cmd ::= CREATE STABLE create_stable_args */
  {  199,   -3 }, /* (142) cmd ::= CREATE TABLE create_table_list */
  {  243,   -1 }, /* (143) create_table_list ::= create_from_stable */
  {  243,   -2 }, /* (144) create_table_list ::= create_table_list create_from_stable */
  {  241,   -6 }, /* (145) create_table_args ::= ifnotexists ids cpxName LP columnlist RP */
  {  242,  -10 }, /* (146) create_stable_args ::= ifnotexists ids cpxName LP columnlist RP TAGS LP columnlist RP */
  {  244,  -10 }, /* (147) create_from_stable ::= ifnotexists ids cpxName USING ids cpxName TAGS LP tagitemlist RP */
  {  244,  -13 }, /* (148) create_from_stable ::= ifnotexists ids cpxName USING ids cpxName LP tagNamelist RP TAGS LP tagitemlist RP */
  {  247,   -3 }, /* (149) tagNamelist ::= tagNamelist COMMA ids */
  {  247,   -1 }, /* (150) tagNamelist ::= ids */
  {  241,   -7 }, /* (151) create_table_args ::= ifnotexists ids cpxName to_opt split_opt AS select */
  {  248,    0 }, /* (152) to_opt ::= */
  {  248,   -3 }, /* (153) to_opt ::= TO ids cpxName */
  {  249,    0 }, /* (154) split_opt ::= */
  {  249,   -2 }, /* (155) split_opt ::= SPLIT ids */
  {  245,   -3 }, /* (156) columnlist ::= columnlist COMMA column */
  {  245,   -1 }, /* (157) columnlist ::= column */
  {  252,   -2 }, /* (158) column ::= ids typename */
  {  246,   -3 }, /* (159) tagitemlist ::= tagitemlist COMMA tagitem */
  {  246,   -1 }, /* (160) tagitemlist ::= tagitem */
  {  253,   -1 }, /* (161) tagitem ::= INTEGER */
  {  253,   -1 }, /* (162) tagitem ::= FLOAT */
  {  253,   -1 }, /* (163) tagitem ::= STRING */
  {  253,   -1 }, /* (164) tagitem ::= BOOL */
  {  253,   -1 }, /* (165) tagitem ::= NULL */
  {  253,   -1 }, /* (166) tagitem ::= NOW */
  {  253,   -2 }, /* (167) tagitem ::= MINUS INTEGER */
  {  253,   -2 }, /* (168) tagitem ::= MINUS FLOAT */
  {  253,   -2 }, /* (169) tagitem ::= PLUS INTEGER */
  {  253,   -2 }, /* (170) tagitem ::= PLUS FLOAT */
  {  250,  -14 }, /* (171) select ::= SELECT selcollist from where_opt interval_option sliding_opt session_option windowstate_option fill_opt groupby_opt having_opt orderby_opt slimit_opt limit_opt */
  {  250,   -3 }, /* (172) select ::= LP select RP */
  {  267,   -1 }, /* (173) union ::= select */
  {  267,   -4 }, /* (174) union ::= union UNION ALL select */
  {  199,   -1 }, /* (175) cmd ::= union */
  {  250,   -2 }, /* (176) select ::= SELECT selcollist */
  {  268,   -2 }, /* (177) sclp ::= selcollist COMMA */
  {  268,    0 }, /* (178) sclp ::= */
  {  254,   -4 }, /* (179) selcollist ::= sclp distinct expr as */
  {  254,   -2 }, /* (180) selcollist ::= sclp STAR */
  {  271,   -2 }, /* (181) as ::= AS ids */
  {  271,   -1 }, /* (182) as ::= ids */
  {  271,    0 }, /* (183) as ::= */
  {  269,   -1 }, /* (184) distinct ::= DISTINCT */
  {  269,    0 }, /* (185) distinct ::= */
  {  255,   -2 }, /* (186) from ::= FROM tablelist */
  {  255,   -2 }, /* (187) from ::= FROM sub */
  {  273,   -3 }, /* (188) sub ::= LP union RP */
  {  273,   -4 }, /* (189) sub ::= LP union RP ids */
  {  273,   -6 }, /* (190) sub ::= sub COMMA LP union RP ids */
  {  272,   -2 }, /* (191) tablelist ::= ids cpxName */
  {  272,   -3 }, /* (192) tablelist ::= ids cpxName ids */
  {  272,   -4 }, /* (193) tablelist ::= tablelist COMMA ids cpxName */
  {  272,   -5 }, /* (194) tablelist ::= tablelist COMMA ids cpxName ids */
  {  274,   -1 }, /* (195) tmvar ::= VARIABLE */
  {  257,   -4 }, /* (196) interval_option ::= intervalKey LP tmvar RP */
  {  257,   -6 }, /* (197) interval_option ::= intervalKey LP tmvar COMMA tmvar RP */
  {  257,    0 }, /* (198) interval_option ::= */
  {  275,   -1 }, /* (199) intervalKey ::= INTERVAL */
  {  275,   -1 }, /* (200) intervalKey ::= EVERY */
  {  259,    0 }, /* (201) session_option ::= */
  {  259,   -7 }, /* (202) session_option ::= SESSION LP ids cpxName COMMA tmvar RP */
  {  260,    0 }, /* (203) windowstate_option ::= */
  {  260,   -4 }, /* (204) windowstate_option ::= STATE_WINDOW LP ids RP */
  {  261,    0 }, /* (205) fill_opt ::= */
  {  261,   -6 }, /* (206) fill_opt ::= FILL LP ID COMMA tagitemlist RP */
  {  261,   -4 }, /* (207) fill_opt ::= FILL LP ID RP */
  {  258,   -4 }, /* (208) sliding_opt ::= SLIDING LP tmvar RP */
  {  258,    0 }, /* (209) sliding_opt ::= */
  {  264,    0 }, /* (210) orderby_opt ::= */
  {  264,   -3 }, /* (211) orderby_opt ::= ORDER BY sortlist */
  {  276,   -4 }, /* (212) sortlist ::= sortlist COMMA item sortorder */
  {  276,   -2 }, /* (213) sortlist ::= item sortorder */
  {  278,   -2 }, /* (214) item ::= ids cpxName */
  {  279,   -1 }, /* (215) sortorder ::= ASC */
  {  279,   -1 }, /* (216) sortorder ::= DESC */
  {  279,    0 }, /* (217) sortorder ::= */
  {  262,    0 }, /* (218) groupby_opt ::= */
  {  262,   -3 }, /* (219) groupby_opt ::= GROUP BY grouplist */
  {  280,   -3 }, /* (220) grouplist ::= grouplist COMMA item */
  {  280,   -1 }, /* (221) grouplist ::= item */
  {  263,    0 }, /* (222) having_opt ::= */
  {  263,   -2 }, /* (223) having_opt ::= HAVING expr */
  {  266,    0 }, /* (224) limit_opt ::= */
  {  266,   -2 }, /* (225) limit_opt ::= LIMIT signed */
  {  266,   -4 }, /* (226) limit_opt ::= LIMIT signed OFFSET signed */
  {  266,   -4 }, /* (227) limit_opt ::= LIMIT signed COMMA signed */
  {  265,    0 }, /* (228) slimit_opt ::= */
  {  265,   -2 }, /* (229) slimit_opt ::= SLIMIT signed */
  {  265,   -4 }, /* (230) slimit_opt ::= SLIMIT signed SOFFSET signed */
  {  265,   -4 }, /* (231) slimit_opt ::= SLIMIT signed COMMA signed */
  {  256,    0 }, /* (232) where_opt ::= */
  {  256,   -2 }, /* (233) where_opt ::= WHERE expr */
  {  270,   -3 }, /* (234) expr ::= LP expr RP */
  {  270,   -1 }, /* (235) expr ::= ID */
  {  270,   -3 }, /* (236) expr ::= ID DOT ID */
  {  270,   -3 }, /* (237) expr ::= ID DOT STAR */
  {  270,   -1 }, /* (238) expr ::= INTEGER */
  {  270,   -2 }, /* (239) expr ::= MINUS INTEGER */
  {  270,   -2 }, /* (240) expr ::= PLUS INTEGER */
  {  270,   -1 }, /* (241) expr ::= FLOAT */
  {  270,   -2 }, /* (242) expr ::= MINUS FLOAT */
  {  270,   -2 }, /* (243) expr ::= PLUS FLOAT */
  {  270,   -1 }, /* (244) expr ::= STRING */
  {  270,   -1 }, /* (245) expr ::= NOW */
  {  270,   -1 }, /* (246) expr ::= VARIABLE */
  {  270,   -2 }, /* (247) expr ::= PLUS VARIABLE */
  {  270,   -2 }, /* (248) expr ::= MINUS VARIABLE */
  {  270,   -1 }, /* (249) expr ::= BOOL */
  {  270,   -1 }, /* (250) expr ::= NULL */
  {  270,   -4 }, /* (251) expr ::= ID LP exprlist RP */
  {  270,   -4 }, /* (252) expr ::= ID LP STAR RP */
  {  270,   -3 }, /* (253) expr ::= expr IS NULL */
  {  270,   -4 }, /* (254) expr ::= expr IS NOT NULL */
  {  270,   -3 }, /* (255) expr ::= expr LT expr */
  {  270,   -3 }, /* (256) expr ::= expr GT expr */
  {  270,   -3 }, /* (257) expr ::= expr LE expr */
  {  270,   -3 }, /* (258) expr ::= expr GE expr */
  {  270,   -3 }, /* (259) expr ::= expr NE expr */
  {  270,   -3 }, /* (260) expr ::= expr EQ expr */
  {  270,   -5 }, /* (261) expr ::= expr BETWEEN expr AND expr */
  {  270,   -3 }, /* (262) expr ::= expr AND expr */
  {  270,   -3 }, /* (263) expr ::= expr OR expr */
  {  270,   -3 }, /* (264) expr ::= expr PLUS expr */
  {  270,   -3 }, /* (265) expr ::= expr MINUS expr */
  {  270,   -3 }, /* (266) expr ::= expr STAR expr */
  {  270,   -3 }, /* (267) expr ::= expr SLASH expr */
  {  270,   -3 }, /* (268) expr ::= expr REM expr */
  {  270,   -3 }, /* (269) expr ::= expr LIKE expr */
  {  270,   -5 }, /* (270) expr ::= expr IN LP exprlist RP */
  {  207,   -3 }, /* (271) exprlist ::= exprlist COMMA expritem */
  {  207,   -1 }, /* (272) exprlist ::= expritem */
  {  281,   -1 }, /* (273) expritem ::= expr */
  {  281,    0 }, /* (274) expritem ::= */
  {  199,   -3 }, /* (275) cmd ::= RESET QUERY CACHE */
  {  199,   -3 }, /* (276) cmd ::= SYNCDB ids REPLICA */
  {  199,   -7 }, /* (277) cmd ::= ALTER TABLE ids cpxName ADD COLUMN columnlist */
  {  199,   -7 }, /* (278) cmd ::= ALTER TABLE ids cpxName DROP COLUMN ids */
  {  199,   -7 }, /* (279) cmd ::= ALTER TABLE ids cpxName MODIFY COLUMN columnlist */
  {  199,   -7 }, /* (280) cmd ::= ALTER TABLE ids cpxName ADD TAG columnlist */
  {  199,   -7 }, /* (281) cmd ::= ALTER TABLE ids cpxName DROP TAG ids */
  {  199,   -8 }, /* (282) cmd ::= ALTER TABLE ids cpxName CHANGE TAG ids ids */
  {  199,   -9 }, /* (283) cmd ::= ALTER TABLE ids cpxName SET TAG ids EQ tagitem */
  {  199,   -7 }, /* (284) cmd ::= ALTER TABLE ids cpxName MODIFY TAG columnlist */
  {  199,   -7 }, /* (285) cmd ::= ALTER STABLE ids cpxName ADD COLUMN columnlist */
  {  199,   -7 }, /* (286) cmd ::= ALTER STABLE ids cpxName DROP COLUMN ids */
  {  199,   -7 }, /* (287) cmd ::= ALTER STABLE ids cpxName MODIFY COLUMN columnlist */
  {  199,   -7 }, /* (288) cmd ::= ALTER STABLE ids cpxName ADD TAG columnlist */
  {  199,   -7 }, /* (289) cmd ::= ALTER STABLE ids cpxName DROP TAG ids */
  {  199,   -8 }, /* (290) cmd ::= ALTER STABLE ids cpxName CHANGE TAG ids ids */
  {  199,   -9 }, /* (291) cmd ::= ALTER STABLE ids cpxName SET TAG ids EQ tagitem */
  {  199,   -7 }, /* (292) cmd ::= ALTER STABLE ids cpxName MODIFY TAG columnlist */
  {  199,   -3 }, /* (293) cmd ::= KILL CONNECTION INTEGER */
  {  199,   -5 }, /* (294) cmd ::= KILL STREAM INTEGER COLON INTEGER */
  {  199,   -5 }, /* (295) cmd ::= KILL QUERY INTEGER COLON INTEGER */
};

static void yy_accept(yyParser*);  /* Forward Declaration */

/*
** Perform a reduce action and the shift that must immediately
** follow the reduce.
**
** The yyLookahead and yyLookaheadToken parameters provide reduce actions
** access to the lookahead token (if any).  The yyLookahead will be YYNOCODE
** if the lookahead token has already been consumed.  As this procedure is
** only called from one place, optimizing compilers will in-line it, which
** means that the extra parameters have no performance impact.
*/
static YYACTIONTYPE yy_reduce(
  yyParser *yypParser,         /* The parser */
  unsigned int yyruleno,       /* Number of the rule by which to reduce */
  int yyLookahead,             /* Lookahead token, or YYNOCODE if none */
  ParseTOKENTYPE yyLookaheadToken  /* Value of the lookahead token */
  ParseCTX_PDECL                   /* %extra_context */
){
  int yygoto;                     /* The next state */
  YYACTIONTYPE yyact;             /* The next action */
  yyStackEntry *yymsp;            /* The top of the parser's stack */
  int yysize;                     /* Amount to pop the stack */
  ParseARG_FETCH
  (void)yyLookahead;
  (void)yyLookaheadToken;
  yymsp = yypParser->yytos;
#ifndef NDEBUG
  if( yyTraceFILE && yyruleno<(int)(sizeof(yyRuleName)/sizeof(yyRuleName[0])) ){
    yysize = yyRuleInfo[yyruleno].nrhs;
    if( yysize ){
      fprintf(yyTraceFILE, "%sReduce %d [%s], go to state %d.\n",
        yyTracePrompt,
        yyruleno, yyRuleName[yyruleno], yymsp[yysize].stateno);
    }else{
      fprintf(yyTraceFILE, "%sReduce %d [%s].\n",
        yyTracePrompt, yyruleno, yyRuleName[yyruleno]);
    }
  }
#endif /* NDEBUG */

  /* Check that the stack is large enough to grow by a single entry
  ** if the RHS of the rule is empty.  This ensures that there is room
  ** enough on the stack to push the LHS value */
  if( yyRuleInfo[yyruleno].nrhs==0 ){
#ifdef YYTRACKMAXSTACKDEPTH
    if( (int)(yypParser->yytos - yypParser->yystack)>yypParser->yyhwm ){
      yypParser->yyhwm++;
      assert( yypParser->yyhwm == (int)(yypParser->yytos - yypParser->yystack));
    }
#endif
#if YYSTACKDEPTH>0 
    if( yypParser->yytos>=yypParser->yystackEnd ){
      yyStackOverflow(yypParser);
      /* The call to yyStackOverflow() above pops the stack until it is
      ** empty, causing the main parser loop to exit.  So the return value
      ** is never used and does not matter. */
      return 0;
    }
#else
    if( yypParser->yytos>=&yypParser->yystack[yypParser->yystksz-1] ){
      if( yyGrowStack(yypParser) ){
        yyStackOverflow(yypParser);
        /* The call to yyStackOverflow() above pops the stack until it is
        ** empty, causing the main parser loop to exit.  So the return value
        ** is never used and does not matter. */
        return 0;
      }
      yymsp = yypParser->yytos;
    }
#endif
  }

  switch( yyruleno ){
  /* Beginning here are the reduction cases.  A typical example
  ** follows:
  **   case 0:
  **  #line <lineno> <grammarfile>
  **     { ... }           // User supplied code
  **  #line <lineno> <thisfile>
  **     break;
  */
/********** Begin reduce actions **********************************************/
        YYMINORTYPE yylhsminor;
      case 0: /* program ::= cmd */
      case 139: /* cmd ::= CREATE TABLE create_table_args */ yytestcase(yyruleno==139);
      case 140: /* cmd ::= CREATE TABLE create_stable_args */ yytestcase(yyruleno==140);
      case 141: /* cmd ::= CREATE STABLE create_stable_args */ yytestcase(yyruleno==141);
{}
        break;
      case 1: /* cmd ::= SHOW DATABASES */
{ setShowOptions(pInfo, TSDB_MGMT_TABLE_DB, 0, 0);}
        break;
      case 2: /* cmd ::= SHOW TOPICS */
{ setShowOptions(pInfo, TSDB_MGMT_TABLE_TP, 0, 0);}
        break;
      case 3: /* cmd ::= SHOW FUNCTIONS */
{ setShowOptions(pInfo, TSDB_MGMT_TABLE_FUNCTION, 0, 0);}
        break;
      case 4: /* cmd ::= SHOW MNODES */
{ setShowOptions(pInfo, TSDB_MGMT_TABLE_MNODE, 0, 0);}
        break;
      case 5: /* cmd ::= SHOW DNODES */
{ setShowOptions(pInfo, TSDB_MGMT_TABLE_DNODE, 0, 0);}
        break;
      case 6: /* cmd ::= SHOW ACCOUNTS */
{ setShowOptions(pInfo, TSDB_MGMT_TABLE_ACCT, 0, 0);}
        break;
      case 7: /* cmd ::= SHOW USERS */
{ setShowOptions(pInfo, TSDB_MGMT_TABLE_USER, 0, 0);}
        break;
      case 8: /* cmd ::= SHOW MODULES */
{ setShowOptions(pInfo, TSDB_MGMT_TABLE_MODULE, 0, 0);  }
        break;
      case 9: /* cmd ::= SHOW QUERIES */
{ setShowOptions(pInfo, TSDB_MGMT_TABLE_QUERIES, 0, 0);  }
        break;
      case 10: /* cmd ::= SHOW CONNECTIONS */
{ setShowOptions(pInfo, TSDB_MGMT_TABLE_CONNS, 0, 0);}
        break;
      case 11: /* cmd ::= SHOW STREAMS */
{ setShowOptions(pInfo, TSDB_MGMT_TABLE_STREAMS, 0, 0);  }
        break;
      case 12: /* cmd ::= SHOW VARIABLES */
{ setShowOptions(pInfo, TSDB_MGMT_TABLE_VARIABLES, 0, 0);  }
        break;
      case 13: /* cmd ::= SHOW SCORES */
{ setShowOptions(pInfo, TSDB_MGMT_TABLE_SCORES, 0, 0);   }
        break;
      case 14: /* cmd ::= SHOW GRANTS */
{ setShowOptions(pInfo, TSDB_MGMT_TABLE_GRANTS, 0, 0);   }
        break;
      case 15: /* cmd ::= SHOW VNODES */
{ setShowOptions(pInfo, TSDB_MGMT_TABLE_VNODES, 0, 0); }
        break;
      case 16: /* cmd ::= SHOW VNODES IPTOKEN */
{ setShowOptions(pInfo, TSDB_MGMT_TABLE_VNODES, &yymsp[0].minor.yy0, 0); }
        break;
      case 17: /* dbPrefix ::= */
{yymsp[1].minor.yy0.n = 0; yymsp[1].minor.yy0.type = 0;}
        break;
      case 18: /* dbPrefix ::= ids DOT */
{yylhsminor.yy0 = yymsp[-1].minor.yy0;  }
  yymsp[-1].minor.yy0 = yylhsminor.yy0;
        break;
      case 19: /* cpxName ::= */
{yymsp[1].minor.yy0.n = 0;  }
        break;
      case 20: /* cpxName ::= DOT ids */
{yymsp[-1].minor.yy0 = yymsp[0].minor.yy0; yymsp[-1].minor.yy0.n += 1;    }
        break;
      case 21: /* cmd ::= SHOW CREATE TABLE ids cpxName */
{
   yymsp[-1].minor.yy0.n += yymsp[0].minor.yy0.n;
   setDCLSqlElems(pInfo, TSDB_SQL_SHOW_CREATE_TABLE, 1, &yymsp[-1].minor.yy0);
}
        break;
      case 22: /* cmd ::= SHOW CREATE STABLE ids cpxName */
{
   yymsp[-1].minor.yy0.n += yymsp[0].minor.yy0.n;
   setDCLSqlElems(pInfo, TSDB_SQL_SHOW_CREATE_STABLE, 1, &yymsp[-1].minor.yy0);
}
        break;
      case 23: /* cmd ::= SHOW CREATE DATABASE ids */
{
  setDCLSqlElems(pInfo, TSDB_SQL_SHOW_CREATE_DATABASE, 1, &yymsp[0].minor.yy0);
}
        break;
      case 24: /* cmd ::= SHOW dbPrefix TABLES */
{
    setShowOptions(pInfo, TSDB_MGMT_TABLE_TABLE, &yymsp[-1].minor.yy0, 0);
}
        break;
      case 25: /* cmd ::= SHOW dbPrefix TABLES LIKE ids */
{
    setShowOptions(pInfo, TSDB_MGMT_TABLE_TABLE, &yymsp[-3].minor.yy0, &yymsp[0].minor.yy0);
}
        break;
      case 26: /* cmd ::= SHOW dbPrefix STABLES */
{
    setShowOptions(pInfo, TSDB_MGMT_TABLE_METRIC, &yymsp[-1].minor.yy0, 0);
}
        break;
      case 27: /* cmd ::= SHOW dbPrefix STABLES LIKE ids */
{
    SStrToken token;
    tSetDbName(&token, &yymsp[-3].minor.yy0);
    setShowOptions(pInfo, TSDB_MGMT_TABLE_METRIC, &token, &yymsp[0].minor.yy0);
}
        break;
      case 28: /* cmd ::= SHOW dbPrefix VGROUPS */
{
    SStrToken token;
    tSetDbName(&token, &yymsp[-1].minor.yy0);
    setShowOptions(pInfo, TSDB_MGMT_TABLE_VGROUP, &token, 0);
}
        break;
      case 29: /* cmd ::= SHOW dbPrefix VGROUPS ids */
{
    SStrToken token;
    tSetDbName(&token, &yymsp[-2].minor.yy0);
    setShowOptions(pInfo, TSDB_MGMT_TABLE_VGROUP, &token, &yymsp[0].minor.yy0);
}
        break;
      case 30: /* cmd ::= DROP TABLE ifexists ids cpxName */
{
    yymsp[-1].minor.yy0.n += yymsp[0].minor.yy0.n;
    setDropDbTableInfo(pInfo, TSDB_SQL_DROP_TABLE, &yymsp[-1].minor.yy0, &yymsp[-2].minor.yy0, -1, -1);
}
        break;
      case 31: /* cmd ::= DROP STABLE ifexists ids cpxName */
{
    yymsp[-1].minor.yy0.n += yymsp[0].minor.yy0.n;
    setDropDbTableInfo(pInfo, TSDB_SQL_DROP_TABLE, &yymsp[-1].minor.yy0, &yymsp[-2].minor.yy0, -1, TSDB_SUPER_TABLE);
}
        break;
      case 32: /* cmd ::= DROP DATABASE ifexists ids */
{ setDropDbTableInfo(pInfo, TSDB_SQL_DROP_DB, &yymsp[0].minor.yy0, &yymsp[-1].minor.yy0, TSDB_DB_TYPE_DEFAULT, -1); }
        break;
      case 33: /* cmd ::= DROP TOPIC ifexists ids */
{ setDropDbTableInfo(pInfo, TSDB_SQL_DROP_DB, &yymsp[0].minor.yy0, &yymsp[-1].minor.yy0, TSDB_DB_TYPE_TOPIC, -1); }
        break;
      case 34: /* cmd ::= DROP FUNCTION ids */
{ setDropFuncInfo(pInfo, TSDB_SQL_DROP_FUNCTION, &yymsp[0].minor.yy0); }
        break;
      case 35: /* cmd ::= DROP DNODE ids */
{ setDCLSqlElems(pInfo, TSDB_SQL_DROP_DNODE, 1, &yymsp[0].minor.yy0);    }
        break;
      case 36: /* cmd ::= DROP USER ids */
{ setDCLSqlElems(pInfo, TSDB_SQL_DROP_USER, 1, &yymsp[0].minor.yy0);     }
        break;
      case 37: /* cmd ::= DROP ACCOUNT ids */
{ setDCLSqlElems(pInfo, TSDB_SQL_DROP_ACCT, 1, &yymsp[0].minor.yy0);  }
        break;
      case 38: /* cmd ::= USE ids */
{ setDCLSqlElems(pInfo, TSDB_SQL_USE_DB, 1, &yymsp[0].minor.yy0);}
        break;
      case 39: /* cmd ::= DESCRIBE ids cpxName */
      case 40: /* cmd ::= DESC ids cpxName */ yytestcase(yyruleno==40);
{
    yymsp[-1].minor.yy0.n += yymsp[0].minor.yy0.n;
    setDCLSqlElems(pInfo, TSDB_SQL_DESCRIBE_TABLE, 1, &yymsp[-1].minor.yy0);
}
        break;
      case 41: /* cmd ::= ALTER USER ids PASS ids */
{ setAlterUserSql(pInfo, TSDB_ALTER_USER_PASSWD, &yymsp[-2].minor.yy0, &yymsp[0].minor.yy0, NULL);    }
        break;
      case 42: /* cmd ::= ALTER USER ids PRIVILEGE ids */
{ setAlterUserSql(pInfo, TSDB_ALTER_USER_PRIVILEGES, &yymsp[-2].minor.yy0, NULL, &yymsp[0].minor.yy0);}
        break;
      case 43: /* cmd ::= ALTER DNODE ids ids */
{ setDCLSqlElems(pInfo, TSDB_SQL_CFG_DNODE, 2, &yymsp[-1].minor.yy0, &yymsp[0].minor.yy0);          }
        break;
      case 44: /* cmd ::= ALTER DNODE ids ids ids */
{ setDCLSqlElems(pInfo, TSDB_SQL_CFG_DNODE, 3, &yymsp[-2].minor.yy0, &yymsp[-1].minor.yy0, &yymsp[0].minor.yy0);      }
        break;
      case 45: /* cmd ::= ALTER LOCAL ids */
{ setDCLSqlElems(pInfo, TSDB_SQL_CFG_LOCAL, 1, &yymsp[0].minor.yy0);              }
        break;
      case 46: /* cmd ::= ALTER LOCAL ids ids */
{ setDCLSqlElems(pInfo, TSDB_SQL_CFG_LOCAL, 2, &yymsp[-1].minor.yy0, &yymsp[0].minor.yy0);          }
        break;
      case 47: /* cmd ::= ALTER DATABASE ids alter_db_optr */
      case 48: /* cmd ::= ALTER TOPIC ids alter_topic_optr */ yytestcase(yyruleno==48);
{ SStrToken t = {0};  setCreateDbInfo(pInfo, TSDB_SQL_ALTER_DB, &yymsp[-1].minor.yy0, &yymsp[0].minor.yy10, &t);}
        break;
      case 49: /* cmd ::= ALTER ACCOUNT ids acct_optr */
{ setCreateAcctSql(pInfo, TSDB_SQL_ALTER_ACCT, &yymsp[-1].minor.yy0, NULL, &yymsp[0].minor.yy427);}
        break;
      case 50: /* cmd ::= ALTER ACCOUNT ids PASS ids acct_optr */
{ setCreateAcctSql(pInfo, TSDB_SQL_ALTER_ACCT, &yymsp[-3].minor.yy0, &yymsp[-1].minor.yy0, &yymsp[0].minor.yy427);}
        break;
      case 51: /* cmd ::= COMPACT VNODES IN LP exprlist RP */
{ setCompactVnodeSql(pInfo, TSDB_SQL_COMPACT_VNODE, yymsp[-1].minor.yy345);}
        break;
      case 52: /* ids ::= ID */
      case 53: /* ids ::= STRING */ yytestcase(yyruleno==53);
{yylhsminor.yy0 = yymsp[0].minor.yy0; }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 54: /* ifexists ::= IF EXISTS */
{ yymsp[-1].minor.yy0.n = 1;}
        break;
      case 55: /* ifexists ::= */
      case 57: /* ifnotexists ::= */ yytestcase(yyruleno==57);
      case 185: /* distinct ::= */ yytestcase(yyruleno==185);
{ yymsp[1].minor.yy0.n = 0;}
        break;
      case 56: /* ifnotexists ::= IF NOT EXISTS */
{ yymsp[-2].minor.yy0.n = 1;}
        break;
      case 58: /* cmd ::= CREATE DNODE ids */
{ setDCLSqlElems(pInfo, TSDB_SQL_CREATE_DNODE, 1, &yymsp[0].minor.yy0);}
        break;
      case 59: /* cmd ::= CREATE ACCOUNT ids PASS ids acct_optr */
{ setCreateAcctSql(pInfo, TSDB_SQL_CREATE_ACCT, &yymsp[-3].minor.yy0, &yymsp[-1].minor.yy0, &yymsp[0].minor.yy427);}
        break;
      case 60: /* cmd ::= CREATE DATABASE ifnotexists ids db_optr */
      case 61: /* cmd ::= CREATE TOPIC ifnotexists ids topic_optr */ yytestcase(yyruleno==61);
{ setCreateDbInfo(pInfo, TSDB_SQL_CREATE_DB, &yymsp[-1].minor.yy0, &yymsp[0].minor.yy10, &yymsp[-2].minor.yy0);}
        break;
      case 62: /* cmd ::= CREATE FUNCTION ids AS ids OUTPUTTYPE typename bufsize */
{ setCreateFuncInfo(pInfo, TSDB_SQL_CREATE_FUNCTION, &yymsp[-5].minor.yy0, &yymsp[-3].minor.yy0, &yymsp[-1].minor.yy487, &yymsp[0].minor.yy0, 1);}
        break;
      case 63: /* cmd ::= CREATE AGGREGATE FUNCTION ids AS ids OUTPUTTYPE typename bufsize */
{ setCreateFuncInfo(pInfo, TSDB_SQL_CREATE_FUNCTION, &yymsp[-5].minor.yy0, &yymsp[-3].minor.yy0, &yymsp[-1].minor.yy487, &yymsp[0].minor.yy0, 2);}
        break;
      case 64: /* cmd ::= CREATE USER ids PASS ids */
{ setCreateUserSql(pInfo, &yymsp[-2].minor.yy0, &yymsp[0].minor.yy0);}
        break;
      case 65: /* bufsize ::= */
      case 67: /* pps ::= */ yytestcase(yyruleno==67);
      case 69: /* tseries ::= */ yytestcase(yyruleno==69);
      case 71: /* dbs ::= */ yytestcase(yyruleno==71);
      case 73: /* streams ::= */ yytestcase(yyruleno==73);
      case 75: /* storage ::= */ yytestcase(yyruleno==75);
      case 77: /* qtime ::= */ yytestcase(yyruleno==77);
      case 79: /* users ::= */ yytestcase(yyruleno==79);
      case 81: /* conns ::= */ yytestcase(yyruleno==81);
      case 83: /* state ::= */ yytestcase(yyruleno==83);
{ yymsp[1].minor.yy0.n = 0;   }
        break;
      case 66: /* bufsize ::= BUFSIZE INTEGER */
      case 68: /* pps ::= PPS INTEGER */ yytestcase(yyruleno==68);
      case 70: /* tseries ::= TSERIES INTEGER */ yytestcase(yyruleno==70);
      case 72: /* dbs ::= DBS INTEGER */ yytestcase(yyruleno==72);
      case 74: /* streams ::= STREAMS INTEGER */ yytestcase(yyruleno==74);
      case 76: /* storage ::= STORAGE INTEGER */ yytestcase(yyruleno==76);
      case 78: /* qtime ::= QTIME INTEGER */ yytestcase(yyruleno==78);
      case 80: /* users ::= USERS INTEGER */ yytestcase(yyruleno==80);
      case 82: /* conns ::= CONNS INTEGER */ yytestcase(yyruleno==82);
      case 84: /* state ::= STATE ids */ yytestcase(yyruleno==84);
{ yymsp[-1].minor.yy0 = yymsp[0].minor.yy0;     }
        break;
      case 85: /* acct_optr ::= pps tseries storage streams qtime dbs users conns state */
{
    yylhsminor.yy427.maxUsers   = (yymsp[-2].minor.yy0.n>0)?atoi(yymsp[-2].minor.yy0.z):-1;
    yylhsminor.yy427.maxDbs     = (yymsp[-3].minor.yy0.n>0)?atoi(yymsp[-3].minor.yy0.z):-1;
    yylhsminor.yy427.maxTimeSeries = (yymsp[-7].minor.yy0.n>0)?atoi(yymsp[-7].minor.yy0.z):-1;
    yylhsminor.yy427.maxStreams = (yymsp[-5].minor.yy0.n>0)?atoi(yymsp[-5].minor.yy0.z):-1;
    yylhsminor.yy427.maxPointsPerSecond     = (yymsp[-8].minor.yy0.n>0)?atoi(yymsp[-8].minor.yy0.z):-1;
    yylhsminor.yy427.maxStorage = (yymsp[-6].minor.yy0.n>0)?strtoll(yymsp[-6].minor.yy0.z, NULL, 10):-1;
    yylhsminor.yy427.maxQueryTime   = (yymsp[-4].minor.yy0.n>0)?strtoll(yymsp[-4].minor.yy0.z, NULL, 10):-1;
    yylhsminor.yy427.maxConnections   = (yymsp[-1].minor.yy0.n>0)?atoi(yymsp[-1].minor.yy0.z):-1;
    yylhsminor.yy427.stat    = yymsp[0].minor.yy0;
}
  yymsp[-8].minor.yy427 = yylhsminor.yy427;
        break;
      case 86: /* intitemlist ::= intitemlist COMMA intitem */
      case 159: /* tagitemlist ::= tagitemlist COMMA tagitem */ yytestcase(yyruleno==159);
{ yylhsminor.yy345 = tVariantListAppend(yymsp[-2].minor.yy345, &yymsp[0].minor.yy2, -1);    }
  yymsp[-2].minor.yy345 = yylhsminor.yy345;
        break;
      case 87: /* intitemlist ::= intitem */
      case 160: /* tagitemlist ::= tagitem */ yytestcase(yyruleno==160);
{ yylhsminor.yy345 = tVariantListAppend(NULL, &yymsp[0].minor.yy2, -1); }
  yymsp[0].minor.yy345 = yylhsminor.yy345;
        break;
      case 88: /* intitem ::= INTEGER */
      case 161: /* tagitem ::= INTEGER */ yytestcase(yyruleno==161);
      case 162: /* tagitem ::= FLOAT */ yytestcase(yyruleno==162);
      case 163: /* tagitem ::= STRING */ yytestcase(yyruleno==163);
      case 164: /* tagitem ::= BOOL */ yytestcase(yyruleno==164);
{ toTSDBType(yymsp[0].minor.yy0.type); tVariantCreate(&yylhsminor.yy2, &yymsp[0].minor.yy0); }
  yymsp[0].minor.yy2 = yylhsminor.yy2;
        break;
      case 89: /* keep ::= KEEP intitemlist */
{ yymsp[-1].minor.yy345 = yymsp[0].minor.yy345; }
        break;
      case 90: /* cache ::= CACHE INTEGER */
      case 91: /* replica ::= REPLICA INTEGER */ yytestcase(yyruleno==91);
      case 92: /* quorum ::= QUORUM INTEGER */ yytestcase(yyruleno==92);
      case 93: /* days ::= DAYS INTEGER */ yytestcase(yyruleno==93);
      case 94: /* minrows ::= MINROWS INTEGER */ yytestcase(yyruleno==94);
      case 95: /* maxrows ::= MAXROWS INTEGER */ yytestcase(yyruleno==95);
      case 96: /* blocks ::= BLOCKS INTEGER */ yytestcase(yyruleno==96);
      case 97: /* ctime ::= CTIME INTEGER */ yytestcase(yyruleno==97);
      case 98: /* wal ::= WAL INTEGER */ yytestcase(yyruleno==98);
      case 99: /* fsync ::= FSYNC INTEGER */ yytestcase(yyruleno==99);
      case 100: /* comp ::= COMP INTEGER */ yytestcase(yyruleno==100);
      case 101: /* prec ::= PRECISION STRING */ yytestcase(yyruleno==101);
      case 102: /* update ::= UPDATE INTEGER */ yytestcase(yyruleno==102);
      case 103: /* cachelast ::= CACHELAST INTEGER */ yytestcase(yyruleno==103);
      case 104: /* partitions ::= PARTITIONS INTEGER */ yytestcase(yyruleno==104);
{ yymsp[-1].minor.yy0 = yymsp[0].minor.yy0; }
        break;
      case 105: /* db_optr ::= */
{setDefaultCreateDbOption(&yymsp[1].minor.yy10); yymsp[1].minor.yy10.dbType = TSDB_DB_TYPE_DEFAULT;}
        break;
      case 106: /* db_optr ::= db_optr cache */
{ yylhsminor.yy10 = yymsp[-1].minor.yy10; yylhsminor.yy10.cacheBlockSize = strtol(yymsp[0].minor.yy0.z, NULL, 10); }
  yymsp[-1].minor.yy10 = yylhsminor.yy10;
        break;
      case 107: /* db_optr ::= db_optr replica */
      case 124: /* alter_db_optr ::= alter_db_optr replica */ yytestcase(yyruleno==124);
{ yylhsminor.yy10 = yymsp[-1].minor.yy10; yylhsminor.yy10.replica = strtol(yymsp[0].minor.yy0.z, NULL, 10); }
  yymsp[-1].minor.yy10 = yylhsminor.yy10;
        break;
      case 108: /* db_optr ::= db_optr quorum */
      case 125: /* alter_db_optr ::= alter_db_optr quorum */ yytestcase(yyruleno==125);
{ yylhsminor.yy10 = yymsp[-1].minor.yy10; yylhsminor.yy10.quorum = strtol(yymsp[0].minor.yy0.z, NULL, 10); }
  yymsp[-1].minor.yy10 = yylhsminor.yy10;
        break;
      case 109: /* db_optr ::= db_optr days */
{ yylhsminor.yy10 = yymsp[-1].minor.yy10; yylhsminor.yy10.daysPerFile = strtol(yymsp[0].minor.yy0.z, NULL, 10); }
  yymsp[-1].minor.yy10 = yylhsminor.yy10;
        break;
      case 110: /* db_optr ::= db_optr minrows */
{ yylhsminor.yy10 = yymsp[-1].minor.yy10; yylhsminor.yy10.minRowsPerBlock = strtod(yymsp[0].minor.yy0.z, NULL); }
  yymsp[-1].minor.yy10 = yylhsminor.yy10;
        break;
      case 111: /* db_optr ::= db_optr maxrows */
{ yylhsminor.yy10 = yymsp[-1].minor.yy10; yylhsminor.yy10.maxRowsPerBlock = strtod(yymsp[0].minor.yy0.z, NULL); }
  yymsp[-1].minor.yy10 = yylhsminor.yy10;
        break;
      case 112: /* db_optr ::= db_optr blocks */
      case 127: /* alter_db_optr ::= alter_db_optr blocks */ yytestcase(yyruleno==127);
{ yylhsminor.yy10 = yymsp[-1].minor.yy10; yylhsminor.yy10.numOfBlocks = strtol(yymsp[0].minor.yy0.z, NULL, 10); }
  yymsp[-1].minor.yy10 = yylhsminor.yy10;
        break;
      case 113: /* db_optr ::= db_optr ctime */
{ yylhsminor.yy10 = yymsp[-1].minor.yy10; yylhsminor.yy10.commitTime = strtol(yymsp[0].minor.yy0.z, NULL, 10); }
  yymsp[-1].minor.yy10 = yylhsminor.yy10;
        break;
      case 114: /* db_optr ::= db_optr wal */
{ yylhsminor.yy10 = yymsp[-1].minor.yy10; yylhsminor.yy10.walLevel = strtol(yymsp[0].minor.yy0.z, NULL, 10); }
  yymsp[-1].minor.yy10 = yylhsminor.yy10;
        break;
      case 115: /* db_optr ::= db_optr fsync */
{ yylhsminor.yy10 = yymsp[-1].minor.yy10; yylhsminor.yy10.fsyncPeriod = strtol(yymsp[0].minor.yy0.z, NULL, 10); }
  yymsp[-1].minor.yy10 = yylhsminor.yy10;
        break;
      case 116: /* db_optr ::= db_optr comp */
      case 128: /* alter_db_optr ::= alter_db_optr comp */ yytestcase(yyruleno==128);
{ yylhsminor.yy10 = yymsp[-1].minor.yy10; yylhsminor.yy10.compressionLevel = strtol(yymsp[0].minor.yy0.z, NULL, 10); }
  yymsp[-1].minor.yy10 = yylhsminor.yy10;
        break;
      case 117: /* db_optr ::= db_optr prec */
{ yylhsminor.yy10 = yymsp[-1].minor.yy10; yylhsminor.yy10.precision = yymsp[0].minor.yy0; }
  yymsp[-1].minor.yy10 = yylhsminor.yy10;
        break;
      case 118: /* db_optr ::= db_optr keep */
      case 126: /* alter_db_optr ::= alter_db_optr keep */ yytestcase(yyruleno==126);
{ yylhsminor.yy10 = yymsp[-1].minor.yy10; yylhsminor.yy10.keep = yymsp[0].minor.yy345; }
  yymsp[-1].minor.yy10 = yylhsminor.yy10;
        break;
      case 119: /* db_optr ::= db_optr update */
      case 129: /* alter_db_optr ::= alter_db_optr update */ yytestcase(yyruleno==129);
{ yylhsminor.yy10 = yymsp[-1].minor.yy10; yylhsminor.yy10.update = strtol(yymsp[0].minor.yy0.z, NULL, 10); }
  yymsp[-1].minor.yy10 = yylhsminor.yy10;
        break;
      case 120: /* db_optr ::= db_optr cachelast */
      case 130: /* alter_db_optr ::= alter_db_optr cachelast */ yytestcase(yyruleno==130);
{ yylhsminor.yy10 = yymsp[-1].minor.yy10; yylhsminor.yy10.cachelast = strtol(yymsp[0].minor.yy0.z, NULL, 10); }
  yymsp[-1].minor.yy10 = yylhsminor.yy10;
        break;
      case 121: /* topic_optr ::= db_optr */
      case 131: /* alter_topic_optr ::= alter_db_optr */ yytestcase(yyruleno==131);
{ yylhsminor.yy10 = yymsp[0].minor.yy10; yylhsminor.yy10.dbType = TSDB_DB_TYPE_TOPIC; }
  yymsp[0].minor.yy10 = yylhsminor.yy10;
        break;
      case 122: /* topic_optr ::= topic_optr partitions */
      case 132: /* alter_topic_optr ::= alter_topic_optr partitions */ yytestcase(yyruleno==132);
{ yylhsminor.yy10 = yymsp[-1].minor.yy10; yylhsminor.yy10.partitions = strtol(yymsp[0].minor.yy0.z, NULL, 10); }
  yymsp[-1].minor.yy10 = yylhsminor.yy10;
        break;
      case 123: /* alter_db_optr ::= */
{ setDefaultCreateDbOption(&yymsp[1].minor.yy10); yymsp[1].minor.yy10.dbType = TSDB_DB_TYPE_DEFAULT;}
        break;
      case 133: /* typename ::= ids */
{
  yymsp[0].minor.yy0.type = 0;
  tSetColumnType (&yylhsminor.yy487, &yymsp[0].minor.yy0);
}
  yymsp[0].minor.yy487 = yylhsminor.yy487;
        break;
      case 134: /* typename ::= ids LP signed RP */
{
  if (yymsp[-1].minor.yy525 <= 0) {
    yymsp[-3].minor.yy0.type = 0;
    tSetColumnType(&yylhsminor.yy487, &yymsp[-3].minor.yy0);
  } else {
    yymsp[-3].minor.yy0.type = -yymsp[-1].minor.yy525;  // negative value of name length
    tSetColumnType(&yylhsminor.yy487, &yymsp[-3].minor.yy0);
  }
}
  yymsp[-3].minor.yy487 = yylhsminor.yy487;
        break;
      case 135: /* typename ::= ids UNSIGNED */
{
  yymsp[-1].minor.yy0.type = 0;
  yymsp[-1].minor.yy0.n = ((yymsp[0].minor.yy0.z + yymsp[0].minor.yy0.n) - yymsp[-1].minor.yy0.z);
  tSetColumnType (&yylhsminor.yy487, &yymsp[-1].minor.yy0);
}
  yymsp[-1].minor.yy487 = yylhsminor.yy487;
        break;
      case 136: /* signed ::= INTEGER */
{ yylhsminor.yy525 = strtol(yymsp[0].minor.yy0.z, NULL, 10); }
  yymsp[0].minor.yy525 = yylhsminor.yy525;
        break;
      case 137: /* signed ::= PLUS INTEGER */
{ yymsp[-1].minor.yy525 = strtol(yymsp[0].minor.yy0.z, NULL, 10); }
        break;
      case 138: /* signed ::= MINUS INTEGER */
{ yymsp[-1].minor.yy525 = -strtol(yymsp[0].minor.yy0.z, NULL, 10);}
        break;
      case 142: /* cmd ::= CREATE TABLE create_table_list */
{ pInfo->type = TSDB_SQL_CREATE_TABLE; pInfo->pCreateTableInfo = yymsp[0].minor.yy170;}
        break;
      case 143: /* create_table_list ::= create_from_stable */
{
  SCreateTableSql* pCreateTable = calloc(1, sizeof(SCreateTableSql));
  pCreateTable->childTableInfo = taosArrayInit(4, sizeof(SCreatedTableInfo));

  taosArrayPush(pCreateTable->childTableInfo, &yymsp[0].minor.yy72);
  pCreateTable->type = TSQL_CREATE_TABLE_FROM_STABLE;
  yylhsminor.yy170 = pCreateTable;
}
  yymsp[0].minor.yy170 = yylhsminor.yy170;
        break;
      case 144: /* create_table_list ::= create_table_list create_from_stable */
{
  taosArrayPush(yymsp[-1].minor.yy170->childTableInfo, &yymsp[0].minor.yy72);
  yylhsminor.yy170 = yymsp[-1].minor.yy170;
}
  yymsp[-1].minor.yy170 = yylhsminor.yy170;
        break;
      case 145: /* create_table_args ::= ifnotexists ids cpxName LP columnlist RP */
{
  yylhsminor.yy170 = tSetCreateTableInfo(yymsp[-1].minor.yy345, NULL, NULL, TSQL_CREATE_TABLE);
  setSqlInfo(pInfo, yylhsminor.yy170, NULL, TSDB_SQL_CREATE_TABLE);

  yymsp[-4].minor.yy0.n += yymsp[-3].minor.yy0.n;
  setCreatedTableName(pInfo, &yymsp[-4].minor.yy0, &yymsp[-5].minor.yy0);
}
  yymsp[-5].minor.yy170 = yylhsminor.yy170;
        break;
      case 146: /* create_stable_args ::= ifnotexists ids cpxName LP columnlist RP TAGS LP columnlist RP */
{
  yylhsminor.yy170 = tSetCreateTableInfo(yymsp[-5].minor.yy345, yymsp[-1].minor.yy345, NULL, TSQL_CREATE_STABLE);
  setSqlInfo(pInfo, yylhsminor.yy170, NULL, TSDB_SQL_CREATE_TABLE);

  yymsp[-8].minor.yy0.n += yymsp[-7].minor.yy0.n;
  setCreatedTableName(pInfo, &yymsp[-8].minor.yy0, &yymsp[-9].minor.yy0);
}
  yymsp[-9].minor.yy170 = yylhsminor.yy170;
        break;
      case 147: /* create_from_stable ::= ifnotexists ids cpxName USING ids cpxName TAGS LP tagitemlist RP */
{
  yymsp[-5].minor.yy0.n += yymsp[-4].minor.yy0.n;
  yymsp[-8].minor.yy0.n += yymsp[-7].minor.yy0.n;
  yylhsminor.yy72 = createNewChildTableInfo(&yymsp[-5].minor.yy0, NULL, yymsp[-1].minor.yy345, &yymsp[-8].minor.yy0, &yymsp[-9].minor.yy0);
}
  yymsp[-9].minor.yy72 = yylhsminor.yy72;
        break;
      case 148: /* create_from_stable ::= ifnotexists ids cpxName USING ids cpxName LP tagNamelist RP TAGS LP tagitemlist RP */
{
  yymsp[-8].minor.yy0.n += yymsp[-7].minor.yy0.n;
  yymsp[-11].minor.yy0.n += yymsp[-10].minor.yy0.n;
  yylhsminor.yy72 = createNewChildTableInfo(&yymsp[-8].minor.yy0, yymsp[-5].minor.yy345, yymsp[-1].minor.yy345, &yymsp[-11].minor.yy0, &yymsp[-12].minor.yy0);
}
  yymsp[-12].minor.yy72 = yylhsminor.yy72;
        break;
      case 149: /* tagNamelist ::= tagNamelist COMMA ids */
{taosArrayPush(yymsp[-2].minor.yy345, &yymsp[0].minor.yy0); yylhsminor.yy345 = yymsp[-2].minor.yy345;  }
  yymsp[-2].minor.yy345 = yylhsminor.yy345;
        break;
      case 150: /* tagNamelist ::= ids */
{yylhsminor.yy345 = taosArrayInit(4, sizeof(SStrToken)); taosArrayPush(yylhsminor.yy345, &yymsp[0].minor.yy0);}
  yymsp[0].minor.yy345 = yylhsminor.yy345;
        break;
      case 151: /* create_table_args ::= ifnotexists ids cpxName to_opt split_opt AS select */
{
  yylhsminor.yy170 = tSetCreateTableInfo(NULL, NULL, yymsp[0].minor.yy68, TSQL_CREATE_STREAM);
  setSqlInfo(pInfo, yylhsminor.yy170, NULL, TSDB_SQL_CREATE_TABLE);

  setCreatedStreamOpt(pInfo, &yymsp[-3].minor.yy0, &yymsp[-2].minor.yy0);
  yymsp[-5].minor.yy0.n += yymsp[-4].minor.yy0.n;
  setCreatedTableName(pInfo, &yymsp[-5].minor.yy0, &yymsp[-6].minor.yy0);
}
  yymsp[-6].minor.yy170 = yylhsminor.yy170;
        break;
      case 152: /* to_opt ::= */
      case 154: /* split_opt ::= */ yytestcase(yyruleno==154);
{yymsp[1].minor.yy0.n = 0;}
        break;
      case 153: /* to_opt ::= TO ids cpxName */
{
   yymsp[-2].minor.yy0 = yymsp[-1].minor.yy0;
   yymsp[-2].minor.yy0.n += yymsp[0].minor.yy0.n;
}
        break;
      case 155: /* split_opt ::= SPLIT ids */
{ yymsp[-1].minor.yy0 = yymsp[0].minor.yy0;}
        break;
      case 156: /* columnlist ::= columnlist COMMA column */
{taosArrayPush(yymsp[-2].minor.yy345, &yymsp[0].minor.yy487); yylhsminor.yy345 = yymsp[-2].minor.yy345;  }
  yymsp[-2].minor.yy345 = yylhsminor.yy345;
        break;
      case 157: /* columnlist ::= column */
{yylhsminor.yy345 = taosArrayInit(4, sizeof(TAOS_FIELD)); taosArrayPush(yylhsminor.yy345, &yymsp[0].minor.yy487);}
  yymsp[0].minor.yy345 = yylhsminor.yy345;
        break;
      case 158: /* column ::= ids typename */
{
  tSetColumnInfo(&yylhsminor.yy487, &yymsp[-1].minor.yy0, &yymsp[0].minor.yy487);
}
  yymsp[-1].minor.yy487 = yylhsminor.yy487;
        break;
      case 165: /* tagitem ::= NULL */
{ yymsp[0].minor.yy0.type = 0; tVariantCreate(&yylhsminor.yy2, &yymsp[0].minor.yy0); }
  yymsp[0].minor.yy2 = yylhsminor.yy2;
        break;
      case 166: /* tagitem ::= NOW */
{ yymsp[0].minor.yy0.type = TSDB_DATA_TYPE_TIMESTAMP; tVariantCreate(&yylhsminor.yy2, &yymsp[0].minor.yy0);}
  yymsp[0].minor.yy2 = yylhsminor.yy2;
        break;
      case 167: /* tagitem ::= MINUS INTEGER */
      case 168: /* tagitem ::= MINUS FLOAT */ yytestcase(yyruleno==168);
      case 169: /* tagitem ::= PLUS INTEGER */ yytestcase(yyruleno==169);
      case 170: /* tagitem ::= PLUS FLOAT */ yytestcase(yyruleno==170);
{
    yymsp[-1].minor.yy0.n += yymsp[0].minor.yy0.n;
    yymsp[-1].minor.yy0.type = yymsp[0].minor.yy0.type;
    toTSDBType(yymsp[-1].minor.yy0.type);
    tVariantCreate(&yylhsminor.yy2, &yymsp[-1].minor.yy0);
}
  yymsp[-1].minor.yy2 = yylhsminor.yy2;
        break;
      case 171: /* select ::= SELECT selcollist from where_opt interval_option sliding_opt session_option windowstate_option fill_opt groupby_opt having_opt orderby_opt slimit_opt limit_opt */
{
  yylhsminor.yy68 = tSetQuerySqlNode(&yymsp[-13].minor.yy0, yymsp[-12].minor.yy345, yymsp[-11].minor.yy484, yymsp[-10].minor.yy418, yymsp[-4].minor.yy345, yymsp[-2].minor.yy345, &yymsp[-9].minor.yy280, &yymsp[-7].minor.yy295, &yymsp[-6].minor.yy432, &yymsp[-8].minor.yy0, yymsp[-5].minor.yy345, &yymsp[0].minor.yy114, &yymsp[-1].minor.yy114, yymsp[-3].minor.yy418);
}
  yymsp[-13].minor.yy68 = yylhsminor.yy68;
        break;
      case 172: /* select ::= LP select RP */
{yymsp[-2].minor.yy68 = yymsp[-1].minor.yy68;}
        break;
      case 173: /* union ::= select */
{ yylhsminor.yy345 = setSubclause(NULL, yymsp[0].minor.yy68); }
  yymsp[0].minor.yy345 = yylhsminor.yy345;
        break;
      case 174: /* union ::= union UNION ALL select */
{ yylhsminor.yy345 = appendSelectClause(yymsp[-3].minor.yy345, yymsp[0].minor.yy68); }
  yymsp[-3].minor.yy345 = yylhsminor.yy345;
        break;
      case 175: /* cmd ::= union */
{ setSqlInfo(pInfo, yymsp[0].minor.yy345, NULL, TSDB_SQL_SELECT); }
        break;
      case 176: /* select ::= SELECT selcollist */
{
  yylhsminor.yy68 = tSetQuerySqlNode(&yymsp[-1].minor.yy0, yymsp[0].minor.yy345, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
  yymsp[-1].minor.yy68 = yylhsminor.yy68;
        break;
      case 177: /* sclp ::= selcollist COMMA */
{yylhsminor.yy345 = yymsp[-1].minor.yy345;}
  yymsp[-1].minor.yy345 = yylhsminor.yy345;
        break;
      case 178: /* sclp ::= */
      case 210: /* orderby_opt ::= */ yytestcase(yyruleno==210);
{yymsp[1].minor.yy345 = 0;}
        break;
      case 179: /* selcollist ::= sclp distinct expr as */
{
   yylhsminor.yy345 = tSqlExprListAppend(yymsp[-3].minor.yy345, yymsp[-1].minor.yy418,  yymsp[-2].minor.yy0.n? &yymsp[-2].minor.yy0:0, yymsp[0].minor.yy0.n?&yymsp[0].minor.yy0:0);
}
  yymsp[-3].minor.yy345 = yylhsminor.yy345;
        break;
      case 180: /* selcollist ::= sclp STAR */
{
   tSqlExpr *pNode = tSqlExprCreateIdValue(NULL, TK_ALL);
   yylhsminor.yy345 = tSqlExprListAppend(yymsp[-1].minor.yy345, pNode, 0, 0);
}
  yymsp[-1].minor.yy345 = yylhsminor.yy345;
        break;
      case 181: /* as ::= AS ids */
{ yymsp[-1].minor.yy0 = yymsp[0].minor.yy0;    }
        break;
      case 182: /* as ::= ids */
{ yylhsminor.yy0 = yymsp[0].minor.yy0;    }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 183: /* as ::= */
{ yymsp[1].minor.yy0.n = 0;  }
        break;
      case 184: /* distinct ::= DISTINCT */
{ yylhsminor.yy0 = yymsp[0].minor.yy0;  }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 186: /* from ::= FROM tablelist */
      case 187: /* from ::= FROM sub */ yytestcase(yyruleno==187);
{yymsp[-1].minor.yy484 = yymsp[0].minor.yy484;}
        break;
      case 188: /* sub ::= LP union RP */
{yymsp[-2].minor.yy484 = addSubqueryElem(NULL, yymsp[-1].minor.yy345, NULL);}
        break;
      case 189: /* sub ::= LP union RP ids */
{yymsp[-3].minor.yy484 = addSubqueryElem(NULL, yymsp[-2].minor.yy345, &yymsp[0].minor.yy0);}
        break;
      case 190: /* sub ::= sub COMMA LP union RP ids */
{yylhsminor.yy484 = addSubqueryElem(yymsp[-5].minor.yy484, yymsp[-2].minor.yy345, &yymsp[0].minor.yy0);}
  yymsp[-5].minor.yy484 = yylhsminor.yy484;
        break;
      case 191: /* tablelist ::= ids cpxName */
{
  yymsp[-1].minor.yy0.n += yymsp[0].minor.yy0.n;
  yylhsminor.yy484 = setTableNameList(NULL, &yymsp[-1].minor.yy0, NULL);
}
  yymsp[-1].minor.yy484 = yylhsminor.yy484;
        break;
      case 192: /* tablelist ::= ids cpxName ids */
{
  yymsp[-2].minor.yy0.n += yymsp[-1].minor.yy0.n;
  yylhsminor.yy484 = setTableNameList(NULL, &yymsp[-2].minor.yy0, &yymsp[0].minor.yy0);
}
  yymsp[-2].minor.yy484 = yylhsminor.yy484;
        break;
      case 193: /* tablelist ::= tablelist COMMA ids cpxName */
{
  yymsp[-1].minor.yy0.n += yymsp[0].minor.yy0.n;
  yylhsminor.yy484 = setTableNameList(yymsp[-3].minor.yy484, &yymsp[-1].minor.yy0, NULL);
}
  yymsp[-3].minor.yy484 = yylhsminor.yy484;
        break;
      case 194: /* tablelist ::= tablelist COMMA ids cpxName ids */
{
  yymsp[-2].minor.yy0.n += yymsp[-1].minor.yy0.n;
  yylhsminor.yy484 = setTableNameList(yymsp[-4].minor.yy484, &yymsp[-2].minor.yy0, &yymsp[0].minor.yy0);
}
  yymsp[-4].minor.yy484 = yylhsminor.yy484;
        break;
      case 195: /* tmvar ::= VARIABLE */
{yylhsminor.yy0 = yymsp[0].minor.yy0;}
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 196: /* interval_option ::= intervalKey LP tmvar RP */
{yylhsminor.yy280.interval = yymsp[-1].minor.yy0; yylhsminor.yy280.offset.n = 0; yylhsminor.yy280.token = yymsp[-3].minor.yy40;}
  yymsp[-3].minor.yy280 = yylhsminor.yy280;
        break;
      case 197: /* interval_option ::= intervalKey LP tmvar COMMA tmvar RP */
{yylhsminor.yy280.interval = yymsp[-3].minor.yy0; yylhsminor.yy280.offset = yymsp[-1].minor.yy0;   yylhsminor.yy280.token = yymsp[-5].minor.yy40;}
  yymsp[-5].minor.yy280 = yylhsminor.yy280;
        break;
      case 198: /* interval_option ::= */
{memset(&yymsp[1].minor.yy280, 0, sizeof(yymsp[1].minor.yy280));}
        break;
      case 199: /* intervalKey ::= INTERVAL */
{yymsp[0].minor.yy40 = TK_INTERVAL;}
        break;
      case 200: /* intervalKey ::= EVERY */
{yymsp[0].minor.yy40 = TK_EVERY;   }
        break;
      case 201: /* session_option ::= */
{yymsp[1].minor.yy295.col.n = 0; yymsp[1].minor.yy295.gap.n = 0;}
        break;
      case 202: /* session_option ::= SESSION LP ids cpxName COMMA tmvar RP */
{
   yymsp[-4].minor.yy0.n += yymsp[-3].minor.yy0.n;
   yymsp[-6].minor.yy295.col = yymsp[-4].minor.yy0;
   yymsp[-6].minor.yy295.gap = yymsp[-1].minor.yy0;
}
        break;
      case 203: /* windowstate_option ::= */
{ yymsp[1].minor.yy432.col.n = 0; yymsp[1].minor.yy432.col.z = NULL;}
        break;
      case 204: /* windowstate_option ::= STATE_WINDOW LP ids RP */
{ yymsp[-3].minor.yy432.col = yymsp[-1].minor.yy0; }
        break;
      case 205: /* fill_opt ::= */
{ yymsp[1].minor.yy345 = 0;     }
        break;
      case 206: /* fill_opt ::= FILL LP ID COMMA tagitemlist RP */
{
    tVariant A = {0};
    toTSDBType(yymsp[-3].minor.yy0.type);
    tVariantCreate(&A, &yymsp[-3].minor.yy0);

    tVariantListInsert(yymsp[-1].minor.yy345, &A, -1, 0);
    yymsp[-5].minor.yy345 = yymsp[-1].minor.yy345;
}
        break;
      case 207: /* fill_opt ::= FILL LP ID RP */
{
    toTSDBType(yymsp[-1].minor.yy0.type);
    yymsp[-3].minor.yy345 = tVariantListAppendToken(NULL, &yymsp[-1].minor.yy0, -1);
}
        break;
      case 208: /* sliding_opt ::= SLIDING LP tmvar RP */
{yymsp[-3].minor.yy0 = yymsp[-1].minor.yy0;     }
        break;
      case 209: /* sliding_opt ::= */
{yymsp[1].minor.yy0.n = 0; yymsp[1].minor.yy0.z = NULL; yymsp[1].minor.yy0.type = 0;   }
        break;
      case 211: /* orderby_opt ::= ORDER BY sortlist */
{yymsp[-2].minor.yy345 = yymsp[0].minor.yy345;}
        break;
      case 212: /* sortlist ::= sortlist COMMA item sortorder */
{
    yylhsminor.yy345 = tVariantListAppend(yymsp[-3].minor.yy345, &yymsp[-1].minor.yy2, yymsp[0].minor.yy281);
}
  yymsp[-3].minor.yy345 = yylhsminor.yy345;
        break;
      case 213: /* sortlist ::= item sortorder */
{
  yylhsminor.yy345 = tVariantListAppend(NULL, &yymsp[-1].minor.yy2, yymsp[0].minor.yy281);
}
  yymsp[-1].minor.yy345 = yylhsminor.yy345;
        break;
      case 214: /* item ::= ids cpxName */
{
  toTSDBType(yymsp[-1].minor.yy0.type);
  yymsp[-1].minor.yy0.n += yymsp[0].minor.yy0.n;

  tVariantCreate(&yylhsminor.yy2, &yymsp[-1].minor.yy0);
}
  yymsp[-1].minor.yy2 = yylhsminor.yy2;
        break;
      case 215: /* sortorder ::= ASC */
{ yymsp[0].minor.yy281 = TSDB_ORDER_ASC; }
        break;
      case 216: /* sortorder ::= DESC */
{ yymsp[0].minor.yy281 = TSDB_ORDER_DESC;}
        break;
      case 217: /* sortorder ::= */
{ yymsp[1].minor.yy281 = TSDB_ORDER_ASC; }
        break;
      case 218: /* groupby_opt ::= */
{ yymsp[1].minor.yy345 = 0;}
        break;
      case 219: /* groupby_opt ::= GROUP BY grouplist */
{ yymsp[-2].minor.yy345 = yymsp[0].minor.yy345;}
        break;
      case 220: /* grouplist ::= grouplist COMMA item */
{
  yylhsminor.yy345 = tVariantListAppend(yymsp[-2].minor.yy345, &yymsp[0].minor.yy2, -1);
}
  yymsp[-2].minor.yy345 = yylhsminor.yy345;
        break;
      case 221: /* grouplist ::= item */
{
  yylhsminor.yy345 = tVariantListAppend(NULL, &yymsp[0].minor.yy2, -1);
}
  yymsp[0].minor.yy345 = yylhsminor.yy345;
        break;
      case 222: /* having_opt ::= */
      case 232: /* where_opt ::= */ yytestcase(yyruleno==232);
      case 274: /* expritem ::= */ yytestcase(yyruleno==274);
{yymsp[1].minor.yy418 = 0;}
        break;
      case 223: /* having_opt ::= HAVING expr */
      case 233: /* where_opt ::= WHERE expr */ yytestcase(yyruleno==233);
{yymsp[-1].minor.yy418 = yymsp[0].minor.yy418;}
        break;
      case 224: /* limit_opt ::= */
      case 228: /* slimit_opt ::= */ yytestcase(yyruleno==228);
{yymsp[1].minor.yy114.limit = -1; yymsp[1].minor.yy114.offset = 0;}
        break;
      case 225: /* limit_opt ::= LIMIT signed */
      case 229: /* slimit_opt ::= SLIMIT signed */ yytestcase(yyruleno==229);
{yymsp[-1].minor.yy114.limit = yymsp[0].minor.yy525;  yymsp[-1].minor.yy114.offset = 0;}
        break;
      case 226: /* limit_opt ::= LIMIT signed OFFSET signed */
{ yymsp[-3].minor.yy114.limit = yymsp[-2].minor.yy525;  yymsp[-3].minor.yy114.offset = yymsp[0].minor.yy525;}
        break;
      case 227: /* limit_opt ::= LIMIT signed COMMA signed */
{ yymsp[-3].minor.yy114.limit = yymsp[0].minor.yy525;  yymsp[-3].minor.yy114.offset = yymsp[-2].minor.yy525;}
        break;
      case 230: /* slimit_opt ::= SLIMIT signed SOFFSET signed */
{yymsp[-3].minor.yy114.limit = yymsp[-2].minor.yy525;  yymsp[-3].minor.yy114.offset = yymsp[0].minor.yy525;}
        break;
      case 231: /* slimit_opt ::= SLIMIT signed COMMA signed */
{yymsp[-3].minor.yy114.limit = yymsp[0].minor.yy525;  yymsp[-3].minor.yy114.offset = yymsp[-2].minor.yy525;}
        break;
      case 234: /* expr ::= LP expr RP */
{yylhsminor.yy418 = yymsp[-1].minor.yy418; yylhsminor.yy418->exprToken.z = yymsp[-2].minor.yy0.z; yylhsminor.yy418->exprToken.n = (yymsp[0].minor.yy0.z - yymsp[-2].minor.yy0.z + 1);}
  yymsp[-2].minor.yy418 = yylhsminor.yy418;
        break;
      case 235: /* expr ::= ID */
{ yylhsminor.yy418 = tSqlExprCreateIdValue(&yymsp[0].minor.yy0, TK_ID);}
  yymsp[0].minor.yy418 = yylhsminor.yy418;
        break;
      case 236: /* expr ::= ID DOT ID */
{ yymsp[-2].minor.yy0.n += (1+yymsp[0].minor.yy0.n); yylhsminor.yy418 = tSqlExprCreateIdValue(&yymsp[-2].minor.yy0, TK_ID);}
  yymsp[-2].minor.yy418 = yylhsminor.yy418;
        break;
      case 237: /* expr ::= ID DOT STAR */
{ yymsp[-2].minor.yy0.n += (1+yymsp[0].minor.yy0.n); yylhsminor.yy418 = tSqlExprCreateIdValue(&yymsp[-2].minor.yy0, TK_ALL);}
  yymsp[-2].minor.yy418 = yylhsminor.yy418;
        break;
      case 238: /* expr ::= INTEGER */
{ yylhsminor.yy418 = tSqlExprCreateIdValue(&yymsp[0].minor.yy0, TK_INTEGER);}
  yymsp[0].minor.yy418 = yylhsminor.yy418;
        break;
      case 239: /* expr ::= MINUS INTEGER */
      case 240: /* expr ::= PLUS INTEGER */ yytestcase(yyruleno==240);
{ yymsp[-1].minor.yy0.n += yymsp[0].minor.yy0.n; yymsp[-1].minor.yy0.type = TK_INTEGER; yylhsminor.yy418 = tSqlExprCreateIdValue(&yymsp[-1].minor.yy0, TK_INTEGER);}
  yymsp[-1].minor.yy418 = yylhsminor.yy418;
        break;
      case 241: /* expr ::= FLOAT */
{ yylhsminor.yy418 = tSqlExprCreateIdValue(&yymsp[0].minor.yy0, TK_FLOAT);}
  yymsp[0].minor.yy418 = yylhsminor.yy418;
        break;
      case 242: /* expr ::= MINUS FLOAT */
      case 243: /* expr ::= PLUS FLOAT */ yytestcase(yyruleno==243);
{ yymsp[-1].minor.yy0.n += yymsp[0].minor.yy0.n; yymsp[-1].minor.yy0.type = TK_FLOAT; yylhsminor.yy418 = tSqlExprCreateIdValue(&yymsp[-1].minor.yy0, TK_FLOAT);}
  yymsp[-1].minor.yy418 = yylhsminor.yy418;
        break;
      case 244: /* expr ::= STRING */
{ yylhsminor.yy418 = tSqlExprCreateIdValue(&yymsp[0].minor.yy0, TK_STRING);}
  yymsp[0].minor.yy418 = yylhsminor.yy418;
        break;
      case 245: /* expr ::= NOW */
{ yylhsminor.yy418 = tSqlExprCreateIdValue(&yymsp[0].minor.yy0, TK_NOW); }
  yymsp[0].minor.yy418 = yylhsminor.yy418;
        break;
      case 246: /* expr ::= VARIABLE */
{ yylhsminor.yy418 = tSqlExprCreateIdValue(&yymsp[0].minor.yy0, TK_VARIABLE);}
  yymsp[0].minor.yy418 = yylhsminor.yy418;
        break;
      case 247: /* expr ::= PLUS VARIABLE */
      case 248: /* expr ::= MINUS VARIABLE */ yytestcase(yyruleno==248);
{ yymsp[-1].minor.yy0.n += yymsp[0].minor.yy0.n; yymsp[-1].minor.yy0.type = TK_VARIABLE; yylhsminor.yy418 = tSqlExprCreateIdValue(&yymsp[-1].minor.yy0, TK_VARIABLE);}
  yymsp[-1].minor.yy418 = yylhsminor.yy418;
        break;
      case 249: /* expr ::= BOOL */
{ yylhsminor.yy418 = tSqlExprCreateIdValue(&yymsp[0].minor.yy0, TK_BOOL);}
  yymsp[0].minor.yy418 = yylhsminor.yy418;
        break;
      case 250: /* expr ::= NULL */
{ yylhsminor.yy418 = tSqlExprCreateIdValue(&yymsp[0].minor.yy0, TK_NULL);}
  yymsp[0].minor.yy418 = yylhsminor.yy418;
        break;
      case 251: /* expr ::= ID LP exprlist RP */
{ tStrTokenAppend(pInfo->funcs, &yymsp[-3].minor.yy0); yylhsminor.yy418 = tSqlExprCreateFunction(yymsp[-1].minor.yy345, &yymsp[-3].minor.yy0, &yymsp[0].minor.yy0, yymsp[-3].minor.yy0.type); }
  yymsp[-3].minor.yy418 = yylhsminor.yy418;
        break;
      case 252: /* expr ::= ID LP STAR RP */
{ tStrTokenAppend(pInfo->funcs, &yymsp[-3].minor.yy0); yylhsminor.yy418 = tSqlExprCreateFunction(NULL, &yymsp[-3].minor.yy0, &yymsp[0].minor.yy0, yymsp[-3].minor.yy0.type); }
  yymsp[-3].minor.yy418 = yylhsminor.yy418;
        break;
      case 253: /* expr ::= expr IS NULL */
{yylhsminor.yy418 = tSqlExprCreate(yymsp[-2].minor.yy418, NULL, TK_ISNULL);}
  yymsp[-2].minor.yy418 = yylhsminor.yy418;
        break;
      case 254: /* expr ::= expr IS NOT NULL */
{yylhsminor.yy418 = tSqlExprCreate(yymsp[-3].minor.yy418, NULL, TK_NOTNULL);}
  yymsp[-3].minor.yy418 = yylhsminor.yy418;
        break;
      case 255: /* expr ::= expr LT expr */
{yylhsminor.yy418 = tSqlExprCreate(yymsp[-2].minor.yy418, yymsp[0].minor.yy418, TK_LT);}
  yymsp[-2].minor.yy418 = yylhsminor.yy418;
        break;
      case 256: /* expr ::= expr GT expr */
{yylhsminor.yy418 = tSqlExprCreate(yymsp[-2].minor.yy418, yymsp[0].minor.yy418, TK_GT);}
  yymsp[-2].minor.yy418 = yylhsminor.yy418;
        break;
      case 257: /* expr ::= expr LE expr */
{yylhsminor.yy418 = tSqlExprCreate(yymsp[-2].minor.yy418, yymsp[0].minor.yy418, TK_LE);}
  yymsp[-2].minor.yy418 = yylhsminor.yy418;
        break;
      case 258: /* expr ::= expr GE expr */
{yylhsminor.yy418 = tSqlExprCreate(yymsp[-2].minor.yy418, yymsp[0].minor.yy418, TK_GE);}
  yymsp[-2].minor.yy418 = yylhsminor.yy418;
        break;
      case 259: /* expr ::= expr NE expr */
{yylhsminor.yy418 = tSqlExprCreate(yymsp[-2].minor.yy418, yymsp[0].minor.yy418, TK_NE);}
  yymsp[-2].minor.yy418 = yylhsminor.yy418;
        break;
      case 260: /* expr ::= expr EQ expr */
{yylhsminor.yy418 = tSqlExprCreate(yymsp[-2].minor.yy418, yymsp[0].minor.yy418, TK_EQ);}
  yymsp[-2].minor.yy418 = yylhsminor.yy418;
        break;
      case 261: /* expr ::= expr BETWEEN expr AND expr */
{ tSqlExpr* X2 = tSqlExprClone(yymsp[-4].minor.yy418); yylhsminor.yy418 = tSqlExprCreate(tSqlExprCreate(yymsp[-4].minor.yy418, yymsp[-2].minor.yy418, TK_GE), tSqlExprCreate(X2, yymsp[0].minor.yy418, TK_LE), TK_AND);}
  yymsp[-4].minor.yy418 = yylhsminor.yy418;
        break;
      case 262: /* expr ::= expr AND expr */
{yylhsminor.yy418 = tSqlExprCreate(yymsp[-2].minor.yy418, yymsp[0].minor.yy418, TK_AND);}
  yymsp[-2].minor.yy418 = yylhsminor.yy418;
        break;
      case 263: /* expr ::= expr OR expr */
{yylhsminor.yy418 = tSqlExprCreate(yymsp[-2].minor.yy418, yymsp[0].minor.yy418, TK_OR); }
  yymsp[-2].minor.yy418 = yylhsminor.yy418;
        break;
      case 264: /* expr ::= expr PLUS expr */
{yylhsminor.yy418 = tSqlExprCreate(yymsp[-2].minor.yy418, yymsp[0].minor.yy418, TK_PLUS);  }
  yymsp[-2].minor.yy418 = yylhsminor.yy418;
        break;
      case 265: /* expr ::= expr MINUS expr */
{yylhsminor.yy418 = tSqlExprCreate(yymsp[-2].minor.yy418, yymsp[0].minor.yy418, TK_MINUS); }
  yymsp[-2].minor.yy418 = yylhsminor.yy418;
        break;
      case 266: /* expr ::= expr STAR expr */
{yylhsminor.yy418 = tSqlExprCreate(yymsp[-2].minor.yy418, yymsp[0].minor.yy418, TK_STAR);  }
  yymsp[-2].minor.yy418 = yylhsminor.yy418;
        break;
      case 267: /* expr ::= expr SLASH expr */
{yylhsminor.yy418 = tSqlExprCreate(yymsp[-2].minor.yy418, yymsp[0].minor.yy418, TK_DIVIDE);}
  yymsp[-2].minor.yy418 = yylhsminor.yy418;
        break;
      case 268: /* expr ::= expr REM expr */
{yylhsminor.yy418 = tSqlExprCreate(yymsp[-2].minor.yy418, yymsp[0].minor.yy418, TK_REM);   }
  yymsp[-2].minor.yy418 = yylhsminor.yy418;
        break;
      case 269: /* expr ::= expr LIKE expr */
{yylhsminor.yy418 = tSqlExprCreate(yymsp[-2].minor.yy418, yymsp[0].minor.yy418, TK_LIKE);  }
  yymsp[-2].minor.yy418 = yylhsminor.yy418;
        break;
      case 270: /* expr ::= expr IN LP exprlist RP */
{yylhsminor.yy418 = tSqlExprCreate(yymsp[-4].minor.yy418, (tSqlExpr*)yymsp[-1].minor.yy345, TK_IN); }
  yymsp[-4].minor.yy418 = yylhsminor.yy418;
        break;
      case 271: /* exprlist ::= exprlist COMMA expritem */
{yylhsminor.yy345 = tSqlExprListAppend(yymsp[-2].minor.yy345,yymsp[0].minor.yy418,0, 0);}
  yymsp[-2].minor.yy345 = yylhsminor.yy345;
        break;
      case 272: /* exprlist ::= expritem */
{yylhsminor.yy345 = tSqlExprListAppend(0,yymsp[0].minor.yy418,0, 0);}
  yymsp[0].minor.yy345 = yylhsminor.yy345;
        break;
      case 273: /* expritem ::= expr */
{yylhsminor.yy418 = yymsp[0].minor.yy418;}
  yymsp[0].minor.yy418 = yylhsminor.yy418;
        break;
      case 275: /* cmd ::= RESET QUERY CACHE */
{ setDCLSqlElems(pInfo, TSDB_SQL_RESET_CACHE, 0);}
        break;
      case 276: /* cmd ::= SYNCDB ids REPLICA */
{ setDCLSqlElems(pInfo, TSDB_SQL_SYNC_DB_REPLICA, 1, &yymsp[-1].minor.yy0);}
        break;
      case 277: /* cmd ::= ALTER TABLE ids cpxName ADD COLUMN columnlist */
{
    yymsp[-4].minor.yy0.n += yymsp[-3].minor.yy0.n;
    SAlterTableInfo* pAlterTable = tSetAlterTableInfo(&yymsp[-4].minor.yy0, yymsp[0].minor.yy345, NULL, TSDB_ALTER_TABLE_ADD_COLUMN, -1);
    setSqlInfo(pInfo, pAlterTable, NULL, TSDB_SQL_ALTER_TABLE);
}
        break;
      case 278: /* cmd ::= ALTER TABLE ids cpxName DROP COLUMN ids */
{
    yymsp[-4].minor.yy0.n += yymsp[-3].minor.yy0.n;

    toTSDBType(yymsp[0].minor.yy0.type);
    SArray* K = tVariantListAppendToken(NULL, &yymsp[0].minor.yy0, -1);

    SAlterTableInfo* pAlterTable = tSetAlterTableInfo(&yymsp[-4].minor.yy0, NULL, K, TSDB_ALTER_TABLE_DROP_COLUMN, -1);
    setSqlInfo(pInfo, pAlterTable, NULL, TSDB_SQL_ALTER_TABLE);
}
        break;
      case 279: /* cmd ::= ALTER TABLE ids cpxName MODIFY COLUMN columnlist */
{
    yymsp[-4].minor.yy0.n += yymsp[-3].minor.yy0.n;
    SAlterTableInfo* pAlterTable = tSetAlterTableInfo(&yymsp[-4].minor.yy0, yymsp[0].minor.yy345, NULL, TSDB_ALTER_TABLE_CHANGE_COLUMN, -1);
    setSqlInfo(pInfo, pAlterTable, NULL, TSDB_SQL_ALTER_TABLE);
}
        break;
      case 280: /* cmd ::= ALTER TABLE ids cpxName ADD TAG columnlist */
{
    yymsp[-4].minor.yy0.n += yymsp[-3].minor.yy0.n;
    SAlterTableInfo* pAlterTable = tSetAlterTableInfo(&yymsp[-4].minor.yy0, yymsp[0].minor.yy345, NULL, TSDB_ALTER_TABLE_ADD_TAG_COLUMN, -1);
    setSqlInfo(pInfo, pAlterTable, NULL, TSDB_SQL_ALTER_TABLE);
}
        break;
      case 281: /* cmd ::= ALTER TABLE ids cpxName DROP TAG ids */
{
    yymsp[-4].minor.yy0.n += yymsp[-3].minor.yy0.n;

    toTSDBType(yymsp[0].minor.yy0.type);
    SArray* A = tVariantListAppendToken(NULL, &yymsp[0].minor.yy0, -1);

    SAlterTableInfo* pAlterTable = tSetAlterTableInfo(&yymsp[-4].minor.yy0, NULL, A, TSDB_ALTER_TABLE_DROP_TAG_COLUMN, -1);
    setSqlInfo(pInfo, pAlterTable, NULL, TSDB_SQL_ALTER_TABLE);
}
        break;
      case 282: /* cmd ::= ALTER TABLE ids cpxName CHANGE TAG ids ids */
{
    yymsp[-5].minor.yy0.n += yymsp[-4].minor.yy0.n;

    toTSDBType(yymsp[-1].minor.yy0.type);
    SArray* A = tVariantListAppendToken(NULL, &yymsp[-1].minor.yy0, -1);

    toTSDBType(yymsp[0].minor.yy0.type);
    A = tVariantListAppendToken(A, &yymsp[0].minor.yy0, -1);

    SAlterTableInfo* pAlterTable = tSetAlterTableInfo(&yymsp[-5].minor.yy0, NULL, A, TSDB_ALTER_TABLE_CHANGE_TAG_COLUMN, -1);
    setSqlInfo(pInfo, pAlterTable, NULL, TSDB_SQL_ALTER_TABLE);
}
        break;
      case 283: /* cmd ::= ALTER TABLE ids cpxName SET TAG ids EQ tagitem */
{
    yymsp[-6].minor.yy0.n += yymsp[-5].minor.yy0.n;

    toTSDBType(yymsp[-2].minor.yy0.type);
    SArray* A = tVariantListAppendToken(NULL, &yymsp[-2].minor.yy0, -1);
    A = tVariantListAppend(A, &yymsp[0].minor.yy2, -1);

    SAlterTableInfo* pAlterTable = tSetAlterTableInfo(&yymsp[-6].minor.yy0, NULL, A, TSDB_ALTER_TABLE_UPDATE_TAG_VAL, -1);
    setSqlInfo(pInfo, pAlterTable, NULL, TSDB_SQL_ALTER_TABLE);
}
        break;
      case 284: /* cmd ::= ALTER TABLE ids cpxName MODIFY TAG columnlist */
{
    yymsp[-4].minor.yy0.n += yymsp[-3].minor.yy0.n;
    SAlterTableInfo* pAlterTable = tSetAlterTableInfo(&yymsp[-4].minor.yy0, yymsp[0].minor.yy345, NULL, TSDB_ALTER_TABLE_MODIFY_TAG_COLUMN, -1);
    setSqlInfo(pInfo, pAlterTable, NULL, TSDB_SQL_ALTER_TABLE);
}
        break;
      case 285: /* cmd ::= ALTER STABLE ids cpxName ADD COLUMN columnlist */
{
    yymsp[-4].minor.yy0.n += yymsp[-3].minor.yy0.n;
    SAlterTableInfo* pAlterTable = tSetAlterTableInfo(&yymsp[-4].minor.yy0, yymsp[0].minor.yy345, NULL, TSDB_ALTER_TABLE_ADD_COLUMN, TSDB_SUPER_TABLE);
    setSqlInfo(pInfo, pAlterTable, NULL, TSDB_SQL_ALTER_TABLE);
}
        break;
      case 286: /* cmd ::= ALTER STABLE ids cpxName DROP COLUMN ids */
{
    yymsp[-4].minor.yy0.n += yymsp[-3].minor.yy0.n;

    toTSDBType(yymsp[0].minor.yy0.type);
    SArray* K = tVariantListAppendToken(NULL, &yymsp[0].minor.yy0, -1);

    SAlterTableInfo* pAlterTable = tSetAlterTableInfo(&yymsp[-4].minor.yy0, NULL, K, TSDB_ALTER_TABLE_DROP_COLUMN, TSDB_SUPER_TABLE);
    setSqlInfo(pInfo, pAlterTable, NULL, TSDB_SQL_ALTER_TABLE);
}
        break;
      case 287: /* cmd ::= ALTER STABLE ids cpxName MODIFY COLUMN columnlist */
{
    yymsp[-4].minor.yy0.n += yymsp[-3].minor.yy0.n;
    SAlterTableInfo* pAlterTable = tSetAlterTableInfo(&yymsp[-4].minor.yy0, yymsp[0].minor.yy345, NULL, TSDB_ALTER_TABLE_CHANGE_COLUMN, TSDB_SUPER_TABLE);
    setSqlInfo(pInfo, pAlterTable, NULL, TSDB_SQL_ALTER_TABLE);
}
        break;
      case 288: /* cmd ::= ALTER STABLE ids cpxName ADD TAG columnlist */
{
    yymsp[-4].minor.yy0.n += yymsp[-3].minor.yy0.n;
    SAlterTableInfo* pAlterTable = tSetAlterTableInfo(&yymsp[-4].minor.yy0, yymsp[0].minor.yy345, NULL, TSDB_ALTER_TABLE_ADD_TAG_COLUMN, TSDB_SUPER_TABLE);
    setSqlInfo(pInfo, pAlterTable, NULL, TSDB_SQL_ALTER_TABLE);
}
        break;
      case 289: /* cmd ::= ALTER STABLE ids cpxName DROP TAG ids */
{
    yymsp[-4].minor.yy0.n += yymsp[-3].minor.yy0.n;

    toTSDBType(yymsp[0].minor.yy0.type);
    SArray* A = tVariantListAppendToken(NULL, &yymsp[0].minor.yy0, -1);

    SAlterTableInfo* pAlterTable = tSetAlterTableInfo(&yymsp[-4].minor.yy0, NULL, A, TSDB_ALTER_TABLE_DROP_TAG_COLUMN, TSDB_SUPER_TABLE);
    setSqlInfo(pInfo, pAlterTable, NULL, TSDB_SQL_ALTER_TABLE);
}
        break;
      case 290: /* cmd ::= ALTER STABLE ids cpxName CHANGE TAG ids ids */
{
    yymsp[-5].minor.yy0.n += yymsp[-4].minor.yy0.n;

    toTSDBType(yymsp[-1].minor.yy0.type);
    SArray* A = tVariantListAppendToken(NULL, &yymsp[-1].minor.yy0, -1);

    toTSDBType(yymsp[0].minor.yy0.type);
    A = tVariantListAppendToken(A, &yymsp[0].minor.yy0, -1);

    SAlterTableInfo* pAlterTable = tSetAlterTableInfo(&yymsp[-5].minor.yy0, NULL, A, TSDB_ALTER_TABLE_CHANGE_TAG_COLUMN, TSDB_SUPER_TABLE);
    setSqlInfo(pInfo, pAlterTable, NULL, TSDB_SQL_ALTER_TABLE);
}
        break;
      case 291: /* cmd ::= ALTER STABLE ids cpxName SET TAG ids EQ tagitem */
{
    yymsp[-6].minor.yy0.n += yymsp[-5].minor.yy0.n;

    toTSDBType(yymsp[-2].minor.yy0.type);
    SArray* A = tVariantListAppendToken(NULL, &yymsp[-2].minor.yy0, -1);
    A = tVariantListAppend(A, &yymsp[0].minor.yy2, -1);

    SAlterTableInfo* pAlterTable = tSetAlterTableInfo(&yymsp[-6].minor.yy0, NULL, A, TSDB_ALTER_TABLE_UPDATE_TAG_VAL, TSDB_SUPER_TABLE);
    setSqlInfo(pInfo, pAlterTable, NULL, TSDB_SQL_ALTER_TABLE);
}
        break;
      case 292: /* cmd ::= ALTER STABLE ids cpxName MODIFY TAG columnlist */
{
    yymsp[-4].minor.yy0.n += yymsp[-3].minor.yy0.n;
    SAlterTableInfo* pAlterTable = tSetAlterTableInfo(&yymsp[-4].minor.yy0, yymsp[0].minor.yy345, NULL, TSDB_ALTER_TABLE_MODIFY_TAG_COLUMN, TSDB_SUPER_TABLE);
    setSqlInfo(pInfo, pAlterTable, NULL, TSDB_SQL_ALTER_TABLE);
}
        break;
      case 293: /* cmd ::= KILL CONNECTION INTEGER */
{setKillSql(pInfo, TSDB_SQL_KILL_CONNECTION, &yymsp[0].minor.yy0);}
        break;
      case 294: /* cmd ::= KILL STREAM INTEGER COLON INTEGER */
{yymsp[-2].minor.yy0.n += (yymsp[-1].minor.yy0.n + yymsp[0].minor.yy0.n); setKillSql(pInfo, TSDB_SQL_KILL_STREAM, &yymsp[-2].minor.yy0);}
        break;
      case 295: /* cmd ::= KILL QUERY INTEGER COLON INTEGER */
{yymsp[-2].minor.yy0.n += (yymsp[-1].minor.yy0.n + yymsp[0].minor.yy0.n); setKillSql(pInfo, TSDB_SQL_KILL_QUERY, &yymsp[-2].minor.yy0);}
        break;
      default:
        break;
/********** End reduce actions ************************************************/
  };
  assert( yyruleno<sizeof(yyRuleInfo)/sizeof(yyRuleInfo[0]) );
  yygoto = yyRuleInfo[yyruleno].lhs;
  yysize = yyRuleInfo[yyruleno].nrhs;
  yyact = yy_find_reduce_action(yymsp[yysize].stateno,(YYCODETYPE)yygoto);

  /* There are no SHIFTREDUCE actions on nonterminals because the table
  ** generator has simplified them to pure REDUCE actions. */
  assert( !(yyact>YY_MAX_SHIFT && yyact<=YY_MAX_SHIFTREDUCE) );

  /* It is not possible for a REDUCE to be followed by an error */
  assert( yyact!=YY_ERROR_ACTION );

  yymsp += yysize+1;
  yypParser->yytos = yymsp;
  yymsp->stateno = (YYACTIONTYPE)yyact;
  yymsp->major = (YYCODETYPE)yygoto;
  yyTraceShift(yypParser, yyact, "... then shift");
  return yyact;
}

/*
** The following code executes when the parse fails
*/
#ifndef YYNOERRORRECOVERY
static void yy_parse_failed(
  yyParser *yypParser           /* The parser */
){
  ParseARG_FETCH
  ParseCTX_FETCH
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sFail!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yytos>yypParser->yystack ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser fails */
/************ Begin %parse_failure code ***************************************/
/************ End %parse_failure code *****************************************/
  ParseARG_STORE /* Suppress warning about unused %extra_argument variable */
  ParseCTX_STORE
}
#endif /* YYNOERRORRECOVERY */

/*
** The following code executes when a syntax error first occurs.
*/
static void yy_syntax_error(
  yyParser *yypParser,           /* The parser */
  int yymajor,                   /* The major type of the error token */
  ParseTOKENTYPE yyminor         /* The minor type of the error token */
){
  ParseARG_FETCH
  ParseCTX_FETCH
#define TOKEN yyminor
/************ Begin %syntax_error code ****************************************/

  pInfo->valid = false;
  int32_t outputBufLen = tListLen(pInfo->msg);
  int32_t len = 0;

  if(TOKEN.z) {
    char msg[] = "syntax error near \"%s\"";
    int32_t sqlLen = strlen(&TOKEN.z[0]);

    if (sqlLen + sizeof(msg)/sizeof(msg[0]) + 1 > outputBufLen) {
        char tmpstr[128] = {0};
        memcpy(tmpstr, &TOKEN.z[0], sizeof(tmpstr)/sizeof(tmpstr[0]) - 1);
        len = sprintf(pInfo->msg, msg, tmpstr);
    } else {
        len = sprintf(pInfo->msg, msg, &TOKEN.z[0]);
    }

  } else {
    len = sprintf(pInfo->msg, "Incomplete SQL statement");
  }

  assert(len <= outputBufLen);
/************ End %syntax_error code ******************************************/
  ParseARG_STORE /* Suppress warning about unused %extra_argument variable */
  ParseCTX_STORE
}

/*
** The following is executed when the parser accepts
*/
static void yy_accept(
  yyParser *yypParser           /* The parser */
){
  ParseARG_FETCH
  ParseCTX_FETCH
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sAccept!\n",yyTracePrompt);
  }
#endif
#ifndef YYNOERRORRECOVERY
  yypParser->yyerrcnt = -1;
#endif
  assert( yypParser->yytos==yypParser->yystack );
  /* Here code is inserted which will be executed whenever the
  ** parser accepts */
/*********** Begin %parse_accept code *****************************************/

/*********** End %parse_accept code *******************************************/
  ParseARG_STORE /* Suppress warning about unused %extra_argument variable */
  ParseCTX_STORE
}

/* The main parser program.
** The first argument is a pointer to a structure obtained from
** "ParseAlloc" which describes the current state of the parser.
** The second argument is the major token number.  The third is
** the minor token.  The fourth optional argument is whatever the
** user wants (and specified in the grammar) and is available for
** use by the action routines.
**
** Inputs:
** <ul>
** <li> A pointer to the parser (an opaque structure.)
** <li> The major token number.
** <li> The minor token number.
** <li> An option argument of a grammar-specified type.
** </ul>
**
** Outputs:
** None.
*/
void Parse(
  void *yyp,                   /* The parser */
  int yymajor,                 /* The major token code number */
  ParseTOKENTYPE yyminor       /* The value for the token */
  ParseARG_PDECL               /* Optional %extra_argument parameter */
){
  YYMINORTYPE yyminorunion;
  YYACTIONTYPE yyact;   /* The parser action. */
#if !defined(YYERRORSYMBOL) && !defined(YYNOERRORRECOVERY)
  int yyendofinput;     /* True if we are at the end of input */
#endif
#ifdef YYERRORSYMBOL
  int yyerrorhit = 0;   /* True if yymajor has invoked an error */
#endif
  yyParser *yypParser = (yyParser*)yyp;  /* The parser */
  ParseCTX_FETCH
  ParseARG_STORE

  assert( yypParser->yytos!=0 );
#if !defined(YYERRORSYMBOL) && !defined(YYNOERRORRECOVERY)
  yyendofinput = (yymajor==0);
#endif

  yyact = yypParser->yytos->stateno;
#ifndef NDEBUG
  if( yyTraceFILE ){
    if( yyact < YY_MIN_REDUCE ){
      fprintf(yyTraceFILE,"%sInput '%s' in state %d\n",
              yyTracePrompt,yyTokenName[yymajor],yyact);
    }else{
      fprintf(yyTraceFILE,"%sInput '%s' with pending reduce %d\n",
              yyTracePrompt,yyTokenName[yymajor],yyact-YY_MIN_REDUCE);
    }
  }
#endif

  do{
    assert( yyact==yypParser->yytos->stateno );
    yyact = yy_find_shift_action((YYCODETYPE)yymajor,yyact);
    if( yyact >= YY_MIN_REDUCE ){
      yyact = yy_reduce(yypParser,yyact-YY_MIN_REDUCE,yymajor,
                        yyminor ParseCTX_PARAM);
    }else if( yyact <= YY_MAX_SHIFTREDUCE ){
      yy_shift(yypParser,yyact,(YYCODETYPE)yymajor,yyminor);
#ifndef YYNOERRORRECOVERY
      yypParser->yyerrcnt--;
#endif
      break;
    }else if( yyact==YY_ACCEPT_ACTION ){
      yypParser->yytos--;
      yy_accept(yypParser);
      return;
    }else{
      assert( yyact == YY_ERROR_ACTION );
      yyminorunion.yy0 = yyminor;
#ifdef YYERRORSYMBOL
      int yymx;
#endif
#ifndef NDEBUG
      if( yyTraceFILE ){
        fprintf(yyTraceFILE,"%sSyntax Error!\n",yyTracePrompt);
      }
#endif
#ifdef YYERRORSYMBOL
      /* A syntax error has occurred.
      ** The response to an error depends upon whether or not the
      ** grammar defines an error token "ERROR".  
      **
      ** This is what we do if the grammar does define ERROR:
      **
      **  * Call the %syntax_error function.
      **
      **  * Begin popping the stack until we enter a state where
      **    it is legal to shift the error symbol, then shift
      **    the error symbol.
      **
      **  * Set the error count to three.
      **
      **  * Begin accepting and shifting new tokens.  No new error
      **    processing will occur until three tokens have been
      **    shifted successfully.
      **
      */
      if( yypParser->yyerrcnt<0 ){
        yy_syntax_error(yypParser,yymajor,yyminor);
      }
      yymx = yypParser->yytos->major;
      if( yymx==YYERRORSYMBOL || yyerrorhit ){
#ifndef NDEBUG
        if( yyTraceFILE ){
          fprintf(yyTraceFILE,"%sDiscard input token %s\n",
             yyTracePrompt,yyTokenName[yymajor]);
        }
#endif
        yy_destructor(yypParser, (YYCODETYPE)yymajor, &yyminorunion);
        yymajor = YYNOCODE;
      }else{
        while( yypParser->yytos >= yypParser->yystack
            && (yyact = yy_find_reduce_action(
                        yypParser->yytos->stateno,
                        YYERRORSYMBOL)) > YY_MAX_SHIFTREDUCE
        ){
          yy_pop_parser_stack(yypParser);
        }
        if( yypParser->yytos < yypParser->yystack || yymajor==0 ){
          yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
          yy_parse_failed(yypParser);
#ifndef YYNOERRORRECOVERY
          yypParser->yyerrcnt = -1;
#endif
          yymajor = YYNOCODE;
        }else if( yymx!=YYERRORSYMBOL ){
          yy_shift(yypParser,yyact,YYERRORSYMBOL,yyminor);
        }
      }
      yypParser->yyerrcnt = 3;
      yyerrorhit = 1;
      if( yymajor==YYNOCODE ) break;
      yyact = yypParser->yytos->stateno;
#elif defined(YYNOERRORRECOVERY)
      /* If the YYNOERRORRECOVERY macro is defined, then do not attempt to
      ** do any kind of error recovery.  Instead, simply invoke the syntax
      ** error routine and continue going as if nothing had happened.
      **
      ** Applications can set this macro (for example inside %include) if
      ** they intend to abandon the parse upon the first syntax error seen.
      */
      yy_syntax_error(yypParser,yymajor, yyminor);
      yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
      break;
#else  /* YYERRORSYMBOL is not defined */
      /* This is what we do if the grammar does not define ERROR:
      **
      **  * Report an error message, and throw away the input token.
      **
      **  * If the input token is $, then fail the parse.
      **
      ** As before, subsequent error messages are suppressed until
      ** three input tokens have been successfully shifted.
      */
      if( yypParser->yyerrcnt<=0 ){
        yy_syntax_error(yypParser,yymajor, yyminor);
      }
      yypParser->yyerrcnt = 3;
      yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
      if( yyendofinput ){
        yy_parse_failed(yypParser);
#ifndef YYNOERRORRECOVERY
        yypParser->yyerrcnt = -1;
#endif
      }
      break;
#endif
    }
  }while( yypParser->yytos>yypParser->yystack );
#ifndef NDEBUG
  if( yyTraceFILE ){
    yyStackEntry *i;
    char cDiv = '[';
    fprintf(yyTraceFILE,"%sReturn. Stack=",yyTracePrompt);
    for(i=&yypParser->yystack[1]; i<=yypParser->yytos; i++){
      fprintf(yyTraceFILE,"%c%s", cDiv, yyTokenName[i->major]);
      cDiv = ' ';
    }
    fprintf(yyTraceFILE,"]\n");
  }
#endif
  return;
}

/*
** Return the fallback token corresponding to canonical token iToken, or
** 0 if iToken has no fallback.
*/
int ParseFallback(int iToken){
#ifdef YYFALLBACK
  if( iToken<(int)(sizeof(yyFallback)/sizeof(yyFallback[0])) ){
    return yyFallback[iToken];
  }
#else
  (void)iToken;
#endif
  return 0;
}

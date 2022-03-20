/* printf with debug level functionality
 *
 *              Author   : Viki (a) Vignesh Natarajan
 *              Contact  : neoviki.com
 *              LICENCE  : MIT 
 */

#ifndef NEOVIKI_PRINTF_H
#define NEOVIKI_PRINTF_H

/*enable new line character in printf*/
//#define ENABLE_NEWLINE

/*Enable function name and line number in print*/
#define ENABLE_FUNC_NAME_N_LINE_NO

/*set the debug level display orientation ( by default it is left )*/
//#define PRINT_ORIENT_RIGHT

#define NEOVIKI_PRINT_PREFIX "[ "
#define NEOVIKI_PRINT_SUFFIX " ]"

#ifdef PRINT_ORIENT_RIGHT
    #define SUCC "  success"
    #define FAIL "  failure"
    #define ERRO "    error"
    #define INFO "     info"
    #define CRIC " critical"
    #define ALER "    alert"
    #define NOTC "   notice"
    #define WARN "  warning"
    #define DEBG "    debug"
#else
    #define SUCC "success  "
    #define FAIL "failure  "
    #define ERRO "error    "
    #define INFO "info     "
    #define CRIC "critical "
    #define ALER "alert    "
    #define NOTC "notice   "
    #define WARN "warning  "
    #define DEBG "debug    "
#endif

typedef enum{
    debug=1,
    info,
    critical,
    success,
    failure,
    error,
    warning,
    notice,
    alert,
}debug_level_t;

#define neoviki_printf(debug_level, format, ...) __neoviki_printf__(debug_level, __func__, __LINE__, format,  ##__VA_ARGS__)

//void neoviki_printf(debug_level_t debug_level,  char *format, ...);
void __neoviki_printf__(debug_level_t debug_level,  char const *fn, long lineno, char *format, ...);
void neoviki_debug(int status);
extern int __neoviki_debug__;

#endif /*NEOVIKI_PRINTF_H*/

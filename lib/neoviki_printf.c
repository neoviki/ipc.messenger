/* printf with debug level functionality
 *
 *              Author   : Viki (a) Vignesh Natarajan
 *              Contact  : neoviki.com
 *              LICENCE  : MIT 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdarg.h>
#include "neoviki_printf.h"

int __neoviki_debug__;
/* status = 1 -> enable  debug
 * status = 0 -> disable debug
 */
void neoviki_debug(int status)
{
    if(status == 1){
        __neoviki_debug__ = 1;
    }else{
        __neoviki_debug__ = 0;
    }
}


void __neoviki_printf__(debug_level_t debug_level, char const *fn, long lineno,  char *format, ...)
{
    va_list argument_list;
    char buff[512], prepend[16];
    
    switch(debug_level){
        case success  :   
            if(__neoviki_debug__ == 0) return;
            strcpy(prepend, SUCC); 
            break;
        case failure  :   
            strcpy(prepend, FAIL); break;
        case error    :   
            strcpy(prepend, ERRO); break;
        case info     :   
            if(__neoviki_debug__ == 0) return;
            strcpy(prepend, INFO); break;
        case critical :   
            strcpy(prepend, CRIC); break;
        case alert    :   
            strcpy(prepend, ALER); break;
        case notice   :   
            if(__neoviki_debug__ == 0) return;
            strcpy(prepend, NOTC); break;
        case warning  :   
            strcpy(prepend, WARN); break;
        case debug    :  
            /*don't print if class is debug and neoviki_printf_debug is disabled*/
            if(__neoviki_debug__ == 0) return;
            strcpy(prepend, DEBG);
            break;
        default :   strcpy(prepend, INFO); break;
    }


    va_start(argument_list, format);
    vsprintf(buff, format, argument_list);
    va_end(argument_list);

#ifndef ENABLE_NEWLINE

#ifdef ENABLE_FUNC_NAME_N_LINE_NO
    printf("%s%s# %s:%ld%s %s", NEOVIKI_PRINT_PREFIX, prepend, fn, lineno, NEOVIKI_PRINT_SUFFIX, buff);
#endif

#ifndef ENABLE_FUNC_NAME_N_LINE_NO
    printf("%s%s%s %s", NEOVIKI_PRINT_PREFIX, prepend, NEOVIKI_PRINT_SUFFIX, buff);
#endif

#endif


#ifdef ENABLE_NEWLINE
#ifdef ENABLE_FUNC_NAME_N_LINE_NO
    printf("%s%s# %s:%ld%s %s\n", NEOVIKI_PRINT_PREFIX, prepend, fn, lineno, NEOVIKI_PRINT_SUFFIX, buff);
#endif

#ifndef ENABLE_FUNC_NAME_N_LINE_NO
    printf("%s%s%s %s\n", NEOVIKI_PRINT_PREFIX, prepend, NEOVIKI_PRINT_SUFFIX, buff);
#endif
#endif
}



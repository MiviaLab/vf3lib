/*----------------------------------------------------
 * error.h
 * Header of error.cc
 * Error handling functions
 *
 * Author: P. Foggia
 * $Id: error.h,v 1.2 1998/12/12 12:17:32 foggia Exp $
 *--------------------------------------------------*/

/*----------------------------------------------------
 * REVISION HISTORY
 *   $Log: error.h,v $
 *   Revision 1.2  1998/12/12 12:17:32  foggia
 *   Now supports full printf syntax
 *
 *   Revision 1.1  1998/09/16 17:35:14  foggia
 *   Initial revision
 *
 ---------------------------------------------------*/

#ifndef ERROR_H

#include <stddef.h>
#include <string>
#include <stdarg.h>
/*------------------------------------------
 * void error(msg, ...)
 * Prints an error message and exits
 * the program.
 * The syntax is the same of printf,
 * except that a trailing \n is automatically
 * appended.
 -----------------------------------------*/
void error(std::string msg, ...)
{
  va_list ap;
  va_start(ap, msg);
  fprintf(stderr, "ERROR: ");
  vfprintf(stderr, (char *)msg.data(), ap);
  fprintf(stderr, "\n");
  va_end(ap);
  exit(1);
}


#define FAIL(reason)    error("%s in %s:%d", (reason), __FILE__, \
                                                          (int)__LINE__)

#define OUT_OF_MEMORY()  FAIL("Out of memory")
#define CANT_HAPPEN()    FAIL("Can't happen")




#endif

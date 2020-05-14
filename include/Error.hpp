/*----------------------------------------------------
 * @file   ARGLoader.hpp
 * @author P. Foggia (pfoggia\@unisa.it)
 * @author V.Carletti (vcarletti\@unisa.it)
 * @date  December, 2014 
 * @brief Error handling functions
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
  *   Revision 2.0 2017/10/21 carletti
  *   Created Error Printer Functor
  ---------------------------------------------------*/

#ifndef ERROR_H
#define ERROR_H

#include <stddef.h>
#include <string>
#include <stdarg.h>

  /**
  * @fn error
  * @brief Prints an error message and exits the program.
  * @details The syntax is the same of printf,
  *	except that a trailing \n is automatically appended.
  */
void error (std::string msg, ...)
	{
		va_list ap;
		va_start(ap, msg);
		fprintf(stderr, "ERROR: ");
		vfprintf(stderr, (char *)msg.data(), ap);
		fprintf(stderr, "\n");
		va_end(ap);
		exit(1);
	}

#endif

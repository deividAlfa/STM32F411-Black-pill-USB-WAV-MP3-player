/*
 * stdio_.h
 *
 *  Created on: Apr 29, 2021
 *      Author: David
 */

#ifndef INC_STDIO__H_
#define INC_STDIO__H_



/*
 * stdio.c
 *
 *  Created on: Apr 29, 2021
 *      Author: David
 */

#ifndef SRC_STDIO_C_
#define SRC_STDIO_C_

/**************************************************************************//*****
 * @file     printf.c
 * @brief    Implementation of several stdio.h methods, such as printf(),
 *           sprintf() and so on. This reduces the memory footprint of the
 *           binary when using those methods, compared to the libc implementation.
 ********************************************************************************/
void PrintChar_(char c);
signed int PutChar_(char *pStr, char c);
signed int PutString_(char *pStr, const char *pSource);
signed int PutUnsignedInt_(char *pStr,char fill,signed int width,unsigned int value);
signed int PutSignedInt_(char *pStr,char fill,signed int width,unsigned int value);
signed int PutHexa_(char *pStr,char fill,signed int width,unsigned int value);
signed int vsnprintf_(char *pStr, size_t length, const char *pFormat, va_list ap);
signed int snprintf_(char *pString, size_t length, const char *pFormat, ...);
signed int vsprintf_(char *pString, const char *pFormat, va_list ap);
signed int vfprintf_(FILE *pStream, const char *pFormat, va_list ap);
signed int vprintf_(const char *pFormat, va_list ap);
signed int fprintf_(FILE *pStream, const char *pFormat, ...);
signed int printf_(const char *pFormat, ...);
signed int sprintf_(char *pStr, const char *pFormat, ...);
signed int puts_(const char *pStr);
signed int fputc_(signed int c, FILE *pStream)
signed int fputs_(const char *pStr, FILE *pStream);

#endif /* SRC_STDIO_C_ */




#endif /* INC_STDIO__H_ */

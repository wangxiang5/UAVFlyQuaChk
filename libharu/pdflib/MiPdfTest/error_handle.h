#include "hpdf.h"

#ifndef _ERROR_HANDLE_HEAD_
#define _ERROR_HANDLE_HEAD_
#ifdef HPDF_DLL
void  __stdcall
#else
void
#endif
error_handler  (HPDF_STATUS   error_no,
                HPDF_STATUS   detail_no,
                void         *user_data);
#else
#ifdef HPDF_DLL
extern void  __stdcall
#else
extern void
#endif 
error_handler  (HPDF_STATUS   error_no,
                HPDF_STATUS   detail_no,
                void         *user_data);
#endif

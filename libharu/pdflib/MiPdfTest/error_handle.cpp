#include "stdafx.h"
#include "windows.h"
#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>

#include "error_handle.h"

jmp_buf env;

#ifdef HPDF_DLL
void  __stdcall
#else
void
#endif
error_handler  (HPDF_STATUS   error_no,
				HPDF_STATUS   detail_no,
				void         *user_data)
{
	printf ("ERROR: error_no=%04X, detail_no=%u\n", (HPDF_UINT)error_no,
		(HPDF_UINT)detail_no);
	longjmp(env, 1);
}
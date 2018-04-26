// MiPdfTest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "windows.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include <string>
#include <algorithm>
#include "hpdf.h"

#include "text_demo.h"
#include "raw_image_demo.h"

#include "error_handle.h"
using namespace std;

int _tmain1(int argc, _TCHAR* argv[])
{ 
	if(argc!=2) return -1;
	string strPath= argv[1];
	strPath="C:\\Users\\Administrator\\Desktop\\wx\\ssw\\sswUAVFlyQuaSys\\000.dat";
	raw_image_demo_main((char*)(strPath.c_str()));
	return 0;
}

jmp_buf env2;

void
	error_handler2  (HPDF_STATUS   error_no,
	HPDF_STATUS   detail_no,
	void         *user_data)
{
	printf ("ERROR: error_no=%04X, detail_no=%u/n", (HPDF_UINT)error_no,
		(HPDF_UINT)detail_no);
	longjmp(env2, 1);
}

void DisplayImage(HPDF_Doc& pdf, HPDF_Page& page, int x,int y,int nwidth, int nheight, string& imgPath)
{
	HPDF_Image hpdfImage = HPDF_LoadPngImageFromFile(pdf,imgPath.c_str());
	if (hpdfImage)
		HPDF_Page_DrawImage(page, hpdfImage,x,y,nwidth,nheight);
}
int main()
{
	  HPDF_Doc pdf;
    HPDF_Page page;
    HPDF_Destination dst;
    HPDF_Font font;
    HPDF_Image hpdfImage;
    
    pdf = HPDF_New (error_handler2, NULL); 
    if (!pdf) {
        printf ("ERROR: cannot create pdf object./n");
        return 1;
    }
    
    if (setjmp(env2)) {
        HPDF_Free (pdf);
        return 1;
    }

//    HPDF_SetCompressionMode (pdf, HPDF_COMP_ALL);

    font = HPDF_GetFont (pdf, "Helvetica", NULL);
    /* add a new page object. */
    page = HPDF_AddPage (pdf);
	int nPageWidth = HPDF_Page_GetWidth(page);
	int nPageHeight = HPDF_Page_GetHeight(page);
	int nwidth=200;
	int nheight=180;
	int x = (nPageWidth-nwidth)/2;
	int y = nPageHeight-nheight-30;
	DisplayImage(pdf,page,x,y,nwidth,nheight,string("00.png"));
    /* save the document to a file */
    HPDF_SaveToFile (pdf, "01.pdf");    
    /* clean up */
    HPDF_Free (pdf);
	return 0;
}


////////////////////show image in pdf///////////////////


// #include "windows.h"
// #include "include/hpdf.h"
// #include <setjmp.h>
// #include <stdio.h>
// 
// #pragma comment(lib, "lib/libhpdf.lib")
// #pragma comment(lib, "lib/zlib.lib")
// #pragma comment(lib, "lib/libpng.lib")
// 
// jmp_buf env;
// 
// void
// error_handler  (HPDF_STATUS   error_no,
//                 HPDF_STATUS   detail_no,
//                 void         *user_data)
// {
//     printf ("ERROR: error_no=%04X, detail_no=%u/n", (HPDF_UINT)error_no,
//         (HPDF_UINT)detail_no);
//     longjmp(env, 1);
// }
// 
// int main(int nArgc, char** pArgv)
// {
//     HPDF_Doc pdf;
//     HPDF_Page page;
//     HPDF_Destination dst;
//     HPDF_Font font;
//     HPDF_Image hpdfImage;
//     
//     pdf = HPDF_New (error_handler, NULL); 
//     if (!pdf) {
//         printf ("ERROR: cannot create pdf object./n");
//         return 1;
//     }
//     
//     if (setjmp(env)) {
//         HPDF_Free (pdf);
//         return 1;
//     }
// 
// //    HPDF_SetCompressionMode (pdf, HPDF_COMP_ALL);
// 
//     font = HPDF_GetFont (pdf, "Helvetica", NULL);
// 
//     char * pstrPath[2] = {
//         "e://a.png",
//         "e://read_book.png"
//     };
// 
//     hpdfImage = HPDF_LoadPngImageFromFile(pdf, pstrPath[0]);
// 
//     /* add a new page object. */
//     page = HPDF_AddPage (pdf);
// 
//     dst = HPDF_Page_CreateDestination (page);
//     HPDF_Destination_SetXYZ (dst, 0, HPDF_Page_GetHeight (page), 1);
//     HPDF_SetOpenAction(pdf, dst);
//     
//     HPDF_Page_SetHeight (page, HPDF_Image_GetHeight(hpdfImage));
//     HPDF_Page_SetWidth (page, HPDF_Image_GetWidth(hpdfImage));
// 
//     HPDF_Page_SetFontAndSize (page, font, 12);
// 
//     unsigned int oldWidth = HPDF_Image_GetWidth(hpdfImage);
// 
//     for (int i = 0; i < 2; i ++)
//     {
//         /*
//     
//         HPDF_Page_SetRGBFill (page, 1.0, 0, 0);
//         HPDF_Page_MoveTo (page, 100, 100);
//         HPDF_Page_LineTo (page, 100, 180);
//         */
// 
//     //    HPDF_SetPageMode(pdf, HPDF_PAGE_MODE_FULL_SCREEN);
//         hpdfImage = HPDF_LoadPngImageFromFile(pdf, pstrPath[i]);
//         
//         if (hpdfImage)
//         {
//             int x = 0;
//             int y = 0;
//             if (i == 1)
//             {
//                 x = oldWidth - HPDF_Image_GetWidth(hpdfImage) - 3;
//             }
// 
//             HPDF_Page_DrawImage(page, 
//                                 hpdfImage, 
//                                 x, y, 
//                                 HPDF_Image_GetWidth(hpdfImage),
//                                 HPDF_Image_GetHeight(hpdfImage));
//         }
//     }
// 
//     /* save the document to a file */
//     HPDF_SaveToFile (pdf, "e://test.pdf");    
//     /* clean up */
//     HPDF_Free (pdf);
// 
//     return 0;
// }




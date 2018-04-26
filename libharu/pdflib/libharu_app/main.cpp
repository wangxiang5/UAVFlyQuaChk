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

#define __WIN32__

void
draw_image (HPDF_Doc     pdf,
            HPDF_Page    page,
			const char  *filename,
			float        x,
			float        y,
			const char  *text)
{
#ifdef __WIN32__
	const char* FILE_SEPARATOR = "\\";
#else
	const char* FILE_SEPARATOR = "/";
#endif
	char filename1[255];

	//HPDF_Page page = HPDF_GetCurrentPage (pdf);
	HPDF_Image image;

	//strcpy(filename1, "images");
	//strcat(filename1, FILE_SEPARATOR);
	//strcat(filename1, filename);
  strcpy(filename1, filename);
  std::string strName = filename1;
  int pos = strName.find_last_of('.');
  strName = strName.substr(pos+1);  
  std::transform(strName.begin(),strName.end(),strName.begin(),tolower);
  if(strName == "png")
  {
      image = HPDF_LoadPngImageFromFile(pdf, filename1);
  }
  else if(strName == "jpg" || strName == "jpeg")
  {
      image = HPDF_LoadJpegImageFromFile (pdf, filename1);
  }    

	/* Draw image to the canvas. */
    HPDF_UINT widthImage = HPDF_Image_GetWidth (image);
    HPDF_UINT heightImage = HPDF_Image_GetHeight (image);
    HPDF_REAL heightPage = HPDF_Page_GetHeight (page);
    HPDF_REAL widthPage = HPDF_Page_GetWidth (page);

    std::string tmp = text;
    if(tmp == "center")
    {
        x = (widthPage-widthImage)/2;
        y = (heightPage-heightImage)/2;
    }

	HPDF_Page_DrawImage (page, image, x, y, widthImage, heightImage);    

	/* Print the text. */
	/*HPDF_Page_BeginText (page);    
	HPDF_Page_SetTextLeading (page, 16);
	HPDF_Page_MoveTextPos (page, x, y);
	HPDF_Page_ShowText (page, filename);
	HPDF_Page_ShowTextNextLine (page, text);
	HPDF_Page_EndText (page);*/
}


int main (int argc, char **argv)
{
  //main_text_demo(argc, argv);
    raw_image_demo_main(argc, argv);

#ifdef COMPILE_THIS_
    HPDF_Doc pdf;
    HPDF_Page page;
    HPDF_Destination dst;
    HPDF_Font font;
    HPDF_Image hpdfImage;
    
    pdf = HPDF_New (error_handler, NULL); 
    if (!pdf) {
        printf ("ERROR: cannot create pdf object./n");
        return 1;
    }
    
    if (setjmp(env)) {
        HPDF_Free (pdf);
        return 1;
    }

//    HPDF_SetCompressionMode (pdf, HPDF_COMP_ALL);

    font = HPDF_GetFont (pdf, "Helvetica", NULL);

    char * pstrPath[2] = {
        "e:\\a.png",
        "e:\\read_book.png"
    };

    hpdfImage = HPDF_LoadPngImageFromFile(pdf, pstrPath[0]);

    /* add a new page object. */
    page = HPDF_AddPage (pdf);

    dst = HPDF_Page_CreateDestination (page);
    HPDF_Destination_SetXYZ (dst, 0, HPDF_Page_GetHeight (page), 1);
    HPDF_SetOpenAction(pdf, dst);
    
    HPDF_Page_SetHeight (page, HPDF_Image_GetHeight(hpdfImage));
    HPDF_Page_SetWidth (page, HPDF_Image_GetWidth(hpdfImage));

    HPDF_Page_SetFontAndSize (page, font, 12);

    unsigned int oldWidth = HPDF_Image_GetWidth(hpdfImage);

    for (int i = 0; i < 2; i ++)
    {
        /*
    
        HPDF_Page_SetRGBFill (page, 1.0, 0, 0);
        HPDF_Page_MoveTo (page, 100, 100);
        HPDF_Page_LineTo (page, 100, 180);
        */

    //    HPDF_SetPageMode(pdf, HPDF_PAGE_MODE_FULL_SCREEN);
        hpdfImage = HPDF_LoadPngImageFromFile(pdf, pstrPath[i]);
        
        if (hpdfImage)
        {
            int x = 0;
            int y = 0;
            if (i == 1)
            {
                x = oldWidth - HPDF_Image_GetWidth(hpdfImage) - 3;
            }

            HPDF_Page_DrawImage(page, 
                                hpdfImage, 
                                x, y, 
                                HPDF_Image_GetWidth(hpdfImage),
                                HPDF_Image_GetHeight(hpdfImage));
        }
    }

    /* save the document to a file */
    HPDF_SaveToFile (pdf, "e://test.pdf");    
    /* clean up */
    HPDF_Free (pdf);
#endif

#ifdef COMPILE_THIS_
	HPDF_Doc  pdf;
	HPDF_Font font;
	HPDF_Page page;
	char fname[256];
	HPDF_Destination dst;

	strcpy (fname, argv[0]);
	strcat (fname, ".pdf");

	pdf = HPDF_New (error_handler, NULL);
	if (!pdf) {
		printf ("error: cannot create PdfDoc object\n");
		return 1;
	}

	/* error-handler */
	if (setjmp(env)) {
		HPDF_Free (pdf);
		return 1;
	}

	HPDF_SetCompressionMode (pdf, HPDF_COMP_ALL);

	/* create default-font */
	font = HPDF_GetFont (pdf, "Helvetica", NULL);

	/* add a new page object. */
	page = HPDF_AddPage (pdf);

	HPDF_Page_SetWidth (page, 650);
	HPDF_Page_SetHeight (page, 500);

	dst = HPDF_Page_CreateDestination (page);
	HPDF_Destination_SetXYZ (dst, 0, HPDF_Page_GetHeight (page), 1);
	HPDF_SetOpenAction(pdf, dst);

	HPDF_Page_BeginText (page);
	HPDF_Page_SetFontAndSize (page, font, 20);
	HPDF_Page_MoveTextPos (page, 220, HPDF_Page_GetHeight (page) - 70);
	HPDF_Page_ShowText (page, "JpegDemo");
	HPDF_Page_EndText (page);

	HPDF_Page_SetFontAndSize (page, font, 12);

	draw_image (pdf, "rgb.jpg", 70, HPDF_Page_GetHeight (page) - 410,
		"24bit color image");
	draw_image (pdf, "gray.jpg", 340, HPDF_Page_GetHeight (page) - 410,
		"8bit grayscale image");

	/* save the document to a file */
	HPDF_SaveToFile (pdf, fname);

	/* clean up */
	HPDF_Free (pdf);
#endif

	return 0;
}


#ifndef __GAUGES_H
#define __GAUGES_H

#include "../lib/e-Paper/EPD_IT8951.h"
#include "../lib/Config/DEV_Config.h"

// 1 bit per pixel, which is 2 grayscale
#define BitsPerPixel_1 1
// 2 bit per pixel, which is 4 grayscale 
#define BitsPerPixel_2 2
// 4 bit per pixel, which is 16 grayscale
#define BitsPerPixel_4 4
// 8 bit per pixel, which is 256 grayscale, but will automatically reduce by hardware to 4bpp, which is 16 grayscale
#define BitsPerPixel_8 8

extern UBYTE *Refresh_Frame_Buf;
extern UBYTE *Panel_Frame_Buf;
extern UBYTE *Panel_Area_Frame_Buf;
extern bool Four_Byte_Align;

UBYTE Start_Gauges(IT8951_Dev_Info Dev_Info, UDOUBLE Init_Target_Memory_Addr, char* server);
UBYTE Display_BMP(UWORD Panel_Width, UWORD Panel_Height, UDOUBLE Init_Target_Memory_Addr, UBYTE BitsPerPixel);
void Draw_STW(UWORD spd);
void Draw_WST(UWORD spd);
void Draw_wind_line(UWORD angle, UBYTE color);
void Draw_angles(UWORD angle);
void Draw_wind_angle(UWORD angle);
void Draw_depth(UWORD dpt);
void Draw_sectors(void);
void Draw_StartClock(UWORD sec);
void CleanCurl(void);

#endif

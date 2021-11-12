#include "../lib/Config/DEV_Config.h"
#include "gauges.h"
#include "../lib/GUI/GUI_BMPfile.h"
#include <math.h>
#include <stdlib.h>     //exit()
#include <signal.h>     //signal()

UWORD VCOM = 2510;
IT8951_Dev_Info Dev_Info;
UWORD Panel_Width;
UWORD Panel_Height;
UDOUBLE Init_Target_Memory_Addr;
int epd_mode = 0;	

void  Handler(int signo){
    Debug("\r\nHandler:exit\r\n");
    if(Refresh_Frame_Buf != NULL){
        free(Refresh_Frame_Buf);
        Debug("free Refresh_Frame_Buf\r\n");
        Refresh_Frame_Buf = NULL;
    }
    if(Panel_Frame_Buf != NULL){
        free(Panel_Frame_Buf);
        Debug("free Panel_Frame_Buf\r\n");
        Panel_Frame_Buf = NULL;
    }
    if(Panel_Area_Frame_Buf != NULL){
        free(Panel_Area_Frame_Buf);
        Debug("free Panel_Area_Frame_Buf\r\n");
        Panel_Area_Frame_Buf = NULL;
    }
    if(bmp_src_buf != NULL){
        free(bmp_src_buf);
        Debug("free bmp_src_buf\r\n");
        bmp_src_buf = NULL;
    }
    if(bmp_dst_buf != NULL){
        free(bmp_dst_buf);
        Debug("free bmp_dst_buf\r\n");
        bmp_dst_buf = NULL;
    }
    EPD_IT8951_Clear_Refresh(Dev_Info, Init_Target_Memory_Addr, INIT_Mode);
    CleanCurl();
    Display_BMP(Panel_Width, Panel_Height, Init_Target_Memory_Addr, BitsPerPixel_1); //Anchor pic when gauges not in use
    //We recommended refresh the panel to white color before storing in the warehouse. (After seiling season?)
    //EPD_IT8951_Clear_Refresh(Dev_Info, Init_Target_Memory_Addr, INIT_Mode);
    Debug("Going to sleep\r\n");
    EPD_IT8951_Sleep();
    DEV_Module_Exit();   
    exit(0);
}

int main() 
{
    signal(SIGINT, Handler);    
    if(DEV_Module_Init()!=0){
        return -1;
    }
    Dev_Info = EPD_IT8951_Init(VCOM);    
    Panel_Width = Dev_Info.Panel_W;
    Panel_Height = Dev_Info.Panel_H;    
    Init_Target_Memory_Addr = Dev_Info.Memory_Addr_L | (Dev_Info.Memory_Addr_H << 16);
    A2_Mode = 6;    
	EPD_IT8951_Clear_Refresh(Dev_Info, Init_Target_Memory_Addr, INIT_Mode);     
    Start_Gauges(Dev_Info,Init_Target_Memory_Addr);    
    return 0;
}

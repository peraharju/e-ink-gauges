/********************************************************
* | File      	:   gauges.c
* | Author      :   Pertti Harju / Waveshare electronics
* 
* | Info        :
*   Displays four values from Signal-K server 'deltas' 
*----------------
* |	This version:   V1.0
* | Date        :   2021-11-11
***************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <math.h>
#include <curl/curl.h>
#include <time.h>

#include "../lib/GUI/GUI_Paint.h"
#include "../lib/GUI/GUI_BMPfile.h"
#include "../lib/Config/Debug.h"
#include "gauges.h"

UBYTE *Refresh_Frame_Buf = NULL;
UBYTE *Panel_Frame_Buf = NULL;
UBYTE *Panel_Area_Frame_Buf = NULL;

extern int epd_mode;
UWORD tspeed, tdepth, ttws, twangle;
UWORD speed, depth, tws, wangle, iangle;
CURL *scurl;  
CURL *dcurl;
CURL *acurl;  
CURL *vcurl;

static void Epd_Mode(int mode) 
{	
	Paint_SetRotate(ROTATE_0);
	Paint_SetMirroring(MIRROR_NONE);	
}

//curl callbacks
size_t speedFunction(void* ptr, size_t size, size_t nmemb, void* data)
{ 
	float kn =  1.94384449 * atof(ptr); 
    speed = kn * 10;	
    return 0;
}
size_t depthFunction(void* ptr, size_t size, size_t nmemb, void* data)
{ 
	float dp =  atof(ptr); 
    depth =   dp * 10; 	 
    return 0;
}
size_t windFunction(void* ptr, size_t size, size_t nmemb, void* data)
{ 	 
    tws = atof(ptr) * 10; 	 
    return 0;
}
size_t angleFunction(void* ptr, size_t size, size_t nmemb, void* data)
{ 
	float wang =  atof(ptr) * 57.2957795;
	if(wang < 0){
		wangle = 360 - (-1 * wang);
	}
	else { 
    	wangle =  wang;
	}	
    return 0;
}
// end callbacks

UBYTE Start_Gauges(IT8951_Dev_Info Dev_Info, UDOUBLE Init_Target_Memory_Addr)
{
    UWORD Panel_Width = Dev_Info.Panel_W;
    UWORD Panel_Height = Dev_Info.Panel_H;
    UWORD Dynamic_Area_Width = 1200;
    UWORD Dynamic_Area_Height = 825;
	UWORD interval = 2000;
    UDOUBLE Imagesize;
    bool pr = true, dcounter = false, pres = false;
	UBYTE res, button;;
    FILE *fileStream;	    
    char modecommand[4];		
	unsigned int x_seconds=0;	
	unsigned int  time_left = 0;
	unsigned int count_down_time_in_secs = 299; //5 min mark 
	time_t x_startTime, x_countTime;

    Imagesize = ((Panel_Width * 1 % 8 == 0)? (Panel_Width * 1 / 8 ): (Panel_Width * 1 / 8 + 1)) * Panel_Height;
    if((Refresh_Frame_Buf = (UBYTE *)malloc(Imagesize)) == NULL){
        Debug("Failed to apply for picture memory...\r\n");
        return -1;
    }  
	Imagesize = ((Dynamic_Area_Width % 8 == 0)? (Dynamic_Area_Width / 8 ): (Dynamic_Area_Width / 8 + 1)) * Dynamic_Area_Height;
	Paint_NewImage(Refresh_Frame_Buf, Dynamic_Area_Width, Dynamic_Area_Height, 0, BLACK);
	Paint_SelectImage(Refresh_Frame_Buf);
	Epd_Mode(epd_mode);	
	Paint_SetBitsPerPixel(1); 	
	Paint_Clear(WHITE); 	
	Draw_sectors();
	EPD_IT8951_1bp_Refresh(Refresh_Frame_Buf, 0, 0, Dynamic_Area_Width, Dynamic_Area_Height, A2_Mode, Init_Target_Memory_Addr,false);
	scurl = curl_easy_init();
	dcurl = curl_easy_init();
	acurl = curl_easy_init();
	vcurl = curl_easy_init();	
	if(scurl){
    	curl_easy_setopt(scurl, CURLOPT_URL,"http://192.168.1.109:3000/signalk/v1/api/vessels/self/navigation/speedThroughWater/value");    
		curl_easy_setopt(scurl, CURLOPT_WRITEFUNCTION, speedFunction); 	
		curl_easy_setopt(scurl, CURLOPT_TIMEOUT,1L);			
  	}
  	if(dcurl){
		curl_easy_setopt(dcurl, CURLOPT_URL,"http://192.168.1.109:3000/signalk/v1/api/vessels/self/environment/depth/belowTransducer/value");    
		curl_easy_setopt(dcurl, CURLOPT_WRITEFUNCTION, depthFunction);
		curl_easy_setopt(dcurl, CURLOPT_TIMEOUT,1L);
	}		
	if(vcurl){
    	curl_easy_setopt(vcurl, CURLOPT_URL, "http://192.168.1.109:3000/signalk/v1/api/vessels/self/environment/wind/speedTrue/value");    
		curl_easy_setopt(vcurl, CURLOPT_WRITEFUNCTION, windFunction);
		curl_easy_setopt(vcurl, CURLOPT_TIMEOUT,1L);				
  	}	
  	if(acurl){
		curl_easy_setopt(acurl, CURLOPT_URL, "http://192.168.1.109:3000/signalk/v1/api/vessels/self/environment/wind/angleApparent/value");    
		curl_easy_setopt(acurl, CURLOPT_WRITEFUNCTION, angleFunction);
		curl_easy_setopt(acurl, CURLOPT_TIMEOUT,1L);
	}

	while(1){  // display values if changed		 								
		res = curl_easy_perform(acurl);			
		if(res == 7){ // no data
			wangle = 4000; 
		}							
		if(wangle != twangle){			
			Draw_wind_angle(wangle);			
			twangle = wangle;
			pr = true;
		}		
		res = curl_easy_perform(vcurl);
		if(res == 7){
			tws = 10000;
		}
		if(tws != ttws){
			Draw_WST(tws);
			ttws = tws;
			pr = true;
		}
		res = curl_easy_perform(scurl);
		if(res == 7){
			speed = 1000;
		}
		if(speed != tspeed){
			Draw_STW(speed);
			tspeed = speed;
			pr = true;
		}
		res = curl_easy_perform(dcurl);		
		if(res == 7){
			depth = -1;
		}		
		fileStream = fopen("modefile", "r");
		fgets (modecommand, 3, fileStream);
		fclose(fileStream);	
		button = DEV_Digital_Read(24); //5 min to start button pressed
		if(button == 0) pres = true;
		if(modecommand[0] == '5' || pres){									
			if(!dcounter){				
				x_startTime = time(NULL);
				Paint_ClearWindows(800, 440, 1100, 500, 0xFF);	
				Paint_DrawString_EN(800,440,"TIME TO START", &Font24, 0x00, 0xFF);
				dcounter = true;				
			}
			x_countTime = time(NULL);
			interval = 700;               						
			x_seconds = x_countTime - x_startTime;					
			time_left = count_down_time_in_secs - x_seconds;
			if(time_left < 1)
				Draw_StartClock(1100); //display -GO					
			else   
				Draw_StartClock(time_left);						
			if(x_seconds > count_down_time_in_secs + 15){ //display depth 15 sec after start
				fileStream = fopen("modefile", "w");
				fprintf(fileStream, "na\n");
				fclose(fileStream);
				pres = false;				
			}
			pr = true;			
		}					
		if(modecommand[0] != '5' && !pres){					
			if(depth != tdepth){
					Draw_depth(depth);
					tdepth = depth;
					pr = true;
			}
			if(dcounter){
				interval = 2000;
				count_down_time_in_secs = 299;
				dcounter = false;
				Paint_ClearWindows(800, 440, 1100, 500, 0xFF);	
				Paint_DrawString_EN(800,440,"DEPTH  M", &Font24, 0x00, 0xFF);
				depth = -1;
			}
		}	
		if(pr){ // do refresh if changes						
			EPD_IT8951_1bp_Refresh(Refresh_Frame_Buf, 0, 0, Dynamic_Area_Width, Dynamic_Area_Height, A2_Mode, Init_Target_Memory_Addr,false);			
			pr = false;
		}			
		DEV_Delay_ms(interval);			        														
	}	
    return 0;
}

void Draw_StartClock(UWORD seconds)
{	
	UWORD hour, min, sec;
	Paint_ClearWindows(670, 550, 1125, 750, 0xFF);
	if(seconds < 1000){
		hour = seconds/3600;
		min = (seconds -(3600*hour))/60;
		sec = (seconds -(3600*hour)-(min*60));		
		Paint_DrawCircle(826, 625, 10, 0x20, DOT_PIXEL_2X2, DRAW_FILL_FULL);
		Paint_DrawCircle(826, 675, 10, 0x20, DOT_PIXEL_2X2, DRAW_FILL_FULL);
		Draw7segNum(670,550, min, 140);
		Draw7segNum(985,550,(sec % 10),140);
		Draw7segNum(835,550,(sec/10) % 10,140);
	}
	else{ // -GO 
		Draw7segNum(670,550, 13, 140);
		Draw7segNum(985,550,0,140);
		Draw7segNum(835,550,6,140);
	}
}

void Draw_depth(UWORD dpt)
{
	Paint_ClearWindows(670, 550, 1125, 750, 0xFF);	
	if(dpt < 50000){			
		Paint_DrawCircle(965, 738, 10, 0x20, DOT_PIXEL_2X2, DRAW_FILL_FULL);
		Paint_DrawCircle(972, 746, 5, 0x20, DOT_PIXEL_2X2, DRAW_FILL_FULL);
		UBYTE wsten = (dpt/100) % 10;
		if(wsten > 0) 
			Draw7segNum(670,550, wsten, 140);
		Draw7segNum(985,550,(dpt % 10),140);
		Draw7segNum(820,550,(dpt/10) % 10,140);
	}
	else{		
		Draw7segNum(670,550,13,140);
		Draw7segNum(985,550,13,140);
		Draw7segNum(820,550,13,140);	
	}	
}

void Draw_STW(UWORD spd)
{
	Paint_ClearWindows(730, 80, 1130, 340, 0xFF);
	if(spd < 1000){
		Paint_DrawCircle(922, 320, 13, 0x20, DOT_PIXEL_2X2, DRAW_FILL_FULL);
		Paint_DrawCircle(930, 333, 6, 0x20, DOT_PIXEL_2X2, DRAW_FILL_FULL);
		Draw7segNum(940,80,spd % 10,180);
		Draw7segNum(740,80,(spd/10) % 10,180);
	}
	else{
		Draw7segNum(940,80,13,180);
		Draw7segNum(740,80,13,180);
	}	
}

void Draw_WST(UWORD spd)
{	
	Paint_ClearWindows(85, 640, 445, 810, 0xFF);
	if(spd < 10000){
		Paint_DrawCircle(315, 798, 9, 0x20, DOT_PIXEL_2X2, DRAW_FILL_FULL);
		Paint_DrawCircle(319, 807, 5, 0x20, DOT_PIXEL_2X2, DRAW_FILL_FULL); 
		UBYTE wsten = (spd/100) % 10;
		Draw7segNum(327,640,spd % 10,120);
		Draw7segNum(198,640,(spd/10) % 10,120);
		if(wsten > 0)   
			Draw7segNum(85,640, wsten, 120);
	}
	else{		
		Draw7segNum(327,640,13,120);
		Draw7segNum(198,640,13,120);		
		Draw7segNum(85,640,13, 120);
	}	
}

void Draw_wind_angle(UWORD angle)
{	
	UWORD sangle, ten;
	Paint_ClearWindows(120, 250, 422, 390, 0xFF);
	Paint_ClearWindows(284, 465, 340, 490, 0xFF);
	Paint_ClearWindows(285, 155, 335, 225, 0xFF);
	if(angle < 3900){	
		if(angle > 180){
			sangle = 360 - angle;
		}
		else {
			sangle = angle;
		}
		if(sangle >= 100)
			Draw7segNum(120, 250, 1, 100);
		ten = sangle/10 % 10;
		if(ten > 0 || sangle > 100)
		Draw7segNum(220, 250, ten, 100);
		Draw7segNum(320, 250, sangle %10, 100);
		if(sangle > 60){
			Paint_DrawNum(284, 465, 180, &Font24, 0x00, 0xFF);
			Draw7segNum(285, 155, 10, 50);
		}
		else{ //angle pointer zoom 3X
			Paint_DrawNum(284, 465, 60, &Font24, 0x00, 0xFF);
			Draw7segNum(283, 155, 11, 50);   
			angle = 3 * angle;
		}	
		Draw_wind_line(iangle, 0xff); //clear previous angle pointer
		Draw_wind_line(angle, 0x50);
		iangle = angle;
	}
	else{		
		Draw7segNum(120, 250,13,100);				
		Draw7segNum(220, 250,13,100);
		Draw7segNum(320, 250,13,100);
		Draw7segNum(283, 155, 13, 50);
		Draw_wind_line(iangle, 0xff);	
	}
}

void Draw_wind_line(UWORD angle, UBYTE color)
{	
	double fac = 0.017453;
	float radian = angle * fac;	
	float sx = 310 + 288 * -1 * (cos(radian));        
	float sy = 310 + 288 * (sin(radian));
	float ex = 310 + 233 * -1 * (cos(radian)); 
	float ey = 310 + 233 * (sin(radian));
	Paint_DrawLine(sy, sx, ey, ex, color, DOT_PIXEL_8X8, LINE_STYLE_SOLID);		
}

void Draw_angles(UWORD angle)
{
	double fac = 0.017453;
	float radian = angle * fac;
	float sx = 310 + 220 * -1 * (cos(radian));        
	float sy = 310 + 220 * (sin(radian));
	float ex = 310 + 200 * -1 * (cos(radian)); 
	float ey = 310 + 200 * (sin(radian));
	Paint_DrawLine(sy, sx, ey, ex, 0x50, DOT_PIXEL_2X2, LINE_STYLE_SOLID);			
}

void Draw_sectors(void)
{
	Paint_DrawCircle(310, 310, 300, 0x30, DOT_PIXEL_3X3, DRAW_FILL_EMPTY);
	Paint_DrawCircle(310, 310, 220, 0x20, DOT_PIXEL_2X2, DRAW_FILL_EMPTY);
	Paint_DrawCircle(450, 265, 12, 0x20, DOT_PIXEL_3X3, DRAW_FILL_EMPTY);
	Paint_DrawLine(310, 120, 240, 210, 0x30 , DOT_PIXEL_3X3, LINE_STYLE_SOLID);
	Paint_DrawLine(310, 120, 380, 210, 0x30 , DOT_PIXEL_3X3, LINE_STYLE_SOLID);
	Paint_DrawLine(240, 210, 240, 235, 0x30 , DOT_PIXEL_3X3, LINE_STYLE_SOLID);
	Paint_DrawLine(380, 210, 380, 235, 0x30 , DOT_PIXEL_3X3, LINE_STYLE_SOLID);
	Paint_DrawLine(240, 235, 380, 235, 0x30 , DOT_PIXEL_3X3, LINE_STYLE_SOLID);
	Paint_DrawLine(630, 0, 630, 825, 0x30 , DOT_PIXEL_4X4, LINE_STYLE_SOLID);
	Paint_DrawLine(630, 412, 1200, 412, 0x30 , DOT_PIXEL_4X4, LINE_STYLE_DOTTED);
	Paint_DrawString_EN(800,25,"VELOCITY  KN", &Font24, 0x00, 0xFF);	
	Paint_DrawString_EN(800,440,"DEPTH  M", &Font24, 0x00, 0xFF);
	Paint_DrawString_EN(480,785,"M/S", &Font24, 0x00, 0xFF);
	Paint_DrawString_EN(290,410,"AWD", &Font24, 0x00, 0xFF);
	Paint_DrawString_EN(70,610,"TWS", &Font24, 0x00, 0xFF);
	for(int i=0;i<340;i=i+30){
		Draw_angles(i);	
	}		
}

void CleanCurl(void)
{
	curl_easy_cleanup(dcurl);
	curl_easy_cleanup(scurl);
	curl_easy_cleanup(acurl);
	curl_easy_cleanup(vcurl);	
}

UBYTE Display_BMP(UWORD Panel_Width, UWORD Panel_Height, UDOUBLE Init_Target_Memory_Addr, UBYTE BitsPerPixel)
{
    UWORD WIDTH = Panel_Width;  
    UWORD HEIGHT = Panel_Height;
    UDOUBLE Imagesize;
    Imagesize = ((WIDTH * BitsPerPixel % 8 == 0)? (WIDTH * BitsPerPixel / 8 ): (WIDTH * BitsPerPixel / 8 + 1)) * HEIGHT;
    if((Refresh_Frame_Buf = (UBYTE *)malloc(Imagesize)) == NULL){
        Debug("Failed to apply for black memory...\r\n");
        return -1;
    }
    Paint_NewImage(Refresh_Frame_Buf, WIDTH, HEIGHT, 0, BLACK);
    Paint_SelectImage(Refresh_Frame_Buf);
	Epd_Mode(epd_mode);
    Paint_SetBitsPerPixel(BitsPerPixel);
    Paint_Clear(WHITE);	
    GUI_ReadBmp("./pic/rile1.bmp", 0, 0);
	EPD_IT8951_1bp_Refresh(Refresh_Frame_Buf, 0, 0, WIDTH,  HEIGHT, A2_Mode, Init_Target_Memory_Addr,false);
    if(Refresh_Frame_Buf != NULL){
        free(Refresh_Frame_Buf);
        Refresh_Frame_Buf = NULL;
    }
    DEV_Delay_ms(2000);
    return 0;
}
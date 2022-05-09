﻿#include <main.h>
#include <windows.h>
#include <intrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <shlwapi.h>

void levelgen(){
	HANDLE h = CreateFile("level.lvl",GENERIC_READ,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
	if((int)h != -1){
		ReadFile(h,map,MAPRAM,0,0);
		ReadFile(h,mapdata,MAPRAM,0,0);
	}
	else{
		for(int i = 0;i < properties->lvlSz*properties->lvlSz*4;i+=4){
			map[i] = 28;
			map[i+1] = 255;
			map[i+2] = 255;
			map[i+3] = 255;
			mapdata[i] = 255;
			mapdata[i+1] = 255;
			mapdata[i+2] = 255;
		}
		for(int i = properties->lvlSz*properties->lvlSz*4;i < properties->lvlSz*properties->lvlSz*properties->lvlSz*4;i+=4){
			map[i+1] = 128;
			map[i+2] = 128;
			map[i+3] = 128;
		}
	}
	CloseHandle(h);
}

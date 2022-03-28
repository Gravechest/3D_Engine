#include <main.h>
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
		}
		map[(10 + 10 * properties->lvlSz + 8 * properties->lvlSz * properties->lvlSz) * 4] = 1;
	}
	CloseHandle(h);
}

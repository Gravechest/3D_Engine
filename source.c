#include <windows.h>
#include <math.h>
#include <intrin.h>
#include <stdio.h>
#include <glew.h>
#include <GL/gl.h>
#include <main.h>

#define resx 512
#define resy 512

#define RENDERDISTANCE 32

#define VRAM resx*resy*4

#define PI_2 1.57079632679489661923

char *CLsource;
unsigned char *map;
unsigned char *mapdata;
float *lightmap;

int settings;
char touchStatus;
char threadStatus;
char abilities;

PROPERTIES *properties;
PLAYERDATA *player;

RGB colorSel;

unsigned char blockSel = 1;
unsigned char toolSel;	

char specialBlock[2];
float specialBlockcrd[3];

float mousex;
float mousey;
float stamina = 1.0;
float brightness;
float averageBrightness;
float maxBrightness;

BITMAPINFO bmi = { sizeof(BITMAPINFOHEADER),resx,resy,1,32,BI_RGB };	

HANDLE renderingThread;
HANDLE consoleThread;
HANDLE physicsThread;
HANDLE entitiesThread;
HANDLE staticEntitiesThread;
HANDLE ittmapThread;

const char name[] = "window";

HWND window;
HDC dc;
MSG Msg;

void playerDeath(){
	player->xpos = player->xspawn;
	player->ypos = player->yspawn;
	player->zpos = player->zspawn;
	player->xvel = 0;
	player->yvel = 0;
	player->zvel = 0;
}

void blockDetection(float x,float y,float z,int axis){
	int block = crds2map(x,y,z);
	switch(map[block]){
	case 3:
		break;
	case 12:
		specialBlock[0] = map[block];
		specialBlock[1] |= axis;
		break;
	case 14:
		playerDeath();
		break;
	case 49:
		if(axis == 32 || axis == 1){
			break;
		}
		touchStatus |= axis;
		break;
	case 67:
		break;
	default:
		touchStatus |= axis;
		break;
	}
}

void specialBlockDetection(float x,float y,float z,int axis){
	int block = crds2map(x,y,z);
	switch(map[block]){
	case 3:
		break;
	case 49:
		if(touchStatus){
			break;
		}
		if(y-(int)y>z-(int)z){
			specialBlock[0] = map[block];
			specialBlock[1] |= axis;
		}
		break;
	case 67:
		player->xpos = mapdata[block] + player->xpos - (int)player->xpos;
		player->ypos = mapdata[block+1] + player->ypos - (int)player->ypos;
		player->zpos = mapdata[block+2] + player->zpos - (int)player->zpos + 1;
		break;
	default:
		touchStatus |= 64;
		break;
	}
}

void hitboxZdown(float x,float y,float z){
	if(map[((int)x + (int)y * properties->lvlSz + (int)z * properties->lvlSz * properties->lvlSz) * 4]){
		z -= player->zvel;
		if(!map[((int)x + (int)y * properties->lvlSz + (int)z * properties->lvlSz * properties->lvlSz) * 4]){
			z += player->zvel;
			blockDetection(x,y,z,1);
		}
		else{
			z += player->zvel;
			specialBlockDetection(x,y,z,1);
		}
	}
	if(z < 0){
		playerDeath();
	}
}

void hitboxZup(float x,float y,float z){
	if(z > properties->lvlSz || map[((int)x + (int)y * properties->lvlSz + (int)z * properties->lvlSz * properties->lvlSz) * 4]){
		z -= player->zvel;
		if(!map[((int)x + (int)y * properties->lvlSz + (int)z * properties->lvlSz * properties->lvlSz) * 4]){
			z += player->zvel;
			blockDetection(x,y,z,2);
		}
		else{
			z += player->zvel;
			specialBlockDetection(x,y,z,2);
		}
	}
}

void hitboxXdown(float x,float y,float z){
	if(x < 0 || x > properties->lvlSz || map[((int)x + (int)y * properties->lvlSz + (int)z * properties->lvlSz * properties->lvlSz) * 4]){
		x -= player->xvel;
		if(!map[((int)x + (int)y * properties->lvlSz + (int)z * properties->lvlSz * properties->lvlSz) * 4]){
			x += player->xvel;
			blockDetection(x,y,z,4);
		}
		else{
			x += player->xvel;
			specialBlockDetection(x,y,z,4);
		}
	}
}

void hitboxXup(float x,float y,float z){
	if(x < 0 || x > properties->lvlSz || map[((int)x + (int)y * properties->lvlSz + (int)z * properties->lvlSz * properties->lvlSz) * 4]){
		x -= player->xvel;
		if(!map[((int)x + (int)y * properties->lvlSz + (int)z * properties->lvlSz * properties->lvlSz) * 4]){
			x += player->xvel;
			blockDetection(x,y,z,8);
		}
		else{
			x += player->xvel;
			specialBlockDetection(x,y,z,8);
		}
	}
}

void hitboxYdown(float x,float y,float z){
	if(y < 0 || y > properties->lvlSz || map[((int)x + (int)y * properties->lvlSz + (int)z * properties->lvlSz * properties->lvlSz) * 4]){
		y -= player->yvel;
		if(!map[((int)x + (int)y * properties->lvlSz + (int)z * properties->lvlSz * properties->lvlSz) * 4]){
			y += player->yvel;
			blockDetection(x,y,z,16);
		}
		else{
			y += player->yvel;
			specialBlockDetection(x,y,z,16);
		}
	}
}

void hitboxYup(float x,float y,float z){
	if(y < 0 || y > properties->lvlSz || map[((int)x + (int)y * properties->lvlSz + (int)z * properties->lvlSz * properties->lvlSz) * 4]){
		y -= player->yvel;
		if(!map[((int)x + (int)y * properties->lvlSz + (int)z * properties->lvlSz * properties->lvlSz) * 4]){
			y += player->yvel;
			blockDetection(x,y,z,32);
		}
		else{
			y += player->yvel;
			specialBlockDetection(x,y,z,32);
		}
	}
}

void rayItterate(RAY *ray){
    if(ray->sidex < ray->sidey){
        if(ray->sidex < ray->sidez){
			ray->ix += ray->stepx;
			ray->sidex += ray->deltax;
			ray->side = 0;
        }
        else{
			ray->iz += ray->stepz;
			ray->sidez += ray->deltaz;
			ray->side = 2;
        }
    }
    else if(ray->sidey < ray->sidez){
		ray->iy += ray->stepy;
		ray->sidey += ray->deltay;
		ray->side = 1;
    }
    else{
		ray->iz += ray->stepz;
		ray->sidez += ray->deltaz;
		ray->side = 2;
    }
}

void updateLightRay(RAY *ray,float red,float green,float blue){
	while(green > 0.01 || red > 0.01 || blue > 0.01){
		rayItterate(ray);
		if(ray->ix < 0 || ray->iy < 0 || ray->iz < 0 || ray->ix >= properties->lvlSz || ray->iy >= properties->lvlSz || ray->iz >= properties->lvlSz){
			break;
		}
		int block = crds2map(ray->ix,ray->iy,ray->iz);
		if(map[block]){
			switch(map[block]){
			case 60:
				break;	
			default:
				if(map[block] > 7 && map[block] < 12){
					if(red > 0){
						lightmap[block+1] = 255;
					}
					if(green > 0){
						lightmap[block+2] = 255;
					}
					if(blue > 0){
						lightmap[block+3] = 255;
					}
					break;
				}
				switch(ray->side){
				case 0:
					ray->vx = -ray->vx;
					ray->stepx = -ray->stepx;
					break;
				case 1:
					ray->vy = -ray->vy;
					ray->stepy = -ray->stepy;
					break;
				case 2:
					ray->vz = -ray->vz;
					ray->stepz = -ray->stepz;
					break;
				}
				break;
			}
			switch(map[block]){
			case 21:
				red   *= 0.05;
				green *= 0.15;
				blue  *= 0.45;
				break;
			case 22:
				red   *= 0.05;
				green *= 0.45;
				blue  *= 0.05;
				break;
			case 23:
				red   *= 0.25;
				green *= 0.25;
				blue  *= 0.25;
				break;
			case 24:
				red   *= 0.05;
				green *= 0.05;
				blue  *= 0.05;
				break;
			case 25:
				red   *= 0.15;
				green *= 0.35;
				blue  *= 0.3;
				break;
			case 26:
				red   *= 0.05;
				green *= 0.2;
				blue  *= 0.5;
				break;
			case 27:
				red   *= 0.4;
				green *= 0.2;
				blue  *= 0.05;
				break;
			case 29:
				red   *= 0.5;
				green *= 0.05;
				blue  *= 0.05;
				break;
			default:
				red   *= 0.5;
				green *= 0.5;
				blue  *= 0.5;
			}
			continue;
		}
		if(lightmap[block+1] > 255 || lightmap[block+2] > 255 || lightmap[block+3] > 255){
			continue;
		}
		lightmap[block+1] += red;
		lightmap[block+2] += green;
		lightmap[block+3] += blue;
	}
}

void updateLight(int pos,float r,float g,float b){
	RAY ray;
	for(float i = -0.78; i < 0.78;i+= 0.003){
		for(float i2 = -0.78; i2 < 0.78;i2+= 0.003){
			float i3 = (float)rand() /RAND_MAX * 6.28 - 3.14;
			ray = rayCreate((float)(pos%(properties->lvlSz*4)/4)+0.5,
		      (float)(pos / (properties->lvlSz*4) * (properties->lvlSz*4) % (properties->lvlSz*properties->lvlSz*4) / (properties->lvlSz*4)) + 0.5,
			  (float)(pos / (properties->lvlSz*properties->lvlSz*4)) + 0.5,
			  sinf(i3) * cosf(i+i2),cosf(i3) * cosf(i+i2), sinf(i+i2));
			updateLightRay(&ray,r,g,b);
		}
	}
}

inline int max3(int val1,int val2,int val3){
	if(val1 > val2){
		if(val1 > val3){
			return val1;
		}
		else{
			return val3;
		}
	}
	else if(val2 > val3){
		return val2;
	}
	else{
		return val3;
	}
}

void updateLight2(){
	lightmap = HeapAlloc(GetProcessHeap(),8,sizeof(float) * MAPRAM);	
	for(int i = 0;i < MAPRAM;i+=4){
		if(map[i] == 8){
			map[i] = 0;
			updateLight(i+1,(float)(mapdata[i])/255.0,(float)(mapdata[i+1])/255.0,(float)(mapdata[i+2])/255.0);
			map[i] = 8;	
		}
	}
	for(int i = 0;i < MAPRAM;i+=4){
		map[i+1] = lightmap[i+1];
		map[i+2] = lightmap[i+2];
		map[i+3] = lightmap[i+3];
	}
	HeapFree(GetProcessHeap(),0,lightmap);
	glMes[glMesC].id = 3;
	glMesC++;
	glMes[glMesC].id = 6;
	glMesC++;
}

void updateBlock(int pos,int val){
	map[pos] = val;
	glMes[glMesC].id = 3;
	glMesC++;
}

void updateBlockLight(int pos){
	map[pos+1] = colorSel.r;
	map[pos+2] = colorSel.g;
	map[pos+3] = colorSel.b;
	glMes[glMesC].id = 3;
	glMesC++;
}

void deleteBlock(int pos){
	map[pos] = 0;
	glMes[glMesC].id = 3;
	glMesC++;
}

void spawnEntity(float x,float y,float z,float vx,float vy,float vz,float sz,int id){
	entity[entityC].x  = x;
	entity[entityC].y  = y;
	entity[entityC].z  = z;
	entity[entityC].vx = vx;
	entity[entityC].vy = vy;
	entity[entityC].vz = vz;
	entity[entityC].sz = sz;
	entity[entityC].id = id;
	entityC++;
}

void HDR(float x,float y){
	RAY ray;
	ray.x = player->xpos;	
	ray.y = player->ypos;
	ray.z = player->zpos;
	ray.vz = sinf(player->yangle + y);
	ray.vx = cosf(player->xangle + x) * cosf(player->yangle + y);
	ray.vy = sinf(player->xangle + x) * cosf(player->yangle + y);
	int i;
	int im = 24;
	float b = 0.1;
	for(;i < im;i++){
		if(ray.x < 0 || ray.y < 0 || ray.z < 0 || ray.x >= properties->lvlSz || ray.y >= properties->lvlSz || ray.z >= properties->lvlSz){
			break;
		}	
		int block = ((int)ray.x + (int)ray.y * properties->lvlSz + (int)ray.z * properties->lvlSz * properties->lvlSz) * 4;
		if(map[block]){
			if(ray.x - (int)ray.x < 0.00002 || ray.x - (int)ray.x > 0.99998){
				ray.vx = -ray.vx;
			}
			else if(ray.y - (int)ray.y < 0.00002 || ray.y - (int)ray.y > 0.99998){
				ray.vy = -ray.vy;
			}
			else if(ray.z - (int)ray.z < 0.00002 || ray.z - (int)ray.z > 0.99998){
				ray.vz = -ray.vz;
			}
			break;
		}
		b += max3(map[block+1],map[block+2],map[block+3]);
		rayItterate(&ray);
	}
	b /= i;
	maxBrightness = max(b,maxBrightness);
	averageBrightness += b;
}

//windows messages kunnen hier worden behandeld

long _stdcall proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
	switch(msg){
	case WM_QUIT:
	case WM_CLOSE:
	case WM_DESTROY:{
		HANDLE h = CreateFile("level.lvl",GENERIC_WRITE,0,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
		WriteFile(h,map,MAPRAM,0,0);
		WriteFile(h,mapdata,MAPRAM,0,0);
		CloseHandle(h);
		ExitProcess(0);
		}
	case WM_ACTIVATE:
		switch(wParam){
		case WA_INACTIVE:
			SuspendThread(physicsThread);
			settings ^= 0x80;
			break;
		default:
			ResumeThread(physicsThread);
			settings ^= 0x80;
			break;
		}
	case WM_KEYDOWN:
		switch(wParam){
		case VK_LCONTROL:
			if(!abilities & 0x01){ 
				player->xvel *= 2.5	;
				player->yvel *= 2.5;
				player->zvel -= 0.3;
				abilities ^= 0x01;
			}
		}
		if(GetKeyState(VK_F1) & 0x80){
			settings ^= 1;
		}
		if(GetKeyState(VK_F6) & 0x80){
			for(int i = 0;i < properties->lvlSz*properties->lvlSz*properties->lvlSz*4;i+=4){
				if(map[i] == 1){
					map[i] = 0;
				}
			}
		}
		if(GetKeyState(VK_F7) & 0x80){
			for(int i = 0;i < properties->lvlSz*properties->lvlSz*properties->lvlSz*4;i+=4){
				if(map[i] < 2){
					int x = i%(properties->lvlSz*4)-20;
					int y = i/(properties->lvlSz*4)%(properties->lvlSz*4)*properties->lvlSz*4-20*properties->lvlSz;
					int z = i/properties->lvlSz/properties->lvlSz/4*properties->lvlSz*properties->lvlSz*4-20*properties->lvlSz*properties->lvlSz;
					if(x < 0){
						x = 0;
					}
					if(y < 0){
						y = 0;
					}
					if(z < 0){
						z = 0;
					}
					if(z > properties->lvlSz*properties->lvlSz*properties->lvlSz*4-properties->lvlSz*properties->lvlSz*40){
						z = properties->lvlSz*properties->lvlSz*properties->lvlSz*4-properties->lvlSz*properties->lvlSz*40;
					}
					if(y > properties->lvlSz*properties->lvlSz*4-properties->lvlSz*40){
						y = properties->lvlSz*properties->lvlSz*4-properties->lvlSz*40;
					}
					if(x > properties->lvlSz*4-40){
						x = properties->lvlSz*4-40;
					}
					int hit = 0;
					for(int i2 = z;i2 < z + 10*properties->lvlSz*properties->lvlSz*4;i2+=properties->lvlSz*properties->lvlSz*4){
						for(int i3 = y;i3 < y + 10*properties->lvlSz*4;i3+=properties->lvlSz*4){
							for(int i4 = x;i4 < x + 40;i4+=4){
								if(map[i2+i3+i4] > 1){
									i2 = 0x0fffffff;
									i3 = 0x0fffffff;
									i4 = 0x0fffffff;
									hit = 1;
									break;
								}
							}
						}	
					}
					if(!hit){
						map[i] = 1;
					}
				}
			}
		}
		if(GetKeyState(VK_PRIOR) & 0x80){
			toolSel++;
		}
		if(GetKeyState(VK_NEXT) & 0x80){
			toolSel--;
		}
		if(GetKeyState(0x46) & 0x80){
			settings ^= 2;
			if(settings & 2){
				SetWindowPos(window,0,0,0,GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN),0);	
				properties->xres = GetSystemMetrics(SM_CXSCREEN);
				properties->yres = GetSystemMetrics(SM_CYSCREEN);
				glMes[glMesC].id = 0;
				glMesC++;
			}
			else{
				SetWindowPos(window,0,0,0,256,256,0);	
				properties->xres = 256;
				properties->yres = 256;
				glMes[glMesC].id = 0;
				glMesC++;
			}
		}
		if(GetKeyState(VK_ADD) & 0x80){
			selarea.x = 0;
			selarea.y = 0;
			selarea.z = 0;
			if(toolSel == 2 || toolSel == 5){
				if(GetKeyState(VK_RSHIFT) & 0x80){
					if(GetKeyState(VK_RCONTROL) & 0x80){
						colorSel.a++;
					}
					else{
						colorSel.b++;
					}
				}
				else{
					if(GetKeyState(VK_RCONTROL) & 0x80){
						colorSel.g++;
					}
					else{
						colorSel.r++;
					}
				}
			}
			else{
				blockSel++;
			}
		}
		if(GetKeyState(VK_SUBTRACT) & 0x80){
			selarea.x = 0;
			selarea.y = 0;
			selarea.z = 0;
			if(toolSel == 2 || toolSel == 5){
				if(GetKeyState(VK_RSHIFT) & 0x80){
					if(GetKeyState(VK_RCONTROL) & 0x80){
						colorSel.a--;
					}
					else{
						colorSel.b--;
					}
				}
				else{
					if(GetKeyState(VK_RCONTROL) & 0x80){
						colorSel.g--;
					}
					else{
						colorSel.r--;
					}
				}
			}
			else{
				if(blockSel != 0){
					blockSel--;
				}
			}
		}
		if(GetKeyState(0x4b) & 0x80){
			entity[entityC].x  = player->xpos+1;
			entity[entityC].y  = player->ypos;
			entity[entityC].z  = player->zpos;
			entity[entityC].vx = player->xdir / 20;
			entity[entityC].vy = player->ydir / 20;
			entity[entityC].vz = player->zdir / 20;
			entityC++;
		}
		if(GetKeyState(VK_F2) & 0x80){
			settings ^= 4;
			if(settings & 0x04){
				for(int i = 0;i < MAPRAM;i+=4){
					map[i+5] = 128;
					map[i+6] = 128;
					map[i+7] = 128;
				}
				glMes[glMesC].id = 3;
				glMesC++;
			}
			else{
				CreateThread(0,0,updateLight2,0,0,0);
				glMes[glMesC].id = 3;
				glMesC++;
			}
		}
		if(settings & 0x01){
			if(GetKeyState(0x45) & 0x80){
				map[(int)player->xpos + (int)player->ypos * properties->lvlSz + ((int)player->zpos - 1) * properties->lvlSz * properties->lvlSz] = blockSel;
				glMes[glMesC].id = 1;
				glMes[glMesC].data1 = player->xpos;
				glMes[glMesC].data2 = player->ypos;
				glMes[glMesC].data3 = player->zpos - 1;
				glMesC++;
			}
		}
		break;
	case WM_KEYUP:
		switch(wParam){
		case VK_CONTROL:
			abilities ^= 0x01;
			if(map[((int)(player->xpos - 0.2) + (int)(player->ypos - 0.2) * properties->lvlSz + (int)( player->zpos - 1.69) * properties->lvlSz * properties->lvlSz) * 4]){
				player->zvel = 0.211;
			}
			else if(map[((int)(player->xpos + 0.2) + (int)(player->ypos - 0.2) * properties->lvlSz + (int)( player->zpos - 1.69) * properties->lvlSz * properties->lvlSz) * 4]){
				player->zvel = 0.211;
			}
			else if(map[((int)(player->xpos - 0.2) + (int)(player->ypos + 0.2) * properties->lvlSz + (int)( player->zpos - 1.69) * properties->lvlSz * properties->lvlSz) * 4]){
				player->zvel = 0.211;
			}
			else if(map[((int)(player->xpos + 0.2) + (int)(player->ypos + 0.2) * properties->lvlSz + (int)( player->zpos - 1.69) * properties->lvlSz * properties->lvlSz) * 4]){
				player->zvel = 0.211;
			}
			break;
		}
		break;
	case WM_MOUSEMOVE:{
			POINT curp;
			GetCursorPos(&curp);
			mousex = (float)(curp.x - 50) / 250;
			mousey = (float)(curp.y - 50) / 250;
			player->xangle += mousex;
			player->yangle -= mousey;
			if(player->yangle < -1.6){
				player->yangle = -1.6;
			}
			if(player->yangle > 1.6){
				player->yangle = 1.6;
			}
			SetCursorPos(50,50);
		}
		break;
	case WM_MBUTTONDOWN:{
			RAY ray = rayCreate(player->xpos,player->ypos,player->zpos,player->xdir*player->xydir,player->ydir*player->xydir,player->zdir);
			for(int i = 0;i < 12;i++){
				rayItterate(&ray);
				int block = crds2map(ray.ix,ray.iy,ray.iz);
				if(map[block]){
					switch(toolSel){
					case 5:
						colorSel.r = ray.ix;
						colorSel.g = ray.iy;
						colorSel.b = ray.iz;
						break;
					default:
						blockSel = map[block];
						break;
					}
					break;
				}
			}
			break;
		}
	case WM_LBUTTONDOWN:{
			if(settings & 0x01 || settings & 0x04){
				tools();
			}
		break;
		}
	case WM_RBUTTONDOWN:{
			RAY ray = rayCreate(player->xpos,player->ypos,player->zpos,player->xdir*player->xydir,player->ydir*player->xydir,player->zdir);
			for(int i = 0;i < 12;i++){
				rayItterate(&ray);
				int block = crds2map(ray.ix,ray.iy,ray.iz);
				if(map[block]){
					deleteBlock(block);
					break;
				}
			}
			break;
		}
	}
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

WNDCLASS wndclass = {0,proc,0,0,0,0,0,0,name,name};

void physics()
{
	player->xfov = 2;
	for (;;)
	{
		player->xydir = cosf(player->yangle);
		player->xdir  = cosf(player->xangle);
	 	player->ydir  = sinf(player->xangle);
	 	player->zdir  = sinf(player->yangle);

		if (settings & 0x01)
		{
			int amp = 1;
			if(GetKeyState(VK_CONTROL) & 0x80){
				amp = 3;
			}
			if (GetKeyState(0x57) & 0x80)
			{
				player->xpos += cosf(player->xangle) / 8 * amp;
				player->ypos += sinf(player->xangle) / 8 * amp;
			}
			if (GetKeyState(0x53) & 0x80)
			{
				player->xpos -= cosf(player->xangle) / 8 * amp;
				player->ypos -= sinf(player->xangle) / 8 * amp;
			}
			if (GetKeyState(0x44) & 0x80)
			{
				player->xpos += cosf(player->xangle + PI_2) / 8 * amp;
				player->ypos += sinf(player->xangle + PI_2) / 8 * amp;
			}
			if (GetKeyState(0x41) & 0x80)
			{
				player->xpos -= cosf(player->xangle + PI_2) / 8 * amp;
				player->ypos -= sinf(player->xangle + PI_2) / 8 * amp;
			}
			if (GetKeyState(VK_SPACE) & 0x80)
			{
				player->zpos += 0.15 * amp;
			}
			if (GetKeyState(VK_LSHIFT) & 0x80)
			{
				player->zpos -= 0.15 * amp;
			}
			if(player->xpos < 0.0){
				player->xpos = 0.0;
			}
			if(player->ypos < 0.0){
				player->ypos = 0.0;
			}
			if(player->zpos < 0.0){
				player->zpos = 0.0;
			}
			if(player->xpos > properties->lvlSz){
				player->xpos = properties->lvlSz;
			}
			if(player->ypos > properties->lvlSz){
				player->ypos = properties->lvlSz;
			}
			if(player->zpos > properties->lvlSz){
				player->zpos = properties->lvlSz;
			}
		}
		else
		{
			if(GetKeyState(VK_LBUTTON) & 0x80){
				spawnEntity(player->xpos,player->ypos,player->zpos,0,0,0,0.3,1);
			}
			if(stamina < 1.0){
				stamina+=0.045;
			}
			else{
				stamina = 1.0;
			}
			float amp = 1;
			if(GetKeyState(VK_LCONTROL) & 0x80){
				amp = 0.3;
			}
			if (GetKeyState(0x57) & 0x80)
			{
				player->xvel += cosf(player->xangle) / 50 * amp;
				player->yvel += sinf(player->xangle) / 50 * amp;

			}
			if (GetKeyState(0x53) & 0x80)
			{
				player->xvel -= cosf(player->xangle) / 50 * amp;
				player->yvel -= sinf(player->xangle) / 50 * amp;
			}
			if (GetKeyState(0x44) & 0x80)
			{
				player->xvel += cosf(player->xangle + PI_2) / 50 * amp;
				player->yvel += sinf(player->xangle + PI_2) / 50 * amp;
			}
			if (GetKeyState(0x41) & 0x80)
			{
				player->xvel -= cosf(player->xangle + PI_2) / 50 * amp;
				player->yvel -= sinf(player->xangle + PI_2) / 50 * amp;
			}
			player->zvel -= 0.015;

			player->xpos += player->xvel;
			player->ypos += player->yvel;
			player->zpos += player->zvel;

			if (player->zvel < 0){
				if(GetKeyState(VK_LCONTROL) & 0x80){
					hitboxZdown(player->xpos - 0.2, player->ypos - 0.2, player->zpos - 0.7);
					hitboxZdown(player->xpos + 0.2, player->ypos - 0.2, player->zpos - 0.7);
					hitboxZdown(player->xpos - 0.2, player->ypos + 0.2, player->zpos - 0.7);
					hitboxZdown(player->xpos + 0.2, player->ypos + 0.2, player->zpos - 0.7);
				}
				else{
					hitboxZdown(player->xpos - 0.2, player->ypos - 0.2, player->zpos - 1.7);
					hitboxZdown(player->xpos + 0.2, player->ypos - 0.2, player->zpos - 1.7);
					hitboxZdown(player->xpos - 0.2, player->ypos + 0.2, player->zpos - 1.7);
					hitboxZdown(player->xpos + 0.2, player->ypos + 0.2, player->zpos - 1.7);
				}
			}
			else{
				hitboxZup(player->xpos - 0.2, player->ypos - 0.2, player->zpos + 0.2);
				hitboxZup(player->xpos + 0.2, player->ypos - 0.2, player->zpos + 0.2);
				hitboxZup(player->xpos - 0.2, player->ypos + 0.2, player->zpos + 0.2);
				hitboxZup(player->xpos + 0.2, player->ypos + 0.2, player->zpos + 0.2);

			}
			if (player->xvel < 0){
				if(GetKeyState(VK_LCONTROL) & 0x80){
					for(float i = -0.7;i <= 0.2;i+=0.1){
						hitboxXdown(player->xpos - 0.2, player->ypos + 0.2, player->zpos + i);
						hitboxXdown(player->xpos - 0.2, player->ypos - 0.2, player->zpos + i);
					}
				}
				else{
					for(float i = -1.7;i <= 0.2;i+=0.1){
						hitboxXdown(player->xpos - 0.2, player->ypos + 0.2, player->zpos + i);
						hitboxXdown(player->xpos - 0.2, player->ypos - 0.2, player->zpos + i);
					}
				}

			}
			else{
				if(GetKeyState(VK_LCONTROL) & 0x80){
					for(float i = -0.7;i <= 0.2;i+=0.1){
						hitboxXup(player->xpos + 0.2, player->ypos + 0.2, player->zpos + i);
						hitboxXup(player->xpos + 0.2, player->ypos - 0.2, player->zpos + i);
					}
				}
				else{
					for(float i = -1.7;i <= 0.2;i+=0.1){
						hitboxXup(player->xpos + 0.2, player->ypos + 0.2, player->zpos + i);
						hitboxXup(player->xpos + 0.2, player->ypos - 0.2, player->zpos + i);
					}
				}

			}
			if (player->yvel < 0){
				if(GetKeyState(VK_CONTROL) & 0x80){
					for(float i = -0.7;i <= 0.2;i+=0.1){
						hitboxYdown(player->xpos + 0.2, player->ypos - 0.2, player->zpos + i);
						hitboxYdown(player->xpos - 0.2, player->ypos - 0.2, player->zpos + i);
					}
				}
				else{
					for(float i = -1.7;i <= 0.2;i+=0.1){
						hitboxYdown(player->xpos + 0.2, player->ypos - 0.2, player->zpos + i);
						hitboxYdown(player->xpos - 0.2, player->ypos - 0.2, player->zpos + i);
					}
				}

			}
			else{
				if(GetKeyState(VK_LCONTROL) & 0x80){
					for(float i = -0.7;i <= 0.2;i+=0.1){
						hitboxYup(player->xpos + 0.2, player->ypos + 0.2, player->zpos + i);
						hitboxYup(player->xpos - 0.2, player->ypos + 0.2, player->zpos + i);
					}
				}
				else{
					for(float i = -1.7;i <= 0.2;i+=0.1){
						hitboxYup(player->xpos + 0.2, player->ypos + 0.2, player->zpos + i);
						hitboxYup(player->xpos - 0.2, player->ypos + 0.2, player->zpos + i);
					}
				}

			}
			touchStatus &= ~specialBlock[1];
			switch(specialBlock[0]){
			case 12:
				switch(specialBlock[1]){
				case 1:
					player->zvel = 0.5;
					break;
				case 2:
					player->zvel = -0.5;
					break;
				case 4:
					player->xvel = 0.5;
					break;
				case 8:
					player->xvel = -0.5;
					break;
				case 16:
					player->yvel = 0.5;
					break;
				case 32:
					player->yvel = -0.5;
					break;
				}
				break;
			case 49: 
				player->zpos -= player->zvel;
				player->zpos += player->yvel;
				break;
			}
			if (touchStatus & 0x01){
				player->zpos -= player->zvel;
				player->zvel = 0;
				if (GetKeyState(VK_SPACE) & 0x80){
					sound(1);
					player->zvel += 0.2 * stamina;
					player->xvel *= 1.7 * stamina;
					player->yvel *= 1.7 * stamina;
					stamina = 0.0;
				}
			}
			if (touchStatus & 0x02){
				player->zpos -= player->zvel;
				player->zvel = 0;
			}
			if (touchStatus & 0x04){
				player->xpos -= player->xvel;
				player->xvel = 0;
				if(player->zvel < -0.05){
					player->zvel = -0.05;
				}
				if(GetKeyState(VK_SPACE) & 0x80){
					player->zvel += 0.25 * stamina;
					player->xvel += 0.25 * stamina;
					stamina = 0.0;
				}
			}
			if (touchStatus & 0x08){
				player->xpos -= player->xvel;
				player->xvel = 0;
				if(player->zvel < -0.05){
					player->zvel = -0.05;
				}
				if(GetKeyState(VK_SPACE) & 0x80){
					player->zvel += 0.25 * stamina;
					player->xvel += -0.25 * stamina;
					stamina = 0.0;
				}
			}
			if (touchStatus & 0x10){
				player->ypos -= player->yvel;
				player->yvel = 0;
				if(player->zvel < -0.05){
					player->zvel = -0.05;
				}
				if(GetKeyState(VK_SPACE) & 0x80){
					player->zvel += 0.25 * stamina;
					player->yvel += 0.25 * stamina;
					stamina = 0.0;
				}
			}
			if (touchStatus & 0x20){
				player->ypos -= player->yvel;
				player->yvel = 0;
				if(player->zvel < -0.05){
					player->zvel = -0.05;
				}
				if(GetKeyState(VK_SPACE) & 0x80){
					player->zvel += 0.25 * stamina;
					player->yvel += -0.25 * stamina;
					stamina = 0.0;
				}
			}
			if(touchStatus == 64){
				player->zvel = 0;
				player->zpos += 0.1;
			}
			player->xvel /= 1.08;
			player->yvel /= 1.08;
		}
		touchStatus = 0;
		specialBlock[0] = 0;
		specialBlock[1] = 0;
		player->zvel /= 1.003;	
		tick++;
		
		Sleep(15);
	}
}

void main()
{
	//om de Sleep functie accurater te maken
	timeBeginPeriod(1);

	map        = HeapAlloc(GetProcessHeap(),8,MAPRAM);
	mapdata    = HeapAlloc(GetProcessHeap(),8,MAPRAM);
	player     = HeapAlloc(GetProcessHeap(),8,sizeof(PLAYERDATA));
	properties = HeapAlloc(GetProcessHeap(),8,sizeof(PROPERTIES));
	entity     = HeapAlloc(GetProcessHeap(),8,sizeof(ENTITY) * 512);

	wndclass.hInstance = GetModuleHandle(0);
	RegisterClass(&wndclass);
	window = CreateWindowEx(0,name,name,0x90080000,0,0,resy + 16,resx + 39,0,0,wndclass.hInstance,0);
	dc = GetDC(window);

	player->xfov   = 16/9;
	player->yfov   = 1;
	player->xspawn = 5.5;
	player->yspawn = 5.5;
	player->zspawn = 2.5;
	player->zpos   = player->zspawn;
	player->xpos   = player->xspawn;
	player->ypos   = player->yspawn;

	properties->lvlSz          = MAPSZ;
	properties->renderDistance = RENDERDISTANCE;
	properties->xres           = resx;
	properties->yres           = resy;
	properties->fog            = 0.5;

	settings = 1;
	ShowCursor(0);

	levelgen();

	initSound();

	renderingThread      = CreateThread(0,0,openGL,0,0,0);
	physicsThread        = CreateThread(0,0,physics,0,0,0);
	entitiesThread       = CreateThread(0,0,entities,0,0,0);
	ittmapThread         = CreateThread(0,0,ittmap,0,0,0);

	for(;;){
		while(PeekMessage(&Msg,window,0,0,0)){
			GetMessage(&Msg,window,0,0);
			TranslateMessage(&Msg);
			DispatchMessageW(&Msg);
		}
		Sleep(1);
	}
}

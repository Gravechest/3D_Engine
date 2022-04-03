#include <main.h>
#include <windows.h>
#include <math.h>
#include <intrin.h>
#include <stdio.h>
#include <glew.h>
#include <GL/gl.h>


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
/*
bit
1: fly/walk
2: fullScreen
3: lighting
4: 
5:
6:
7:
8: pauze


*/

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

const char name[] = "3D_engine";

HINSTANCE hInstance;
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
	case 9:
		break;
	case 12:
		specialBlock[0] = map[block];
		specialBlock[1] |= axis;
		break;
	case 14:
		playerDeath();
		break;
	case 49:
		if(axis & 0x21){
			break;
		}
		touchStatus |= axis;
		break;
	case 50:
		if(axis & 0x11){
			break;
		}
		touchStatus |= axis;
	case 51:
		if(axis & 0x9){
			break;
		}
		touchStatus |= axis;
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
	case 9:
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
	case 50:
		if(touchStatus){
			break;	
		}
		if(y-(int)y<z-(int)z){
			specialBlock[0] = map[block];
			specialBlock[1] |= axis;
		}
		break;
	case 51:
		if(touchStatus){
			break;	
		}
		if(x-(int)x<z-(int)z){
			specialBlock[0] = map[block];
			specialBlock[1] |= axis;
		}
		break;
	case 67:
		player->xpos = mapdata[block] + player->xpos - (int)player->xpos;
		player->ypos = mapdata[block+1] + player->ypos - (int)player->ypos;
		player->zpos = mapdata[block+2] + player->zpos - (int)player->zpos+1.0;
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
	while(green > 0.003 || red > 0.003 || blue > 0.003){
		rayItterate(ray);
		if(ray->ix < 0 || ray->iy < 0 || ray->iz < 0 || ray->ix >= properties->lvlSz || ray->iy >= properties->lvlSz || ray->iz >= properties->lvlSz){
			break;
		}
		int block = crds2map(ray->ix,ray->iy,ray->iz);
		if(map[block]){
			switch(map[block]){
			case 9:
				break;
			case 20:
				if((float)rand() / RAND_MAX > 1.0){
					goto dflt;
				}
				break;
			case 21:
				break;
			case 60:
				break;	
			dflt:
			default:{
				VEC3 p;
				VEC3 d;
				switch(ray->side){
				case 0:
					if(ray->vx < 0.0){
						p.x = ray->ix+1.0;
					}
					else{
						p.x = ray->ix;
					}
					p.y = ray->y + (ray->sidex - ray->deltax) * ray->vy;
					p.z = ray->z + (ray->sidex - ray->deltax) * ray->vz;
					break;
				case 1:
					if(ray->vy < 0.0){
						p.y = ray->iy+1.0;
					}
					else{
						p.y = ray->iy;
					}
					p.x = ray->x + (ray->sidey - ray->deltay) * ray->vx;
					p.z = ray->z + (ray->sidey - ray->deltay) * ray->vz;
					break;
				case 2:
					if(ray->vz < 0.0){
						p.z = ray->iz+1.0;
					}
					else{
						p.z = ray->iz;
					}
					p.x = ray->x + (ray->sidez - ray->deltaz) * ray->vx;
					p.y = ray->y + (ray->sidez - ray->deltaz) * ray->vy;
					break;
				}
				float r1 = (float)rand() /RAND_MAX * 3.14 - 3.14;
				float r2 = (float)rand() /RAND_MAX * 3.14 - 3.14;
				float r3 = r1 + r2;
				float r4 = (float)rand() /RAND_MAX * 6.28 - 3.14;
				d.x = cosf(r4) * cosf(r3);
				d.y = sinf(r4) * cosf(r3);
				d.z = sinf(r3);
				switch(ray->side){
				case 0:
					if(ray->vx < 0.0 && d.x < 0.0){
						red = 0.0;
						green = 0.0;
						blue = 0.0;
					}
					if(ray->vx >= 0.0 && d.x >= 0.0){
						red = 0.0;
						green = 0.0;
						blue = 0.0;
					}
					break;
				case 1:
					if(ray->vy < 0.0 && d.y < 0.0){
						red = 0.0;
						green = 0.0;
						blue = 0.0;
					}
					if(ray->vy >= 0.0 && d.y >= 0.0){
						red = 0.0;
						green = 0.0;
						blue = 0.0;
					}
					break;
				case 2:
					if(ray->vz < 0.0 && d.z < 0.0){
						red = 0.0;
						green = 0.0;
						blue = 0.0;
					}
					if(ray->vz >= 0.0 && d.z >= 0.0){
						red = 0.0;
						green = 0.0;
						blue = 0.0;
					}
					break;
				}
				*ray = rayCreate(p.x,p.y,p.z,d.x,d.y,d.z);
				break;
				}
			}
			switch(map[block]){
			case 9:
				if(lightmap[block+1] > 255 || lightmap[block+2] > 255 || lightmap[block+3] > 255){
					continue;
				}
				lightmap[block+1] += red;
				lightmap[block+2] += green;
				lightmap[block+3] += blue;
				red   *= 0.95;
				green *= 0.95;
				blue  *= 0.95;
				continue;
			case 20:
				if(lightmap[block+1] > 255 || lightmap[block+2] > 255 || lightmap[block+3] > 255){
					continue;
				}
				lightmap[block+1] += red * 0.45;
				lightmap[block+2] += green * 0.45;
				lightmap[block+3] += blue * 0.45;
				continue;
			case 21:{
				float r = mapdata[block];
				float g = mapdata[block+1];
				float b = mapdata[block+2];
				float m = fmaxf(r,fmaxf(g,b));
				r /= m;
				g /= m;
				b /= m;
				red   *= r;
				green *= g;
				blue  *= b;
				break;
			}
			case 28:
				red   *= (float)mapdata[block]*0.005;
				green *= (float)mapdata[block+1]*0.005;
				blue  *= (float)mapdata[block+2]*0.005;
				continue;
			case 29:
				red   *= 0.5;
				green *= 0.05;
				blue  *= 0.05;
				break;
			default:
				red   *= 1.0;
				green *= 1.0;
				blue  *= 1.0;
				continue;
			}
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
	mapdata[pos] = colorSel.r;
	mapdata[pos+1] = colorSel.g;
	mapdata[pos+2] = colorSel.b;
	mapdata[pos+3] = colorSel.a;
	glMes[glMesC].id = 6;
	glMesC++;
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
		break;
	case WM_KEYDOWN:
		switch(wParam){
		case VK_LCONTROL:
			if(!abilities & 0x01){ 
				player->xvel *= 2.5	;
				player->yvel *= 2.5;
				player->zvel -= 0.3;
				abilities ^= 0x01;
			}
			break;
		case VK_F1:
			settings ^= 0x01;
			break;
		case VK_F5:
			settings ^= 0x08;
			if(settings & 0x08){
				for(int i = 0;i < MAPRAM;i+=4){
					if(map[i] == 0){
						map[i] = 9;
					}
				}
				glMes[glMesC].id = 3;
				glMesC++;
			}
			else{
				for(int i = 0;i < MAPRAM;i+=4){
					if(map[i] == 9){
						map[i] = 0;
					}
				}
				glMes[glMesC].id = 3;
				glMesC++;
			}
			break;
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
				if(GetKeyState(VK_LCONTROL) & 0x80){
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
				if(GetKeyState(VK_LCONTROL) & 0x80){
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
			SetCursorPos(50,50);
			if(mousex > 0.5 || mousey > 0.5){
				break;
			}
			player->xangle += mousex;
			player->yangle -= mousey;
			if(player->yangle < -1.6){
				player->yangle = -1.6;
			}
			if(player->yangle > 1.6){
				player->yangle = 1.6;
			}
			updateCamera();
		}
		break;
	case WM_MBUTTONDOWN:{
			RAY ray = rayCreate(player->xpos,player->ypos,player->zpos,player->xdir*player->xydir,player->ydir*player->xydir,player->zdir);
			while(ray.ix>=0&&ray.ix<=properties->lvlSz&&ray.iy>=0&&ray.iy<=properties->lvlSz&&ray.iz>=0&&ray.iz<=properties->lvlSz){
				int block = crds2map(ray.ix,ray.iy,ray.iz);
				if(map[block]){
					switch(toolSel){
					case 4:{
						CVEC3 scrd = map2crds(block);
						switch(ray.side){
						case 0:
							if(ray.vx < 0.0){
								scrd.x--;
							}
							else{
								scrd.x++;
							}
							break;
						case 1:
							if(ray.vy < 0.0){
								scrd.y--;
							}
							else{
								scrd.y++;
							}
							break;
						case 2:
							if(ray.vz < 0.0){
								scrd.z--;
							}
							else{
								scrd.z++;
							}
							break;
						}
						colorSel.r = scrd.x;
						colorSel.g = scrd.y;
						colorSel.b = scrd.z;
						break;
					}
					case 5:
						colorSel.r = mapdata[block];
						colorSel.g = mapdata[block+1];
						colorSel.b = mapdata[block+2];
						colorSel.a = mapdata[block+3];
						break;
					default:
						blockSel = map[block];
						colorSel.r = mapdata[block];
						colorSel.g = mapdata[block+1];
						colorSel.b = mapdata[block+2];
						colorSel.a = mapdata[block+3];
						break;
					}
					break;
				}
				rayItterate(&ray);
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
			while(ray.ix>=0&&ray.ix<=properties->lvlSz&&ray.iy>=0&&ray.iy<=properties->lvlSz&&ray.iz>=0&&ray.iz<=properties->lvlSz){
				int block = crds2map(ray.ix,ray.iy,ray.iz);
				if(map[block]){
					deleteBlock(block);
					break;
				}
				rayItterate(&ray);
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
				player->xpos += player->xdir / 8 * amp;
				player->ypos += player->ydir / 8 * amp;
			}
			if (GetKeyState(0x53) & 0x80)
			{
				player->xpos -= player->xdir / 8 * amp;
				player->ypos -= player->ydir / 8 * amp;
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
				player->xvel += player->xdir / 50 * amp;
				player->yvel += player->ydir / 50 * amp;

			}
			if (GetKeyState(0x53) & 0x80)
			{
				player->xvel -= player->xdir / 50 * amp;
				player->yvel -= player->ydir / 50 * amp;
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
			case 50:
				player->zpos -= player->zvel;
				player->zpos += player->yvel;
				break;
			case 51:
				player->zpos -= player->zvel;
				player->zpos += player->xvel;
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
	hInstance = wndclass.hInstance;
	dc = GetDC(window);

	HICON hIcon = LoadImage(0,"textures/phill.ico",IMAGE_ICON,32,32,LR_LOADFROMFILE);
	SendMessage(window,WM_SETICON,ICON_SMALL,(long int)hIcon);

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

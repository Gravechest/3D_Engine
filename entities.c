#include <main.h>
#include <stdio.h>
#include <math.h>

char sprite;

max3f(float val1,float val2,float val3){
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

int entityC;
ENTITY *entity;

void entityDeath(int id){
	for(int i = id;i < entityC;i++){
		entity[i] = entity[i+1];
	}
	entityC--;
}

int crds2map(int x,int y,int z){
	return ((int)x+(int)y*properties->lvlSz+(int)z*properties->lvlSz*properties->lvlSz)*4;
}

float spriteXdir(int i){
	float repx = entity[i].x-player->xpos;
	float repy = entity[i].y-player->ypos;
	float dep  = sqrtf(repx*repx+repy*repy);
	float dcex = cosf(player->xangle) * dep;
	float dcey = sinf(player->xangle) * dep;
	float dre  = sqrtf((repx-dcex)*(repx-dcex)+(repy-dcey)*(repy-dcey));
	dcex = cosf(player->xangle+0.01)*dep;
	dcey = sinf(player->xangle+0.01)*dep;
	float dre2 = sqrtf((repx-dcex)*(repx-dcex)+(repy-dcey)*(repy-dcey));
	float ei;
	if(dre > dre2){
		ei = (dre/dep);
	}
	else{
		ei = -(dre/dep);
	}
	return ei * fabsf(ei*0.5) + ei * 0.35;
}

float spriteYdir(int i){
	float repx = entity[i].x-player->xpos;
	float repy = entity[i].y-player->ypos;
	float repz = entity[i].z-player->zpos;
	float dep  = sqrtf(repx*repx+repy*repy+repz*repz);
	float dcez = sinf(player->yangle) * dep;
	float dre  = dcez - repz;
	float ei   = -(dre/dep) * 1.45;
	return ei * fabsf(ei*0.5) + ei * 0.35;
}

float spriteSize(int i){
	float repx = entity[i].x-player->xpos;
	float repy = entity[i].y-player->ypos;
	float repz = entity[i].z-player->zpos;
	float s = sqrtf(repx*repx+repy*repy+repz*repz);
	return 2.0/s;
}

unsigned char checkLOS(float x,float y,float z){
	float nx = player->xpos-x;
	float ny = player->ypos-y;
	float nz = player->zpos-z;
	float nm = fmaxf(fabsf(nx),fmaxf(fabsf(ny),fabsf(nz)));
	nx/=nm;
	ny/=nm;
	nz/=nm;
	RAY ray = rayCreate(x,y,z,nx,ny,nz);
	for(;;){
		rayItterate(&ray);
		int block2 = crds2map(ray.ix,ray.iy,ray.iz);
		if(map[block2]){
			return 0;
		}
		if(ray.ix == (int)player->xpos && ray.iy == (int)player->ypos && ray.iz == (int)player->zpos){
			return 1;
		}
	}
}

void entities(){
	for(;;){
		for(int i = 0;i < entityC;i++){
			entity[i].x += entity[i].vx;
			entity[i].y += entity[i].vy;
			entity[i].z += entity[i].vz;
			if(map[crds2map(entity[i].x,entity[i].y,entity[i].z)]){
				switch(map[crds2map(entity[i].x,entity[i].y,entity[i].z)]){
				case 5:
					break;
				default:
					entityDeath(i);
					i--;
					break;
				}
			}
			float rpx = entity[i].x-entity[i].sz*0.5;
			float rpy = entity[i].y-entity[i].sz*0.5;
			float rpz = entity[i].z-entity[i].sz*0.5;
			if(!checkLOS(rpx,rpy,rpz)){
				continue;
			}
			rpx += entity[i].sz;
			if(!checkLOS(rpx,rpy,rpz)){
				continue;
			}
			rpy += entity[i].sz;
			if(!checkLOS(rpx,rpy,rpz)){
				continue;
			}
			rpz += entity[i].sz;
			if(!checkLOS(rpx,rpy,rpz)){
				continue;
			}
			rpy -= entity[i].sz;
			if(!checkLOS(rpx,rpy,rpz)){
				continue;
			}
			rpx -= entity[i].sz;
			if(!checkLOS(rpx,rpy,rpz)){
				continue;
			}
			rpz -= entity[i].sz;
			rpy += entity[i].sz;
			if(!checkLOS(rpx,rpy,rpz)){
				continue;
			}
			rpz += entity[i].sz;
			if(!checkLOS(rpx,rpy,rpz)){
				continue;
			}
			glMes[glMesC].id = 4;
			float sprz = spriteSize(i)*entity[i].sz;
			glMes[glMesC].fdata1 = spriteXdir(i)-sprz/2.0/1.69;
			glMes[glMesC].fdata2 = spriteYdir(i)-sprz/2.0;
			glMes[glMesC].fdata3 = sprz;
			glMes[glMesC].fdata4 = sprz;
			glMes[glMesC].fdata5 = -sprz;
			glMes[glMesC].fdata6 = entity[i].id;
			glMesC++;
		}
		sprite = 1;
		while(sprite){
			Sleep(1);
		}
	}
}












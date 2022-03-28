#include <main.h>
#include <math.h>

CVEC3 selarea;

RAY rayCreate(float x,float y,float z,float rx,float ry,float rz){
	RAY ray;
	ray.x  		= x;
	ray.y  		= y;
	ray.z  		= z;
	ray.vx		= rx;
	ray.vy 		= ry;
	ray.vz 		= rz;
	
	ray.deltax  = fabsf(1.0/ray.vx);
	ray.deltay  = fabsf(1.0/ray.vy);
	ray.deltaz  = fabsf(1.0/ray.vz);

	if(ray.vx < 0.0){
		ray.stepx = -1;
		ray.sidex = (ray.x-(int)ray.x) * ray.deltax;
	}
	else{
		ray.stepx = 1;
		ray.sidex = ((int)ray.x + 1.0 - ray.x) * ray.deltax;
	}
	if(ray.vy < 0.0){
		ray.stepy = -1;
		ray.sidey = (ray.y-(int)ray.y) * ray.deltay;
	}
	else{
		ray.stepy = 1;
		ray.sidey = ((int)ray.y + 1.0 - ray.y) * ray.deltay;
	}
	if(ray.vz < 0.0){
		ray.stepz = -1;
		ray.sidez = (ray.z-(int)ray.z) * ray.deltaz;
	}
	else{
		ray.stepz = 1;
		ray.sidez = ((int)ray.z + 1.0 - ray.z) * ray.deltaz;
	}
	ray.ix = ray.x;
	ray.iy = ray.y;
	ray.iz = ray.z;
	return ray;
}

void tools(){
	switch(toolSel){
	case 0:{
		RAY ray = rayCreate(player->xpos,player->ypos,player->zpos,player->xdir*player->xydir,player->ydir*player->xydir,player->zdir);
		for(int i = 0;i < 254;i++){
			rayItterate(&ray);
			int block = crds2map(ray.ix,ray.iy,ray.iz);
			if(map[block]){
				switch(ray.side){
				case 0:
					if(ray.vx < 0.0){
						ray.x+=1.0;
						updateBlock(block + 1 * 4,blockSel);
					}
					else{
						ray.x-=1.0;
						updateBlock(block - 1 * 4,blockSel);
					}
					break;
				case 1:
					if(ray.vy < 0.0){
						ray.y+=1.0;
						updateBlock(block + properties->lvlSz * 4,blockSel);
					}
					else{
						ray.y-=1.0;
						updateBlock(block - properties->lvlSz * 4,blockSel);
					}
					break;
				case 2:
					if(ray.vz < 0.0){
						ray.z+=1.0;
						updateBlock(block + properties->lvlSz * properties->lvlSz * 4,blockSel);
					}
					else{
						ray.z-=1.0;
						updateBlock(block - properties->lvlSz * properties->lvlSz * 4,blockSel);
					}
					break;
				}
				switch(blockSel){
				case 8:
					mapdata[block]=50;
					mapdata[block+1]=50;
					mapdata[block+2]=50;
					glMes[glMesC].id = 6;
					glMesC++;
					break;
				case 67:
					toolSel = 4;
					selarea.x = ray.x;
					selarea.y = ray.y;
					selarea.z = ray.z;
					break;
				}
				break;
			}
		}
		break;
		}
	case 1:{
		RAY ray = rayCreate(player->xpos,player->ypos,player->zpos,player->xdir*player->xydir,player->ydir*player->xydir,player->zdir);
		for(int i = 0;i < 254;i++){
			rayItterate(&ray);
			if(ray.x < 0 || ray.y < 0 || ray.z < 0){
				break;
			}	
			int block = crds2map(ray.ix,ray.iy,ray.iz);
			if(map[block]){
				if(!selarea.x && !selarea.y && !selarea.z){
					selarea.x = ray.ix;
					selarea.y = ray.iy;
					selarea.z = ray.iz;
				}
				else{
					unsigned char x = ray.ix;
					unsigned char y = ray.iy;
					unsigned char z = ray.iz;
					if(selarea.z > ray.iz){
						selarea.z ^= z;
						z ^= selarea.z;
						selarea.z ^= z;
					}
					if(selarea.x > ray.ix){
						selarea.x ^= x;
						x ^= selarea.x;
						selarea.x ^= x;
					}
					if(selarea.y > ray.iy){
						selarea.y ^= y;
						y ^= selarea.y;
						selarea.y ^= y;
					}
					for(int i = selarea.x;i <= x;i++){
						for(int i2 = selarea.y;i2 <= y;i2++){
							for(int i3 = selarea.z;i3 <= z;i3++){
								int block = (i + i2 * properties->lvlSz + i3 * properties->lvlSz * properties->lvlSz) * 4;
								map[block] = blockSel;
							}
						}
					}
					selarea.x = 0;
					selarea.y = 0;
					selarea.z = 0;
					glMes[glMesC].id = 3;
					glMesC++;
				}
				break;
			}
		}
		break;
	}
	case 2:{
		RAY ray = rayCreate(player->xpos,player->ypos,player->zpos,player->xdir*player->xydir,player->ydir*player->xydir,player->zdir);
		for(int i = 0;i < 254;i++){
			rayItterate(&ray);
			if(ray.x < 0 || ray.y < 0 || ray.z < 0){
				break;
			}	
			int block = crds2map(ray.ix,ray.iy,ray.iz);
			if(map[block]){
				if(GetKeyState(VK_LCONTROL) & 0x80){
					updateBlockLight(block);
				}
				else{
					if(ray.z - (int)ray.z == 0){
						updateBlockLight(block - properties->lvlSz * properties->lvlSz * 4);
					}
					else if(ray.y - (int)ray.y == 0){
						updateBlockLight(block - properties->lvlSz * 4);
					}
					else if(ray.y - (int)ray.y > 0.99998){
						updateBlockLight(block + properties->lvlSz * 4);
					}
					else if(ray.z - (int)ray.z > 0.99998){
						updateBlockLight(block + properties->lvlSz * properties->lvlSz * 4);
					}
					else if(ray.x - (int)ray.x > 0.99998){
						updateBlockLight(block + 1 * 4);
					}
					else{
						updateBlockLight(block - 1 * 4);
					}
				}
				break;
			}
		}
		break;	
		}
	case 3:
		break;		
	case 4:{
		RAY ray = rayCreate(player->xpos,player->ypos,player->zpos,player->xdir*player->xydir,player->ydir*player->xydir,player->zdir);
		for(int i = 0;i < 254;i++){
			rayItterate(&ray);
			if(ray.x < 0 || ray.y < 0 || ray.z < 0){
				break;
			}	
			int block = crds2map(ray.ix,ray.iy,ray.iz);
			if(map[block]){
				if(!selarea.x && !selarea.y && !selarea.z){
					selarea.x = ray.x;
					selarea.y = ray.y;
					selarea.z = ray.z;
				}
				else{
					block = (selarea.x+selarea.y*properties->lvlSz+selarea.z*properties->lvlSz*properties->lvlSz)*4;
					if(ray.z - (int)ray.z == 0){
						mapdata[block]   = ray.x;
						mapdata[block+1] = ray.y;
						mapdata[block+2] = ray.z-1;
					}
					else if(ray.y - (int)ray.y == 0){
						mapdata[block]   = ray.x;
						mapdata[block+1] = ray.y-1;
						mapdata[block+2] = ray.z;
					}
					else if(ray.y - (int)ray.y > 0.99998){
						mapdata[block]   = ray.x;
						mapdata[block+1] = ray.y+1;
						mapdata[block+2] = ray.z;
					}
					else if(ray.z - (int)ray.z > 0.99998){
						mapdata[block]   = ray.x;
						mapdata[block+1] = ray.y;
						mapdata[block+2] = ray.z+1;
					}
					else if(ray.x - (int)ray.x > 0.99998){
						mapdata[block]   = ray.x+1;
						mapdata[block+1] = ray.y;
						mapdata[block+2] = ray.z;
					}
					else{
						mapdata[block]   = ray.x-1;
						mapdata[block+1] = ray.y;
						mapdata[block+2] = ray.z;
					}
					glMes[glMesC].data1 = selarea.x;
					glMes[glMesC].data2 = selarea.y;
					glMes[glMesC].data3 = selarea.z;
					glMes[glMesC].id = 5;
					glMesC++;
					selarea.x = 0;
					selarea.y = 0;
					selarea.z = 0;
				}
				break;
			}
		}
		break;
	}
	case 5:{
		RAY ray = rayCreate(player->xpos,player->ypos,player->zpos,player->xdir*player->xydir,player->ydir*player->xydir,player->zdir);
		for(int i = 0;i < 254;i++){
			rayItterate(&ray);
			if(ray.x < 0 || ray.y < 0 || ray.z < 0){
				break;
			}
			int block = crds2map(ray.ix,ray.iy,ray.iz);
			if(map[block]){
				mapdata[block]   = colorSel.r;
				mapdata[block+1] = colorSel.g;
				mapdata[block+2] = colorSel.b;
				mapdata[block+3] = colorSel.a;
				glMes[glMesC].id = 6;
				glMesC++;
				break;
			}
		}
		break;
		}
	}
}

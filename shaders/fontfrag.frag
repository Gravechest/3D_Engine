#version 460 core

in vec2 TexCoord;
in float ID;

out vec4 FragColor;

uniform sampler2D font;
uniform ivec2 reso;

void main(){
	switch(int(ID)){
	case 0:
		vec4 ftext = texture(font,vec2(TexCoord.x,1.0 - TexCoord.y));
		if(ftext.r == 0.0){
			discard;
		}	
		FragColor += ftext;
		FragColor.rgb /= 2;
		return;
	case 1:
		float d = 1.0-pow(distance(vec2(0.5),TexCoord),2.0)*5.0;
		if(d > 0.0){
			FragColor.r = d;
			FragColor.a = 1.0;
			return;
		}
		discard;
	}
}

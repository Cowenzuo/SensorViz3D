#version 110
uniform sampler2D textureID;
uniform int valuesNum;
uniform vec4 values[100];
varying vec3 vecWorldPos;
varying vec3 ecNormal;
vec4 HSLToRGB(float H, float S, float L)
{	
	H = mod(H, 360.0);
	if (H < 0.0) H += 360.0;

	float C = (1.0 - abs(2.0 * L - 1.0)) * S;
	float X = C * (1.0 - abs(mod(H / 60.0, 2.0) - 1.0));
	float m = L - C / 2.0;
	float R, G, B;
	if (H >= 0.0 && H < 60.0)
	{
		R = C; G = X; B = 0.0;
	}
	else if (H >= 60.0 && H < 120.0)
	{
		R = X; G = C; B = 0.0;
	}
	else if (H >= 120.0 && H < 180.0)
	{
		R = 0.0; G = C; B = X;
	}
	else if (H >= 180.0 && H < 240.0)
	{
		R = 0.0; G = X; B = C;
	}
	else if (H >= 240.0 && H < 300.0)
	{
		R = X; G = 0.0; B = C;
	}
	else
	{
		R = C; G = 0.0; B = X;
	}
	return vec4((R + m), (G + m), (B + m), 1.0);
}
void main()
{
    vec4 color = texture2D(textureID, gl_TexCoord[0].xy);
	float len[100];
	float lengthAll;
	for(int i=0;i<valuesNum;++i)
	{
		vec4 value = values[i];
		len[i] = 1.0 / pow(length(vecWorldPos - value.xyz),2.0);
		lengthAll += len[i];
	}
	float fValueAll;
	for(int i=0;i<valuesNum;++i)
	{
		float fValue = values[i].w;
		//float fScale = len[i] / lengthAll;
		fValueAll += (fValue *  len[i]);
	}
	fValueAll /= lengthAll;
	if(valuesNum>0)
	{
		color = HSLToRGB((1.0 - fValueAll)*240.0,1.0,0.5);
	}
	else
	{
	    color = HSLToRGB(240.0,1.0,0.5);
	}	
	
    vec3 lightDir = normalize(vec3(gl_LightSource[0].position));
    vec3 normal   = normalize(ecNormal);
    float NdotL   = max(dot(normal, lightDir), 0.0);
    gl_FragColor =  NdotL * color * gl_LightSource[0].diffuse +  color * gl_LightSource[0].ambient;
}
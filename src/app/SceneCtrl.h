#pragma once

#include <QObject>

#include <osg/Node>
#include <osg/Program>

#include "ProjectData.h"

#define SHADER_VERT R"(
	#version 110

	uniform mat4 osg_ViewMatrixInverse;

	varying vec3 vecWorldPos;
	varying vec3 vecNormal;

	void main()
	{
	  gl_TexCoord[0] = gl_MultiTexCoord0;
	  vecWorldPos = (osg_ViewMatrixInverse * gl_ModelViewMatrix * gl_Vertex).xyz;
	  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	  vecNormal = normalize( (gl_NormalMatrix * gl_Normal).xyz );
	}
)"
#define SHADER_FRAG R"(
	#version 110

	uniform int uValueNum;
	uniform vec3 uPositions[20];
	uniform float uValues[20];

	varying vec3 vecWorldPos;
	varying vec3 vecNormal;

	vec4 HSLToRGB(float H, float S, float L)
	{	
		H = mod(H, 360.0);
		if (H < 0.0) H += 360.0;
		float C = (1.0 - abs(2.0 * L - 1.0)) * S;
		float X = C * (1.0 - abs(mod(H / 60.0, 2.0) - 1.0));
		float m = L - C / 2.0;
		float R, G, B;
		if (H >= 0.0 && H < 60.0){R = C; G = X; B = 0.0;}
		else if (H >= 60.0 && H < 120.0){R = X; G = C; B = 0.0;}
		else if (H >= 120.0 && H < 180.0){R = 0.0; G = C; B = X;}
		else if (H >= 180.0 && H < 240.0){R = 0.0; G = X; B = C;}
		else if (H >= 240.0 && H < 300.0){R = X; G = 0.0; B = C;}
		else{R = C; G = 0.0; B = X;}
		return vec4((R + m), (G + m), (B + m), 1.0);
	}
	void main()
	{
		vec4 color = vec4(0.5,0.5,0.5,1.0);
		if(0<uValueNum){
			float len[20];
			float lengthAll=0.0;
			for(int i=0;i<uValueNum;++i)
			{
				vec3 pos = uPositions[i];
				len[i] = 1.0 / pow(length(vecWorldPos - pos.xyz),2.0);
				lengthAll += len[i];
			}
			float fValueAll=0.0;
			for(int i=0;i<uValueNum;++i)
			{
				fValueAll += (uValues[i] *  len[i]);
			}
			if(0.0 < lengthAll && 0.0 < fValueAll){
				fValueAll /= lengthAll;
				color = HSLToRGB((1.0 - fValueAll)*240.0,1.0,0.5);
			}
			else{
				color =vec4(0.0,0.0,1.0,1.0);
			}
		}
	    vec3 lightDir = normalize(vec3(gl_LightSource[0].position));
	    vec3 normal   = normalize(vecNormal);
	    float NdotL   = max(dot(normal, lightDir), 0.0);
	    gl_FragColor =  NdotL * color * gl_LightSource[0].diffuse +  color * gl_LightSource[0].ambient;
	}
)"

class SceneViewer;

	class SceneCtrl :public QObject
	{
		Q_OBJECT
	public:
		SceneCtrl(SceneViewer* sv, QObject* parent = nullptr);
		~SceneCtrl();

		bool installSimRender(QVector<SensorPositon> sp);
		//0~1.0之间的值，提前算好再给进来
		bool updateSimValues(QVector<float> values);

		bool uninstallSimRender();

	private:
		SceneViewer* _sceneViewer;

		osg::ref_ptr<osg::Node> _model;
		osg::ref_ptr<osg::Program> _simRender;

	};


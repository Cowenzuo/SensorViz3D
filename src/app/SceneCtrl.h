#pragma once

#include <QObject>

#include <osg/Node>
#include <osg/Program>

#include "ProjectData.h"

#define SHADER_VERT R"(
	#version 330
	layout(location = 0) in vec4 vertex;
	layout(location = 2) in vec3 normal;

	// osg build-in
	uniform mat4 osg_ModelViewProjectionMatrix;
	uniform mat4 osg_ModelViewMatrix;
	uniform mat4 osg_ViewMatrixInverse;
	uniform mat3 osg_NormalMatrix;

	uniform int uValueNum;
	uniform vec3 uPositions[20];
	uniform float uValues[20];
	uniform int uWeight = 0;
	uniform int uDisplacement = 1;
	uniform float uDispScale = 1.0;
	uniform float uMinValue = 0.0;
	uniform float uRangeValue = 0.0;


	out vec3 oWorldPos;
	out vec3 oNormal;

	void main(void) {
	    // 初始模型坐标
	    vec4 displacedVertex = vertex;
	    
	    // 计算原始世界坐标（注意：实际得到的是模型坐标，建议检查矩阵运算）
	    oWorldPos = (osg_ViewMatrixInverse * osg_ModelViewMatrix * displacedVertex).xyz;
	    oNormal = normalize(osg_NormalMatrix * normal);
	 
	    if(uValueNum > 0 && uDisplacement == 1) {
	        float weightedSum = 0.0;
	        float weightTotal = 0.0;
	        
	        // 计算基于世界坐标的权重
	        for(int i = 0; i < uValueNum; i++) {
	            float distance = length(oWorldPos - uPositions[i]);
	            float w = 1.0 / (distance + 0.0001); // 添加极小值防止除零
	            weightedSum += w * uValues[i];
	            weightTotal += w;
	        }
	        
	        // 归一化并钳制值
	        float interpolatedValue = weightTotal > 0.0 ? weightedSum / weightTotal : 0.0;
	        float normalizedValue = clamp(interpolatedValue, 0.0, 1.0);
			float trueValueScale =(normalizedValue * uRangeValue + uMinValue)* uDispScale;
	 
	        // 应用位移到模型坐标
	        if(uWeight == 0) {
	            displacedVertex.xyz += vec3(trueValueScale, 0.0, 0.0);
	        }
			else if(uWeight == 1) {
	            displacedVertex.xyz += vec3(0.0, trueValueScale, 0.0);
	        }
			else if(uWeight == 2) {
	            displacedVertex.xyz += vec3(0.0, 0.0, trueValueScale);
	        }
			
	    }
	    // 更新世界坐标输出
	    oWorldPos = (osg_ViewMatrixInverse * osg_ModelViewMatrix * displacedVertex).xyz;
	    // 计算最终位置
	    gl_Position = osg_ModelViewProjectionMatrix * displacedVertex;
	}
)"
#define SHADER_FRAG R"(
	#version 330

	layout(location = 0) out vec4 fragcolor;

	in vec3 oWorldPos;
	in vec3 oNormal;

	uniform int uValueNum;
	uniform vec3 uPositions[20];
	uniform float uValues[20];

	// build-in
	uniform vec3 uLightDirection = normalize(vec3(0.0, 0.0, 1.0));  // Z轴朝上坐标系
	uniform vec4 uLightDiffuse = vec4(1.0, 1.0, 1.0, 1.0);
	uniform vec4 uLightAmbient = vec4(0.2, 0.2, 0.2, 1.0);

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
	vec4 HSLToRGB2(float H, float S, float L)
	{
	    H = clamp(H, 0.0, 240.0);
		float quantizedH = floor((H + 15.0) / 30.0) * 30.0;
		quantizedH = min(quantizedH, 240.0);
	    float C = (1.0 - abs(2.0 * L - 1.0)) * S;
	    float X = C * (1.0 - abs(mod(quantizedH / 60.0, 2.0) - 1.0));
	    float m = L - C / 2.0;
	    
	    float R, G, B;
	    if (quantizedH < 60.0) {
	        R = C; G = X; B = 0.0;
	    } else if (quantizedH < 120.0) {
	        R = X; G = C; B = 0.0;
	    } else if (quantizedH < 180.0) {
	        R = 0.0; G = C; B = X;
	    } else {
	        R = 0.0; G = X; B = C;
	    }
	    
	    return vec4((R + m), (G + m), (B + m), 1.0);
	}
	void main()
	{
		vec4 color = vec4(0.5,0.5,0.5,1.0);
		if(0<uValueNum){
            float weightedSum = 0.0;
            float weightTotal = 0.0;
			for (int i = 0; i < uValueNum; i++) {
                float distance = length(oWorldPos - uPositions[i]);
				float weight = 1.0 / distance;
				weightedSum += weight * uValues[i];
				weightTotal += weight;
            }
            float interpolatedValue = weightTotal > 0.0 ? weightedSum / weightTotal : 0.0;
			float normalizedValue = clamp(interpolatedValue, 0.0, 1.0);
			color = HSLToRGB2((1.0 - normalizedValue)*240.0,1.0,0.5);
		}
		float NdotL   = max(dot(oNormal, uLightDirection), 0.0);
		fragcolor =  NdotL * color * uLightDiffuse +  color * uLightAmbient;
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

		bool updateDispmentScaled(float value);

		bool updateWeight(int value);
		bool updateDisplacement(int value);
		bool updateDisplacementRange(float min,float range);

		bool uninstallSimRender();

	private:
		SceneViewer* _sceneViewer;

		osg::ref_ptr<osg::Node> _model;
		osg::ref_ptr<osg::Program> _simRender;

	};


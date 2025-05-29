#version 110
uniform mat4 osg_ViewMatrixInverse;
varying vec3 vecWorldPos;
varying vec3 ecNormal;
void main()
{
  gl_TexCoord[0] = gl_MultiTexCoord0;
  vecWorldPos = (osg_ViewMatrixInverse * gl_ModelViewMatrix * gl_Vertex).xyz;
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
  ecNormal = normalize( (gl_NormalMatrix * gl_Normal).xyz );
}
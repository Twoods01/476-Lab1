struct Material {
   vec3 aColor;
   vec3 dColor;
   vec3 sColor;
   float shine;
};
attribute vec3 aPosition;
attribute vec3 aNormal;

uniform mat4 uProjMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uModelMatrix;
uniform mat4 uNormalMatrix;
uniform vec3 uLightPos;
uniform vec3 uEyePos;

varying vec3 vNormal;
varying vec3 vLight;
varying vec3 vView;
varying vec3 vReflect;

void main() {
   vec4 vPosition;
   vec4 transNorm;
   
   /* First model transforms */
   vPosition = uModelMatrix * vec4(aPosition.x, aPosition.y, aPosition.z, 1);
   
   vLight.x = uLightPos.x - vPosition.x;
   vLight.y = uLightPos.y - vPosition.y;
   vLight.z = uLightPos.z - vPosition.z;
   vLight = normalize(vLight);
   
   transNorm = uNormalMatrix * vec4(aNormal, 0);
   transNorm = normalize(transNorm);
   vNormal.x = transNorm.x;
   vNormal.y = transNorm.y;
   vNormal.z = transNorm.z;
   
   vView.x = uEyePos.x - vPosition.x;
   vView.y = uEyePos.y - vPosition.y;
   vView.z = uEyePos.z - vPosition.z;
   vView = normalize(vView);
   
   vReflect = -vLight + 2.0*(max (0.0, dot(vLight, vNormal))) * vNormal;
   
   vPosition = uViewMatrix * vPosition;
   gl_Position = uProjMatrix * vPosition;
}

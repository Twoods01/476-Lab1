struct Material {
   vec3 aColor;
   vec3 dColor;
   vec3 sColor;
   float shine;
};


uniform vec3 uLColor;
uniform Material uMat;
uniform int uShowNormal;

varying vec3 vNormal;
varying vec3 vLight;
varying vec3 vView;
varying vec3 vReflect;

void main() {
   vec3 Spec, Diffuse;
   
   if(uShowNormal == 1)
      gl_FragColor = vec4(vNormal, 1);
   else
   {
      Diffuse = (uLColor * (max (0.0, dot(vLight, vNormal))) * uMat.dColor);
      Spec = uLColor * pow((dot(vView, vReflect)), uMat.shine) * uMat.sColor;
      gl_FragColor = vec4(Diffuse + Spec + uMat.aColor * uLColor, 1);
   }
}

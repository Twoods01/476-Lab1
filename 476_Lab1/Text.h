//
//  Text.h
//  476_Lab1
//
//

#ifndef _76_Lab1_Text_h
#define _76_Lab1_Text_h

#include "GameObject.hpp"
#include "glm/glm.hpp"

class Text : public GameObject
{
   public:
      void step();
      void draw();
      Text(glm::vec3 pos, glm::vec3 size, float rotation, GLHandles hand);
      glm::vec3 getPos();
      glm::vec3 getVel();
   private:
      void SetModel(glm::vec3 loc, glm::vec3 size, float rotation);
      GLuint textBuffObj, colBuffObj, textNormalBuffObj;
      int TriangleCount;
};

#endif

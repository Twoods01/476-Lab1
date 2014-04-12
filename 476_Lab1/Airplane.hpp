//
//  Airplane.h
//  476_Lab1
//
//  Created by Taylor Woods on 4/11/14.
//  Copyright (c) 2014 Taylor Woods. All rights reserved.
//

#ifndef _76_Lab1_Airplane_h
#define _76_Lab1_Airplane_h

#include "GameObject.hpp"
#include "glm/glm.hpp"

class Airplane : public GameObject
{
   public:
      void step();
      void draw();
      Airplane(glm::vec3 pos, glm::vec3 size, float rotation, GLHandles hand);
      glm::vec3 getPos();
      glm::vec3 getVel();
      void kill();
      void bounce(glm::vec3 normal);
   private:
      void SetModel(glm::vec3 loc, glm::vec3 size, float rotation);
      bool isAlive;
      GLuint planeBuffObj, colBuffObj, planeNormalBuffObj;
      int TriangleCount;
};

#endif

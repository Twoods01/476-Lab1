//
//  GameObject.h
//  476_Lab1
//
//  Created by Taylor Woods on 4/11/14.
//  Copyright (c) 2014 Taylor Woods. All rights reserved.
//

#ifndef GAME_OBJ
#define GAME_OBJ

#include "glm/glm.hpp"
#include "GLHandles.h"
using namespace std;

class GameObject
{
   protected:
      glm::vec3 position;
      glm::vec3 size;
      float rotation;
      glm::vec3 velocity;
      virtual void step() = 0;
      virtual void draw() = 0;
      GLHandles handles;
      timeval lastUpdated;
};

#endif

//
//  Airplane.c
//  476_Lab1
//
//  Created by Taylor Woods on 4/11/14.
//  Copyright (c) 2014 Taylor Woods. All rights reserved.
//

#ifndef AIRPLANE
#define AIRPLANE

#include <stdio.h>
#include <sys/time.h>
#include "CMeshLoaderSimple.h"
#include "GLSL_helper.h"
#include "Airplane.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp" //perspective, trans etc
#include "glm/gtc/type_ptr.hpp" //value_ptr

#define pi 3.14159

Airplane::Airplane(glm::vec3 pos, glm::vec3 size, float rotation, GLHandles hand)
{
   GameObject::position = pos;
   GameObject::size = size;
   GameObject::rotation = rotation;
   GameObject::handles = hand;
   GameObject::velocity = glm::vec3(-cos(rotation * (pi / 180)), 0, sin(rotation * (pi / 180)));
   //cout << "(" << GameObject::velocity.x << ", " << GameObject::velocity.y << ", " << GameObject::velocity.z << ")\n";
   isAlive = true;
   gettimeofday(&lastUpdated, NULL);
   CMeshLoader::loadVertexBufferObjectFromMesh("cessna500.m", TriangleCount,
      planeBuffObj, colBuffObj, planeNormalBuffObj);
}

int diff_ms(timeval t1, timeval t2)
{
   return (((t1.tv_sec - t2.tv_sec) * 1000000) +
           (t1.tv_usec - t2.tv_usec))/1000;
}

void Airplane::step()
{
   timeval curtime;
   gettimeofday(&curtime, NULL);
   if(isAlive)
      GameObject::position += ((float)(diff_ms(curtime, lastUpdated)) / 500.0f) * GameObject::velocity;
   else
      GameObject::position.y -= .05f;
   lastUpdated = curtime;
   //cout << "(" << GameObject::position.x << ", " << GameObject::position.y << ", " << GameObject::position.z << ")\n" << "Last Updated: " << lastUpdated.tv_usec << "\n";
   return;
}

glm::vec3 Airplane::getPos()
{
   return GameObject::position;
}

glm::vec3 Airplane::getVel()
{
   return GameObject::velocity;
}

void Airplane::kill()
{
   isAlive = false;
}

/* Set up matrices to place model in the world */
void Airplane::SetModel(glm::vec3 loc, glm::vec3 size, float rotation) {
   glm::mat4 Scale = glm::scale(glm::mat4(1.0f), size);
   glm::mat4 Trans = glm::translate(glm::mat4(1.0f), loc);
   glm::mat4 Rotate = glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0, 1, 0));
   
   glm::mat4 final = Trans * Rotate * Scale;
   safe_glUniformMatrix4fv(GameObject::handles.uModelMatrix, glm::value_ptr(final));
   safe_glUniformMatrix4fv(GameObject::handles.uNormMatrix, glm::value_ptr(glm::vec4(1.0f)));
}

//Change the planes rotation and velocity to bounce it
void Airplane::bounce(glm::vec3 normal)
{
   glm::vec3 reflect = -velocity + 2.0f * glm::dot(velocity, normal) * normal;
   float theta = acos(glm::dot(velocity, reflect) / (reflect.length() * velocity.length()));
   rotation += theta * (180 / pi);
   if(rotation > 180)
      rotation -= 360;
   GameObject::velocity = glm::vec3(-cos(rotation * (pi / 180)), 0, sin(rotation * (pi / 180)));
   return;
}

void Airplane::draw()
{
   //Enable handles
   safe_glEnableVertexAttribArray(handles.aPosition);
   safe_glEnableVertexAttribArray(handles.aNormal);
   
   SetModel(position, size, rotation);
   
   glBindBuffer(GL_ARRAY_BUFFER, planeBuffObj);
   safe_glVertexAttribPointer(GameObject::handles.aPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
   
   glBindBuffer(GL_ARRAY_BUFFER, planeNormalBuffObj);
   safe_glVertexAttribPointer(GameObject::handles.aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);
   
   glDrawArrays(GL_TRIANGLES, 0, TriangleCount * 3);
   //clean up
	safe_glDisableVertexAttribArray(handles.aPosition);
	safe_glDisableVertexAttribArray(handles.aNormal);
   return;
}

#endif

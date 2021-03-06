//
//  Text.c
//  476_Lab1
//

#ifndef TEXT
#define TEXT

#include <stdio.h>
#include "CMeshLoaderSimple.h"
#include "GLSL_helper.h"
#include "Text.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp" //perspective, trans etc
#include "glm/gtc/type_ptr.hpp" //value_ptr

#define pi 3.14159

Text::Text(glm::vec3 pos, glm::vec3 size, float rotation, GLHandles hand)
{
   GameObject::position = pos;
   GameObject::size = size;
   GameObject::rotation = rotation;
   GameObject::handles = hand;
   GameObject::velocity = glm::vec3(0.0, 0.0, 0.0);;
   //cout << "(" << GameObject::velocity.x << ", " << GameObject::velocity.y << ", " << GameObject::velocity.z << ")\n";
   CMeshLoader::loadVertexBufferObjectFromMesh("cessna500.m", TriangleCount,
      textBuffObj, colBuffObj, textNormalBuffObj);
   //NOT SURE WHAT TO CHANGE THIS TO, HMMM. ASCII CHARACTERS, MAYBE
   
}

void Text::step()
{
   return;
}

glm::vec3 Text::getPos()
{
   return GameObject::position;
}

glm::vec3 Text::getVel()
{
   return GameObject::velocity;
}


/* Set up matrices to place model in the world */
void Text::SetModel(glm::vec3 loc, glm::vec3 size, float rotation) {
   glm::mat4 Scale = glm::scale(glm::mat4(1.0f), size);
   glm::mat4 Trans = glm::translate(glm::mat4(1.0f), loc);
   glm::mat4 Rotate = glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0, 1, 0));
   
   glm::mat4 final = Trans * Rotate * Scale;
   safe_glUniformMatrix4fv(GameObject::handles.uModelMatrix, glm::value_ptr(final));
   safe_glUniformMatrix4fv(GameObject::handles.uNormMatrix, glm::value_ptr(glm::vec4(1.0f)));
}


void Text::draw()
{
   //Enable handles
   safe_glEnableVertexAttribArray(handles.aPosition);
   safe_glEnableVertexAttribArray(handles.aNormal);
   
   SetModel(position, size, rotation);
   
   glBindBuffer(GL_ARRAY_BUFFER, textBuffObj);
   safe_glVertexAttribPointer(GameObject::handles.aPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
   
   glBindBuffer(GL_ARRAY_BUFFER, textNormalBuffObj);
   safe_glVertexAttribPointer(GameObject::handles.aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);
   
   glDrawArrays(GL_TRIANGLES, 0, TriangleCount * 3);
   //clean up
	safe_glDisableVertexAttribArray(handles.aPosition);
	safe_glDisableVertexAttribArray(handles.aNormal);
   return;
}

#endif

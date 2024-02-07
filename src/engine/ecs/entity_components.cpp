#include "entity_components.h"
#include <iostream>
glm::mat3 TransformComponent::mat3()
{
    float cos = glm::cos(rotation);
    float sin = glm::sin(rotation);
    float scaleX = scale.x * localScale.x;
    float scaleY = scale.y * localScale.y;
    //Column major
    /*glm::mat3 mat = {
        scaleX * glm::vec3( cos, sin, 0),
        scaleY * glm::vec3( -sin, cos,0),
        {translation.x + localTranslation.x, translation.y + localTranslation.y, zOrder}
    };*/
    return {
        glm::vec3(1.0f,0.0f,0.0f),
        glm::vec3(0.0f,1.0f,0.0f),
        glm::vec3(translation.x + localTranslation.x, translation.y + localTranslation.y, zOrder)
    };
}

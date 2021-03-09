#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class SphericalCamera
{
public:
    SphericalCamera();
    SphericalCamera(unsigned int w, unsigned int h);
    SphericalCamera(unsigned int w, unsigned int h, const glm::vec4& e, const glm::vec4& r, const glm::vec4& worldUp);
    SphericalCamera(const SphericalCamera& c);

    float fovy;
    unsigned int width, height;  // Screen dimensions
    float near_clip;  // Near clip plane distance
    float far_clip;  // Far clip plane distance

    //Computed attributes
    float aspect;

    glm::vec4 eye,      //The position of the camera in world space
        ref,      //The point in world space towards which the camera is pointing
        look,     //The normalized vector from eye to ref. Is also known as the camera's "forward" vector.
        up,       //The normalized vector pointing upwards IN CAMERA SPACE. This vector is perpendicular to LOOK and RIGHT.
        right,    //The normalized vector pointing rightwards IN CAMERA SPACE. It is perpendicular to UP and LOOK.
        world_up, //The normalized vector pointing upwards IN WORLD SPACE. This is primarily used for computing the camera's initial UP vector.
        V,        //Represents the vertical component of the plane of the viewing frustum that passes through the camera's reference point. Used in Camera::Raycast.
        H;        //Represents the horizontal component of the plane of the viewing frustum that passes through the camera's reference point. Used in Camera::Raycast.

    float phi, theta, zoom;
    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 viewProj;
    glm::mat4 getViewProj();
    glm::mat4 getView();
    glm::mat4 getProj();

    void RecomputeAttributes();

    void RotateAboutUp(float deg);
    void RotateAboutRight(float deg);

    void TranslateAlongLook(float amt);
    void TranslateAlongRight(float amt);
    void TranslateAlongUp(float amt);

    void RotateXAboutPoint(float phi);
    void RotateYAboutPoint(float theta);
    void ZoomToPoint(float zoom);
private:
    glm::mat4 sphericalTransform;
};
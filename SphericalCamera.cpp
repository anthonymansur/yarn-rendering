#include "SphericalCamera.h"
#include <iostream>

SphericalCamera::SphericalCamera() :
    SphericalCamera(400, 400)
{
    look = glm::vec4(0, 0, -1, 0);
    up = glm::vec4(0, 1, 0, 0);
    right = glm::vec4(1, 0, 0, 0);
}

SphericalCamera::SphericalCamera(unsigned int w, unsigned int h) :
    SphericalCamera(w, h, glm::vec4(0, 0, 2, 1), glm::vec4(0, 0, 0, 1), glm::vec4(0, 1, 0, 0))
{}

SphericalCamera::SphericalCamera(unsigned int w, unsigned int h, const glm::vec4& e, const glm::vec4& r, const glm::vec4& worldUp) :
    fovy(45),
    width(w),
    height(h),
    near_clip(0.1f),
    far_clip(1000),
    eye(e),
    ref(r),
    world_up(worldUp),
    phi(0), theta(0), zoom(0)
{
    RecomputeAttributes();
}

SphericalCamera::SphericalCamera(const SphericalCamera& c) :
    fovy(c.fovy),
    width(c.width),
    height(c.height),
    near_clip(c.near_clip),
    far_clip(c.far_clip),
    aspect(c.aspect),
    eye(c.eye),
    ref(c.ref),
    look(c.look),
    up(c.up),
    right(c.right),
    world_up(c.world_up),
    V(c.V),
    H(c.H),
    phi(0), theta(0), zoom(0)
{
    RecomputeAttributes();
}

void SphericalCamera::RecomputeAttributes()
{
    // Spherical SphericalCamera model
    glm::mat4 tz = glm::translate(glm::mat4(1.f), glm::vec3(0, 0, this->zoom));
    glm::mat4 rx = glm::rotate(glm::mat4(1.f), phi, glm::vec3(1, 0, 0));
    glm::mat4 ry = glm::rotate(glm::mat4(1.f), theta, glm::vec3(0, 1, 0));
    this->sphericalTransform = ry * rx * tz;

    this->eye = sphericalTransform * glm::vec4(0, 1, 10, 1);
    // No need to transform ref -- it stays constant. this->ref = sphericalTransform * glm::vec4(0,0,0,1);
    this->up = sphericalTransform * glm::vec4(0, 1, 0, 0);
    this->look = sphericalTransform * glm::vec4(0, -1, -10, 0);
    this->right = sphericalTransform * glm::vec4(1, 0, 0, 0);

    float tan_fovy = tan(glm::radians(fovy / 2));
    float len = glm::length(ref - eye);
    aspect = width / height;
    V = (up)*len * tan_fovy;
    H = (right)*len * aspect * tan_fovy;

    view = glm::lookAt(glm::vec3(eye), glm::vec3(ref), glm::vec3(up));
    proj = glm::perspective(glm::radians(fovy), width / (float)height, near_clip, far_clip);

    viewProj = proj * view;
}

glm::mat4 SphericalCamera::getViewProj()
{
    return viewProj;
}

glm::mat4 SphericalCamera::getView()
{
    return view;
}

glm::mat4 SphericalCamera::getProj()
{
    return proj;
}

void SphericalCamera::RotateAboutUp(float deg)
{
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(deg), glm::vec3(up));
    ref = ref - eye;
    ref = glm::vec4(rotation * ref);
    ref = ref + eye;
    RecomputeAttributes();
}

void SphericalCamera::RotateAboutRight(float deg)
{
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(deg), glm::vec3(right));
    ref = ref - eye;
    ref = rotation * ref;
    ref = ref + eye;
    RecomputeAttributes();
}

void SphericalCamera::TranslateAlongLook(float amt)
{
    glm::vec4 translation = look * amt;
    eye += translation;
    ref += translation;
}

void SphericalCamera::TranslateAlongRight(float amt)
{
    glm::vec4 translation = right * amt;
    eye += translation;
    ref += translation;
}
void SphericalCamera::TranslateAlongUp(float amt)
{
    glm::vec4 translation = up * amt;
    eye += translation;
    ref += translation;
}

void SphericalCamera::RotateXAboutPoint(float phi)
{
    this->phi += phi;
    RecomputeAttributes();
}

void SphericalCamera::RotateYAboutPoint(float theta)
{
    this->theta += theta;
    RecomputeAttributes();
}

void SphericalCamera::ZoomToPoint(float zoom)
{
    this->zoom += zoom;
    RecomputeAttributes();
}
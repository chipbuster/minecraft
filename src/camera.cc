#include "camera.h"
#include <glm/gtx/rotate_vector.hpp>

#include <iostream>
using std::cout;
using std::endl;

std::ostream& operator<<(std::ostream& os, const glm::vec4 x);
std::ostream& operator<<(std::ostream& os, const glm::vec3 x);

namespace {
float pan_speed = 0.1f;
float roll_speed = 0.1f;
float rotation_speed = 0.05f;
float zoom_speed = 0.1f;
}; // namespace

Camera::Camera()
{
    // Relocated positions
    this->eye_ = glm::vec3(0.0f,10.0f,10.0f);
    this->up_  = glm::vec3(0.0f,1.0f,0.0f);
//    this->look_ = glm::normalize(glm::vec3(0.0f,-1.0f,-1.0f));
    this->look_ = glm::normalize(glm::vec3(0.0f,-1.0f,-1.0f));

    update_internal_data();
}

// FIXME: Calculate the view matrix
glm::mat4 Camera::get_view_matrix() const
{
    return world_to_cam_;
}

void Camera::lm_rotate_cam(double screendX, double screendY)
{
    glm::vec2 amt = glm::normalize(glm::dvec2(screendX, screendY));
    glm::vec3 camAxis = amt[0] * right_ - amt[1] * up_;
    glm::vec3 rotAxis = glm::cross(-camAxis, this->look_);

    glm::mat4 rotation = glm::rotate(rotation_speed, rotAxis);
    this->look_ = hom_to_cart(rotation * cart_to_hom(this->look_));
    this->up_ = hom_to_cart(rotation * cart_to_hom(this->up_));

    update_internal_data();
}

void Camera::rm_zoom_cam(double screendY)
{
    if (screendY < 0) { // Zoom in
        this->eye_ += this->look_ * zoom_speed;
        camera_distance_ -= zoom_speed;
    } else {
        this->eye_ -= this->look_ * zoom_speed;
        camera_distance_ -= zoom_speed;
    }
    update_internal_data();
}

void Camera::mm_trans_cam(double screendX, double screendY)
{
    glm::vec2 amt = glm::normalize(glm::dvec2(screendX, screendY));
    cout << amt[0] << "," << amt[1] << endl;
    glm::vec3 camDir = amt[0] * right_ - amt[1] * up_;
    cout << camDir << endl;

    this->eye_ += camDir * pan_speed;
    update_internal_data();
}

void Camera::ws_walk_cam(int direction)
{
    if (direction > 0) {
        eye_ += zoom_speed * look_;
    } else {
        eye_ -= zoom_speed * look_;
    }
    update_internal_data();
}

void Camera::ad_strafe_cam(int direction)
{
    if (direction > 0) {
        eye_ += zoom_speed * right_;
    } else {
        eye_ -= zoom_speed * right_;
    }
    update_internal_data();
}

void Camera::lr_roll_cam(int direction)
{
    if (direction > 0) {
        this->up_ = hom_to_cart(glm::rotate(-roll_speed, this->look_) *
                                cart_to_hom(this->up_));
    } else {
        this->up_ = hom_to_cart(glm::rotate(roll_speed, this->look_) *
                                cart_to_hom(this->up_));

    }
    update_internal_data();
}

void Camera::ud_move_cam(int direction){
    if (direction > 0) {
        this->eye_ += pan_speed * this->up_;
    } else {
        this->eye_ -= pan_speed * this->up_;
    }
    update_internal_data();
}


/* Assuming that look_, up_, and eye_ are the correct pieces of data, update
   redundant data forms (e.g. cam_to_world_, right_) to be consistent */
void Camera::update_internal_data()
{
    // Set rotations
    glm::vec3 camZ = -look_; // ???????????????????????????????????????????????
    glm::vec3 camY = up_;
    glm::vec3 camX = glm::cross(camY, camZ); // cross is RHS, so take opp. order

    glm::mat3 rot(camX, camY, camZ);

    glm::mat4 cam_to_world = rot_trans_to_hom(rot, eye_);

    this->right_ = camX;
    this->cam_to_world_ = cam_to_world;
    this->world_to_cam_ = glm::inverse(cam_to_world_);
    this->center_ = eye_ + camera_distance_ * look_;
}

glm::mat3 hom_to_cart(glm::mat4 inp)
{
    glm::mat3 r;
    r[0][0] = inp[0][0];
    r[0][1] = inp[0][1];
    r[0][2] = inp[0][2];
    r[1][0] = inp[1][0];
    r[1][1] = inp[1][1];
    r[1][2] = inp[1][2];
    r[2][0] = inp[2][0];
    r[2][1] = inp[2][1];
    r[2][2] = inp[2][2];
    return r;
}
glm::vec3 hom_to_cart(glm::vec4 inp)
{
    glm::vec3 r;
    r[0] = inp[0];
    r[1] = inp[1];
    r[2] = inp[2];
    return r;
}

glm::mat4 cart_to_hom(glm::mat3 inp)
{
    glm::mat4 r;
    r[0][0] = inp[0][0];
    r[0][1] = inp[0][1];
    r[0][2] = inp[0][2];
    r[1][0] = inp[1][0];
    r[1][1] = inp[1][1];
    r[1][2] = inp[1][2];
    r[2][0] = inp[2][0];
    r[2][1] = inp[2][1];
    r[2][2] = inp[2][2];
    return r;
}
glm::vec4 cart_to_hom(glm::vec3 inp)
{
    glm::vec4 r;
    r[0] = inp[0];
    r[1] = inp[1];
    r[2] = inp[2];
    r[3] = 1.0f;
    return r;
}

glm::mat4 rot_trans_to_hom(glm::mat3 rot, glm::vec3 t)
{
    glm::mat4 r = cart_to_hom(rot);
    r[3][0] = t[0];
    r[3][1] = t[1];
    r[3][2] = t[2];
    return r;
}
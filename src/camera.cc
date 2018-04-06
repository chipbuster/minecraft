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
float rotation_speed = 0.03f;
float zoom_speed = 0.1f;
}; // namespace

Camera::Camera()
{
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

    glm::vec3 nextLook = glm::normalize(glm::vec3(rotation * glm::vec4(this->look_,1.0f)));
    if(glm::dot(nextLook, this->true_up_) > 0.99){
        this->look_ = this->look_;
    }else{
        this->look_ = nextLook;
        // Rule: look, up_, and true_up_ should be collinear
        this->up_ = glm::normalize(
            this->true_up_ - glm::dot(this->look_, this->true_up_) * this->look_
        );
    }   

    assert(abs(glm::dot(this->up_, this->look_)) < 0.01);

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

/* Assuming that look_, up_, and eye_ are the correct pieces of data, update
   redundant data forms (e.g. cam_to_world_, right_) to be consistent */
void Camera::update_internal_data()
{
    // Set rotations
    glm::vec3 camZ = -look_; // ???????????????????????????????????????????????
    glm::vec3 camY = up_;
    glm::vec3 camX = glm::cross(camY, camZ); // cross is RHS, so take opp. order

    glm::mat3 rot(camX, camY, camZ);

    glm::mat4 cam_to_world = glm::mat4(rot);
    cam_to_world[3] = glm::vec4(eye_,1.0f);

    this->right_ = camX;
    this->cam_to_world_ = cam_to_world;
    this->world_to_cam_ = glm::inverse(cam_to_world_);
    this->center_ = eye_ + camera_distance_ * look_;
}

void Camera::update_physics(double timeDiff){
    // Update camera position with velocity + explicit Euler
    this->eye_ += (float)timeDiff * this->velocity_;

    // Update camera velocity from gravity
    this->velocity_ += glm::vec3(0.0f,-gravity, 0.0f);

    // Update camera velocity from friction
    // TODO
}
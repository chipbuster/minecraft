#include "camera.h"
#include <algorithm>
#include <cmath>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>

#include <iostream>
using std::cout;
using std::endl;
using std::max;
using std::min;

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

    // The input from screen space is nonsensical. Ignore it.
    if (std::isnan(glm::length(amt))) {
        return;
    }

    glm::vec3 camAxis = amt[0] * right_ - amt[1] * up_;
    glm::vec3 rotAxis = glm::cross(-camAxis, this->look_);

    glm::mat4 rotation = glm::rotate(rotation_speed, rotAxis);

    glm::vec3 nextLook =
            glm::normalize(glm::vec3(rotation * glm::vec4(this->look_, 1.0f)));
    if (glm::dot(nextLook, this->true_up_) > 0.99) {
        this->look_ = this->look_;
    } else {
        this->look_ = nextLook;
        // Rule: look, up_, and true_up_ should be collinear
        this->up_ = glm::normalize(this->true_up_ -
                                   glm::dot(this->look_, this->true_up_) *
                                           this->look_);
    }

    assert(fabs(glm::dot(this->up_, this->look_)) < 0.01);

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

    this->eye_ += camDir * pan_speed;
    update_internal_data();
}

void Camera::lr_roll_cam(int direction)
{
    if (physics_mode)
        return;
    if (direction > 0) {
        this->up_ = glm::vec3(glm::rotate(-roll_speed, this->look_) *
                              glm::vec4(this->up_, 0.0f));
    } else {
        this->up_ = glm::vec3(glm::rotate(roll_speed, this->look_) *
                              glm::vec4(this->up_, 0.0f));
    }
    update_internal_data();
}

void Camera::ud_move_cam(int direction)
{
    if (physics_mode)
        return;
    if (direction > 0) {
        this->eye_ += pan_speed * this->up_;
    } else {
        this->eye_ -= pan_speed * this->up_;
    }
    update_internal_data();
}

void Camera::ws_walk_cam(int direction)
{
    if (physics_mode) {
        if (direction > 0) {
            velocity_ += 2.0f * look_;
        } else {
            velocity_ -= 2.0f * look_;
        }
    } else {
        if (direction > 0) {
            eye_ += zoom_speed * look_;
        } else {
            eye_ -= zoom_speed * look_;
        }
    }
    update_internal_data();
}

void Camera::ad_strafe_cam(int direction)
{
    if (physics_mode) {
        if (direction > 0) {
            velocity_ += 2.0f * right_;
        } else {
            velocity_ -= 2.0f * right_;
        }
    } else {
        if (direction > 0) {
            eye_ += zoom_speed * right_;
        } else {
            eye_ -= zoom_speed * right_;
        }
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
    cam_to_world[3] = glm::vec4(eye_, 1.0f);

    this->right_ = camX;
    this->cam_to_world_ = cam_to_world;
    this->world_to_cam_ = glm::inverse(cam_to_world_);
    this->center_ = eye_ + camera_distance_ * look_;
}

enum CollisionType {
    NONE = 0,
    CEIL = 1,
    FLOOR = 2,
    MINX = 4,
    MAXX = 8,
    MINZ = 16,
    MAXZ = 32,
    WALL = 64
};
inline CollisionType operator|(CollisionType a, CollisionType b)
{
    return static_cast<CollisionType>(static_cast<int>(a) |
                                      static_cast<int>(b));
}
inline CollisionType operator&(CollisionType a, CollisionType b)
{
    return static_cast<CollisionType>(static_cast<int>(a) &
                                      static_cast<int>(b));
}

constexpr float eps = 0.10;
constexpr float camH = 1.75;
constexpr float camR = 0.49;
CollisionType collide(const glm::vec3& location, const glm::vec3& Cmin)
{
    float minX = Cmin.x;
    float maxX = minX + 1.0;
    float minZ = Cmin.z;
    float maxZ = minZ + 1.0;
    float minY = Cmin.y;
    float maxY = Cmin.y + 1.0;

    float camBot = location.y - camH;
    float camTop = location.y;
    float camMinX = location.x - camR;
    float camMaxX = location.x + camR;
    float camMinZ = location.z - camR;
    float camMaxZ = location.z + camR;

    CollisionType ct = NONE;

    bool MaxYCross =
            camBot < maxY + eps && camTop > maxY; // Camera crosses maxY
    bool MinYCross =
            camTop > minY - eps && camBot < minY; // Camera crosses minY
    bool MaxXCross = camMinX < maxX && camMaxX > maxX;
    bool MaxZCross = camMinZ < maxZ && camMaxZ > maxZ;
    bool MinXCross = camMaxX > minX && camMinX < minX;
    bool MinZCross = camMaxZ > minZ && camMinZ < minZ;

    // Cylinder is inbounds if it crosses an edge or is entirely within the cell
    bool inBoundsY = camBot < maxY && camBot > minY ||
                     camTop < maxY && camTop > minY ||
                     camBot > minY && camTop < maxY;
    bool inBoundsX = camMinX < maxX && camMinX > minX ||
                     camMaxX < maxX && camMaxX > minX ||
                     camMaxX < maxX && camMinX > minX;
    bool inBoundsZ = camMinZ < maxZ && camMinZ > minZ ||
                     camMaxZ < maxZ && camMaxZ > minZ ||
                     camMaxZ < maxZ && camMinZ > minZ;

    if (MaxYCross && inBoundsX && inBoundsZ) {
        /*
        cout << "Collision of " << glm::to_string(location) << " with "
             << glm::to_string(Cmin) << " of type FLOOR" << std::endl; */
        ct = ct | FLOOR;
    }
    if (MinYCross && inBoundsX && inBoundsZ) {
        /*
        cout << "Collision of " << glm::to_string(location) << " with "
             << glm::to_string(Cmin) << " of type CEIL" << std::endl; */
        ct = ct | CEIL;
    }
    if (MaxXCross && inBoundsY && inBoundsZ) {
        /*
        cout << "Collision of " << glm::to_string(location) << " with "
             << glm::to_string(Cmin) << " of type MINX" << std::endl; */
        ct = ct | MINX;
    }
    if (MinXCross && inBoundsY && inBoundsZ) {
        /*
        cout << "Collision of " << glm::to_string(location) << " with "
             << glm::to_string(Cmin) << " of type MAXX" << std::endl; */
        ct = ct | MAXX;
    }
    if (MaxZCross && inBoundsY && inBoundsX) {
        /*
        cout << "Collision of " << glm::to_string(location) << " with "
             << glm::to_string(Cmin) << " of type MINZ" << std::endl; */
        ct = ct | MINZ;
    }
    if (MinZCross && inBoundsY && inBoundsX) {
        /*
        cout << "Collision of " << glm::to_string(location) << " with "
             << glm::to_string(Cmin) << " of type MAXZ" << std::endl;*/

        ct = ct | MAXZ;
    }
    return ct;
}

void Camera::update_physics(double timestep, const Chunk& C,
                            const std::vector<glm::vec3>& cubes)
{
    if (!physics_mode)
        return;

    constexpr float camH = 1.75;
    constexpr float camR = 0.5;
    // Update camera velocity from gravity
    this->velocity_ += glm::vec3(0.0f, -gravity, 0.0f);

    // Update camera velocity from friction
    this->velocity_ *= pow(0.1, timestep);
    if (glm::length(this->velocity_) < 0.05)
        this->velocity_ = glm::vec3(0.0);

    // Update camera velocity from collisions
    std::vector<glm::vec3>
            coarseCollisions; // Cube indices that we might collide with

    // Coarse detection
    constexpr float coarseRadius = 1.5 + sqrt(camR * camR + camH * camH);
    for (const auto& c : cubes) {
        if (glm::length(c - this->eye_) < coarseRadius) {
            coarseCollisions.push_back(c);
        }
    }

    // Fine detection
    CollisionType allColl = NONE;
    for (const auto& c : coarseCollisions) {
        CollisionType coll = collide(this->eye_, c);

        if (coll & FLOOR) {
            // If we are falling, make sure we don't fall too far
            if (this->velocity_.y < -10) {
                this->eye_.y = c.y + camH + 1.0 + eps / 2;
            }
            this->velocity_.y = max(0.0f, this->velocity_.y);
            allColl = allColl | FLOOR;
        }
        if (coll & CEIL) {
            this->velocity_.y = min(0.0f, this->velocity_.y);
            allColl = allColl | CEIL;
        }
        if (coll & MINX) {
            this->velocity_.x = max(0.0f, this->velocity_.x);
            allColl = allColl | MINX;
        }
        if (coll & MAXX) {
            this->velocity_.x = min(0.0f, this->velocity_.x);
            allColl = allColl | MAXX;
        }
        if (coll & MINZ) {
            this->velocity_.z = max(0.0f, this->velocity_.z);
            allColl = allColl | FLOOR;
        }
        if (coll & MAXZ) {
            this->velocity_.z = min(0.0f, this->velocity_.z);
            allColl = allColl | FLOOR;
        }
    }

    // Update camera position with velocity + explicit Euler
    this->eye_ += (float)timestep * this->velocity_;

    // Calc!
    update_internal_data();
}

glm::vec3 Camera::getEye() const
{
    return eye_;
}

void Camera::jump()
{
    this->velocity_ += glm::vec3(0.0f, 20.0f, 0.0f);
}
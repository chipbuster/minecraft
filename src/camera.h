#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include "Terrain.h"


class Camera {
public:
    glm::mat4 get_view_matrix() const;
    void lm_rotate_cam(double screenX, double screenY);
    void rm_zoom_cam(double screendY);
    void mm_trans_cam(double screenX, double screenY);
    void ws_walk_cam(int direction,
                     const std::vector<glm::vec3>& cubes);
    void ad_strafe_cam(int direction,
                       const std::vector<glm::vec3>& cubes);
    void lr_roll_cam(int direction,
                     const std::vector<glm::vec3>& cubes);
    void ud_move_cam(int direction,
                     const std::vector<glm::vec3>& cubes);
    void update_physics(double timestep, const Chunk& C, const std::vector<glm::vec3>& cubes);
    bool physics_mode = true;
    void jump();

    glm::vec3 getEye() const;

    Camera();
    ~Camera() {};
private:
    float camera_distance_ = 3.0;
    glm::vec3 look_ = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 up_ = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 true_up_ = glm::vec3(0.0f,1.0f,0.0f);
    glm::vec3 eye_ = glm::vec3(0.0f, 4.0f, camera_distance_);
    glm::vec3 right_;
    glm::vec3 center_;
    glm::mat4 cam_to_world_;
    glm::mat4 world_to_cam_;

    glm::vec3 velocity_ = glm::vec3(0.0f,0.0f,0.0f);
    float gravity = 0.98;
    float friction = 1.0;

    void update_internal_data();
    // Note: you may need additional member variables
};

#endif

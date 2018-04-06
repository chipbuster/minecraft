#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

class Camera {
public:
    glm::mat4 get_view_matrix() const;
    void lm_rotate_cam(double screenX, double screenY);
    void rm_zoom_cam(double screendY);
    void mm_trans_cam(double screenX, double screenY);
    void ws_walk_cam(int direction);
    void ad_strafe_cam(int direction);
    void lr_roll_cam(int direction);
    void ud_move_cam(int direction);

    Camera();
    ~Camera() {};
private:
    float camera_distance_ = 3.0;
    glm::vec3 look_;
    glm::vec3 up_;
    glm::vec3 eye_;
    glm::vec3 right_;
    glm::vec3 center_;
    glm::mat4 cam_to_world_;
    glm::mat4 world_to_cam_;

    void update_internal_data();
    // Note: you may need additional member variables
};

glm::vec4 cart_to_hom(glm::vec3 inp);
glm::mat4 cart_to_hom(glm::mat3 inp);
glm::vec3 hom_to_cart(glm::vec4 inp);
glm::mat3 hom_to_cart(glm::mat4 inp);
glm::mat4 rot_trans_to_hom(glm::mat3, glm::vec3);

#endif

#include "camera.h"
#include <math.h>

void rotate_camera(Camera *camera, float up_down, float left_right) {
    float max_angle = 1.24;
    camera->up_down_angle = fminf(fmaxf(camera->up_down_angle + up_down, -max_angle), max_angle);

    camera->left_right_angle += left_right;
    while (camera->left_right_angle < -MPI)
        camera->left_right_angle += 2 * MPI;
    while (camera->left_right_angle > MPI)
        camera->left_right_angle -= 2 * MPI;

    float rotate_up_down[MAT4_SIZE];
    mat4_identity(rotate_up_down);
    mat4_rotation_x(rotate_up_down, camera->up_down_angle);

    float rotate_left_right[MAT4_SIZE];
    mat4_identity(rotate_left_right);
    mat4_rotation_y(rotate_left_right, camera->left_right_angle);

    mat4_identity(camera->rotation);
    // up down needs to be first
    mat4_multiply(camera->rotation, rotate_left_right, rotate_up_down);

    vec4_multiply_mat4(camera->up, vec4(camera->up, 0, 1, 0, 0), camera->rotation);
    vec4_multiply_mat4(camera->forward, vec4(camera->forward, 0, 0, -1, 0), camera->rotation);
}

void update_camera(Camera *camera) {
    mat4_perspective(camera->perspective, camera->fov_y, camera->aspect, camera->near, camera->far);

    // update up, forward and rotation
    rotate_camera(camera, 0, 0);

    float target[VEC3_SIZE];
    vec3_add(target, camera->position, camera->forward);

    mat4_look_at(camera->view, camera->position, target, camera->up);
}

void move_camera(Camera *camera, float *direction, float speed) {
    float delta[VEC3_SIZE];
    vec3_multiply_f(delta, direction, speed);
    vec3_add(camera->position, camera->position, delta);
}

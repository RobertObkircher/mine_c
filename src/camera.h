#ifndef MINE_C_CAMERA_H
#define MINE_C_CAMERA_H

#include <mathc.h>

typedef struct {
    float left_right_angle; // -PI .. PI
    float up_down_angle; // -PI .. PI

    float position[VEC3_SIZE];

    float aspect;
    float fov_y;
    float near;
    float far;

    // computed by update_camera
    float view[MAT4_SIZE];
    float perspective[MAT4_SIZE];
    float up[VEC4_SIZE];
    float forward[VEC4_SIZE];
    float rotation[MAT4_SIZE];
} Camera;

static Camera default_camera = {
        .aspect = 1,
        .fov_y = MPI / 2.0f,
        .near = 0.1f,
        .far = 100.0f
};

void rotate_camera(Camera *camera, float up_down, float left_right);

void update_camera(Camera *camera);

void move_camera(Camera *camera, float direction[VEC3_SIZE], float speed);

#endif //MINE_C_CAMERA_H


#ifndef MATRIX_H
#define MATRIX_H


class Matrix
{
public:
    void normalize(float *x, float *y, float *z);
    void mat_identity(float *matrix);
    void mat_translate(float *matrix, float dx, float dy, float dz);
    void mat_rotate(float *matrix, float x, float y, float z, float angle);
    void mat_vec_multiply(float *vector, float *a, float *b);
    void mat_multiply(float *matrix, float *a, float *b);
    void mat_apply(float *data, float *matrix, int count, int offset, int stride);
    void mat_frustum(
        float *matrix, float left, float right, float bottom,
        float top, float znear, float zfar);
    void mat_perspective(
        float *matrix, float fov, float aspect,
        float near, float far);
    void mat_ortho(
        float *matrix,
        float left, float right, float bottom, float top, float near, float far);

    void set_matrix_2d(float *matrix, int width, int height);
    void set_matrix_3d(
        float *matrix, int width, int height,
        float x, float y, float z, float rx, float ry, float fov, int ortho);
    void set_matrix_item(float *matrix, int width, int height);
};

#endif // MATRIX_H

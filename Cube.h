#ifndef CUBE_H
#define CUBE_H

#include "Matrix.h"

class Cube
{
public:
    void make_cube_faces(
        float *data,
        int left, int right, int top, int bottom, int front, int back,
        int wleft, int wright, int wtop, int wbottom, int wfront, int wback,
        float x, float y, float z, float n);

    void make_cube(
        float *data,
        int left, int right, int top, int bottom, int front, int back,
        float x, float y, float z, float n, int w);

    void make_plant(
        float *data,
        float px, float py, float pz, float n, int w, float rotation);

    void make_player(
        float *data,
        float x, float y, float z, float rx, float ry);

    void make_cube_wireframe(
        float *data, float x, float y, float z, float n);

    void make_character(
        float *data,
        float x, float y, float n, float m, char c);

private:
    Matrix mat;
};

#endif // CUBE_H

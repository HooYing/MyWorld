#version 330 core
layout (location = 0 ) in vec3 position;
layout (location = 1 ) in vec2 uv;

uniform mat4 matrix;

varying vec2 fragment_uv;

void main()
{
    gl_Position =  matrix * vec4(position, 1.0f);
    fragment_uv = uv;
}

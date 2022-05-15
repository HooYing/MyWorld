#version 330 core
layout (location = 0 ) in vec3 position;
layout (location = 1 ) in vec3 normal;
layout (location = 2 ) in vec2 uv;

uniform mat4 matrix;
uniform vec3 camera;
uniform float fog_distance;

varying vec2 fragment_uv;
varying float camera_distance;
varying float fog_factor;
varying float diffuse;

const vec3 light_direction = normalize(vec3(-1.0,1.0,-1.0));

void main()
{
    gl_Position =  matrix * vec4(position, 1.0f);
    fragment_uv = uv;

    camera_distance = distance(camera,position);
    fog_factor = pow(clamp(camera_distance/fog_distance,0.0,1.0),4.0);
    diffuse = max(0.0,dot(normal,light_direction));
}

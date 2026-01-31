#version 330

// Input
// TODO(student): Get vertex attributes from each location
layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_color;
layout(location = 2) in vec3 v_coordinates;
layout(location = 3) in vec3 v_normal;


uniform float in_time;

// Uniform properties
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

// Output
// TODO(student): Output values to fragment shader
out vec3 attribute_color;
out vec3 attribute_normal;
out vec3 attribute_coordinates;

void main()
{
    // TODO(student): Send output to fragment shader
    attribute_color = v_color;
    attribute_normal = v_normal;
    attribute_coordinates = v_coordinates;
    // TODO(student): Compute gl_Position
    vec3 animatedPos = v_position + vec3(sin(in_time), cos(in_time * 0.5), 0.0);
    gl_Position = Projection * View * Model * vec4(animatedPos, 1.0);
}

#version 330

// Input
// TODO(student): Get values from vertex shader
in vec3 attribute_color;
in vec3 attribute_normal;

uniform float in_time;
// Output
layout(location = 0) out vec4 out_color;



void main()
{
    // TODO(student): Write pixel out color
    vec3 baseColor = normalize(attribute_normal) * 0.3 + 0.3;
    float r = baseColor.r * abs(sin(in_time));
    float g = baseColor.g * abs(sin(in_time) * 0.3);
    float b = baseColor.b * abs(sin(in_time) * 0.3 + 0.3);

    out_color = vec4(color, 1.0);
}

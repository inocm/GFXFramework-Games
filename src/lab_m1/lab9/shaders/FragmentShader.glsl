#version 330

// Input from vertex shader
in vec2 texcoord;

// Uniform textures
uniform sampler2D texture_1;
uniform sampler2D texture_2;
uniform bool apply;
// Output
layout(location = 0) out vec4 out_color;

void main()
{
    
    vec4 finalColor;
    if (apply) {
    vec4 color1 = texture(texture_1, texcoord);
    vec4 color2 = texture(texture_2, texcoord);
    finalColor = mix(color1, color2, 0.5);
    } else {
    vec4 color1 = texture(texture_1, texcoord);
    finalColor = color1;
    }
   

    if (finalColor.a < 0.5f)
        discard;

    out_color = finalColor;
}

#version 330

// Input
in vec3 world_position;
in vec3 world_normal;

// Uniforms for light properties
uniform vec3 light_direction; // direction the spotlight points toward (normalized expected)
uniform vec3 light_position;
uniform vec3 light_position_2; // second light (point light)
uniform vec3 eye_position;

uniform float material_kd;
uniform float material_ks;
uniform int material_shininess;

// Spotlight control uniforms
uniform int is_spotlight;     // 0 = off, 1 = on
uniform float spot_cutoff;    // cutoff angle in degrees
uniform float spot_exponent;  // exponent for smooth falloff

uniform vec3 object_color;

// Output
layout(location = 0) out vec4 out_color;


vec3 PhongForLight(vec3 Ldir, vec3 position, vec3 N, vec3 V, float spotAtt, vec3 baseColor)
{
    // Ldir must be normalized and point from fragment toward light
    float NdotL = max(dot(N, Ldir), 0.0);

    // Diffuse
    vec3 diffuse = material_kd * NdotL * baseColor;

    // Specular (Phong reflection)
    vec3 specular = vec3(0.0);
    if (NdotL > 0.0) {
        vec3 R = reflect(-Ldir, N);
        float specFactor = max(dot(V, R), 0.0);
        specular = material_ks * pow(specFactor, float(material_shininess)) * vec3(1.0);
    }

    return spotAtt * (diffuse + specular);
}

void main()
{
    vec3 N = normalize(world_normal);
    vec3 V = normalize(eye_position - world_position);

    // Ambient term
    float ambient_coeff = 0.25;
    vec3 ambient = ambient_coeff * object_color;

    // First light: may be spotlight
    vec3 L1 = normalize(light_position - world_position); // from fragment to light1

    float spotAtt1 = 1.0;
    if (is_spotlight == 1) {
        // Vector from light to fragment
        vec3 L_from_light = normalize(world_position - light_position);
        // light_direction is the direction the spotlight points toward (should be normalized)
        float cosAngle = dot(normalize(light_direction), L_from_light);
        float cosCutoff = cos(radians(spot_cutoff));

        // Use linear interpolation between cosCutoff and 1.0, then apply exponent
        float linearAtt = (cosAngle - cosCutoff) / (1.0 - cosCutoff);
        linearAtt = clamp(linearAtt, 0.0, 1.0);

        // If outside cone, linearAtt will be <= 0 -> after clamp it becomes 0
        spotAtt1 = pow(linearAtt, spot_exponent);
    }

    vec3 color = ambient;

    color += PhongForLight(L1, world_position, N, V, spotAtt1, object_color);

    // Second light: simple point light (no spotlight)
    vec3 L2 = normalize(light_position_2 - world_position);
    color += PhongForLight(L2, world_position, N, V, 1.0, object_color);

    // Final clamp and output
    color = clamp(color, vec3(0.0), vec3(1.0));
    out_color = vec4(color, 1.0);
}
#version 330 core

// Interpolated values from the vertex shaders
in vec3 fragment_position;
in vec3 observer_direction_cameraspace;
in vec3 fragment_normal;
in vec3 fragment_direction;

// Ouput data
out vec3 color;

// Color
//uniform vec3 cube_color;
uniform vec3 light_color;
uniform vec3 light_direction_cameraspace;
uniform mat3 normal_mv_matrix;


vec3 colormap(vec3 direction) // TODO: build a switch for different maps
{
    float brightness = 0.8f;
    float z1 = brightness * (1.0f + direction.z)/2.0f;
    float z2 = brightness * (1.0f - direction.z)/2.0f;
    return vec3(z1, 0.0f, z2);
}

vec3 calculate_color(vec3 normal) {
    float diffuse_factor, specular_factor;
    vec3 light_direction_cameraspace_normalized, observer_direction_cameraspace_normalized;
    vec3 halfway_vector;

    vec3 direction_color = colormap(fragment_direction);

    light_direction_cameraspace_normalized = normalize(light_direction_cameraspace);
    observer_direction_cameraspace_normalized = normalize(observer_direction_cameraspace);

    halfway_vector = normalize(light_direction_cameraspace_normalized + observer_direction_cameraspace_normalized);

    diffuse_factor = max(0, dot(light_direction_cameraspace_normalized, normal));
    specular_factor = pow(max(0, dot(halfway_vector, normal)), 100);
    return direction_color * light_color * (0.3 + diffuse_factor) + 0.5 * light_color * specular_factor;
}

void main()
{
    vec3 normal_cameraspace;
    // Calculate transformed normals
    normal_cameraspace = normalize(normal_mv_matrix * fragment_normal);
    // Calculate Color
    color = calculate_color(normal_cameraspace);
}

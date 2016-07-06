#version 330 core

// Input vertex data, different for all executions of this shader.
in vec3 vertex_position_modelspace;
in vec3 vertex_normal;
in vec3 instance_offset;
in vec3 instance_direction;

// Output data ; will be interpolated for each fragment.
out vec3 observer_direction_cameraspace;
out vec3 fragment_normal;
out vec3 fragment_direction;

// Values that stay constant for the whole mesh.
uniform mat4 mv_matrix;
uniform mat4 mvp_matrix;

mat3 rot_z_to(vec3 direction)
{
    // +z direction
    if ( abs(direction.z - 1.0f) < 1.e-7f )
    {
        return mat3(1.0);
    }
    // -z direction
    else if ( abs(direction.z + 1.0f) < 1.e-7f )
    {
        return mat3(-1.0);
    }
    else
    {
        vec3 e1 = normalize(cross(direction, vec3(0,0,1)));
        vec3 e2 = normalize(cross(direction, e1));
        return mat3(e1, e2, direction); 
    }
}

void main()
{
    mat3 rotation = rot_z_to(instance_direction);
    vec4 vertex_position_cameraspace;

    // Calculate observer direction vector; needed for shading in fragment shader
    vertex_position_cameraspace = mv_matrix * vec4(rotation*vertex_position_modelspace + instance_offset, 1.0);
    observer_direction_cameraspace = vec3(0, 0, 0) - vertex_position_cameraspace.xyz;

    // Output position of the vertex, in clip space : MVP * position
    gl_Position =  mvp_matrix * vec4(rotation*vertex_position_modelspace + instance_offset, 1);

    // The normal of each vertex will be interpolated
    // to produce the normal of each fragment
    fragment_normal = rotation*vertex_normal;
    fragment_direction = instance_direction;
}


[VERTEX]
attribute vec3 a_vertex;
attribute vec3 a_normal;
attribute vec2 a_uv;

uniform mat4 projection_matrix;
uniform mat4 camera_matrix;
uniform mat4 object_matrix;
uniform vec3 object_scale;

varying vec2 v_uv;

void main()
{
    vec4 position = camera_matrix * object_matrix * vec4(0.0, 0.0, a_vertex.y * object_scale.y, 1.0);
    position.x += a_vertex.x * object_scale.x;
    gl_Position = projection_matrix * position;
    v_uv = a_uv.xy;
}

[FRAGMENT]
uniform sampler2D texture_map;
uniform vec4 color;

varying vec2 v_uv;

void main()
{
    gl_FragColor = texture2D(texture_map, v_uv) * color;
    if (gl_FragColor.a == 0.0)
        discard;
}

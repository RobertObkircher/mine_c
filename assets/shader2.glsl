
#ifdef COMPILING_VERTEX_SHADER

uniform mat4 MVP;

layout (location = 0) in vec3 vPos;

void main()
{
    gl_Position = MVP * vec4(vPos, 1.0);
}

#endif

#ifdef COMPILING_FRAGMENT_SHADER

out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}

#endif

varying vec3 color;

#ifdef COMPILING_VERTEX_SHADER

uniform mat4 MVP;
attribute vec3 vCol;
attribute vec3 vPos;
void main()
{
    gl_Position = MVP * vec4(vPos, 1.0);
    color = vCol;
}


#endif

#ifdef COMPILING_FRAGMENT_SHADER

void main()
{
    gl_FragColor = vec4(color, 1.0);
}

#endif

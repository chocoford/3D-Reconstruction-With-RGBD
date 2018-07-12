#version 330 core
out vec4 FragColor;

in vec3 outColor;
in vec3 normal;

void main()
{
    FragColor = vec4(normalize(normal), 1.0f);
}
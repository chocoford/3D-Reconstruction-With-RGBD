#version 330 core
out vec4 FragColor;

in vec3 outColor;
in vec3 normal;

void main()
{
    FragColor = vec4(sqrt(outColor) * normalize(vec3(pow(normal.x, 3), normal.y, sin(normal.z))), 1.0f);
}
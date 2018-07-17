#version 330 core

in vec3 outColor;
out vec4 FragColor;

void main()
{
    FragColor = vec4(outColor.x * outColor.x, outColor.y * outColor.y, outColor.z * outColor.z, 1.0); // set alle 4 vector values to 1.0
}
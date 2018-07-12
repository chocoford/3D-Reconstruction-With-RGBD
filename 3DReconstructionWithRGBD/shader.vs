#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor; // 颜色变量的属性位置值为 1
layout (location = 2) in vec3 aNormal;
out vec3 outColor; // 向片段着色器输出一个颜色


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


void main()
{
	
	gl_Position = projection * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0f);
	//outColor = vec3(-aPos.z / 20); // 将ourColor设置为我们从顶点数据那里得到的输入颜色







	outColor = normalize(aNormal);
}
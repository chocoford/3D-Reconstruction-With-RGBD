#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 nu; // normal upper
layout (location = 2) in vec3 nl; // normal left
layout (location = 3) in vec3 nd; // normal down
layout (location = 4) in vec3 nr; // normal right
layout (location = 5) in vec3 aColor; // 颜色变量的属性位置值为 1
out vec3 outColor; // 向片段着色器输出一个颜色
out vec3 normal;
out vec3 fragPos;
out vec3 worldPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec2 leftHand1;
uniform vec2 leftHand2;
uniform vec2 rightHand1;
uniform vec2 rightHand2;


void main()
{
	fragPos = vec3(model * vec4(aPos, 1.0));
	gl_Position = projection * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0f);
	outColor = aColor; // 将ourColor设置为我们从顶点数据那里得到的输入颜色
	worldPos =  vec3(model * vec4(aPos.x, aPos.y, aPos.z, 1.0f));

	vec3 upLeft = normalize(cross(nu, nl));
	vec3 downRight = normalize(cross(nd, nr));

	normal = (upLeft * downRight);

	if (distance(leftHand1, vec2(aPos)) < 0.05)  outColor = vec3(1.0, 0.0, 0.0);
	else if (distance(vec2(aPos), rightHand1) < 0.05) outColor = vec3(0.0, 0.0, 1.0);
	else if (distance(vec2(aPos), leftHand2) < 0.05) outColor = vec3(1.0, 0.0, 1.0);
	else if (distance(vec2(aPos), rightHand2) < 0.05) outColor = vec3(0.0, 1.0, 1.0);
}
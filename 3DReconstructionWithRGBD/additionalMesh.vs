#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 outColor; // 向片段着色器输出一个颜色

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 leftHand1;
uniform vec2 leftHand2;
uniform vec2 rightHand1;
uniform vec2 rightHand2;


void main() {
	gl_Position = projection * view * (model * vec4(aPos, 1.0f) + vec4(leftHand1, 0.0));
	outColor = aColor;
}
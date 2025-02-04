#version 330 core
out vec4 FragColor;

in float fHeight;

void main()
{
	float h = (fHeight + 16)/32.0f;
	FragColor = vec4(h,h,h, 1.0);
}
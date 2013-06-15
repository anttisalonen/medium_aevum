attribute vec4 aPosition;
attribute vec2 aTexcoord;

varying vec2 vTexcoord;

uniform vec2 uCamera;

void main()
{
	vTexcoord = aTexcoord;
	gl_Position = aPosition + vec4(uCamera, 0.0, 0.0);
}

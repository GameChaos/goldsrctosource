
#pragma sokol @ctype mat4 mat4
#pragma sokol @ctype vec2 v2
#pragma sokol @ctype vec3 v3
#pragma sokol @ctype vec4 v4

#pragma sokol @vs wire_vs

uniform wire_vs_params {
    mat4 mvp;
	vec3 uColour;
};

in vec3 pos;
out vec3 colour;

void main()
{
    gl_Position = mvp * vec4(pos, 1);
	gl_Position.z -= 0.01f;
	colour = uColour;
}

#pragma sokol @end

#pragma sokol @fs wire_fs

in vec3 colour;
out vec4 frag_color;

void main()
{
    frag_color = vec4(colour, 1);
}

#pragma sokol @end
#pragma sokol @program wire wire_vs wire_fs

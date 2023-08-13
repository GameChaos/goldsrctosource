
#pragma sokol @ctype mat4 hmm_mat4
#pragma sokol @ctype vec2 hmm_vec2
#pragma sokol @ctype vec3 hmm_vec3
#pragma sokol @ctype vec4 hmm_vec4

#pragma sokol @vs wire_vs

uniform wire_vs_params {
    mat4 mvp;
	vec3 uColour;
};

out vec3 colour;
in vec3 pos;
in vec3 normal;
in vec2 uv;

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

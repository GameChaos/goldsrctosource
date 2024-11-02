
#pragma sokol @ctype mat4 mat4
#pragma sokol @ctype vec2 v2
#pragma sokol @ctype vec3 v3
#pragma sokol @ctype vec4 v4

#pragma sokol @vs world_vs

uniform world_vs_params {
    mat4 mvp;
	vec3 uCamOrigin;
};

in vec3 iPos;
in vec3 iNormal;
in vec2 iUv;

out vec2 uv;
out vec3 normal;
out vec3 camOrigin;
out vec3 surfacePos;

void main()
{
    gl_Position = mvp * vec4(iPos, 1);
	uv = iUv;
	normal = iNormal;
	camOrigin = uCamOrigin;
	surfacePos = iPos;
}

#pragma sokol @end

// TODO: this could be an #include or something, cos it's identical to mesh.glsl's frag shader
#pragma sokol @fs world_fs

vec3 ToSrgb(vec3 linearRGB)
{
    bvec3 cutoff = lessThan(linearRGB, vec3(0.00313066844250063));
    vec3 higher = vec3(1.055) * pow(abs(linearRGB), vec3(1.0 / 2.4)) - vec3(0.055);
	vec3 lower = linearRGB * vec3(12.92);
	
	vec3 result = mix(higher, lower, cutoff);
	return result;
}

vec3 ToLinear(vec3 srgb)
{
	bvec3 cutoff = lessThan(srgb, vec3(0.04045));
	vec3 lower = srgb * vec3(0.0773993808049536f);
	vec3 higher = pow(abs((srgb + vec3(0.055)) * vec3(0.9478672985781991)), vec3(2.4));
	
	vec3 result = mix(higher, lower, cutoff);
	return result;
}
uniform texture2D tex;
uniform sampler smp;

in vec2 uv;
in vec3 normal;
in vec3 camOrigin;
in vec3 surfacePos;
out vec4 frag_color;

void main()
{
	float dot = abs(dot(normal, normalize(surfacePos - camOrigin)));
	vec3 colour = ToLinear(texture(sampler2D(tex, smp), uv / vec2(textureSize(sampler2D(tex, smp), 0))).rgb);
	colour *= dot;
	colour = ToSrgb(colour);
	
	frag_color = vec4(colour, 1);
}


#pragma sokol @end
#pragma sokol @program world world_vs world_fs
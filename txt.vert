#version 460

#define StageVertex
#include "config.h"

struct txtquad {
	mat4 model;  // (align 16)  64     +
	vec4 rgba;   // (align 16)  16     +
//	uint pad[3]; // (align  4) (4 * 3
	vec3 pad;    // (align 16)  OR 12) +
	uint value;  // (align  4)  4      = 96
	};           // (align 16)  96 % 16 == 0

layout(set = SetUniform, binding = BindingUniform) uniform Uniform {
	mat4 viewproj;
	bool invertY;
} txtUniform;

layout(set = SetStorage, binding = BindingStorage) readonly buffer Storage {
	txtquad quad[Quads];
} txtStorage;

const float WorldMin = 0.f;
const float WorldMax = QuadSize;

// NOTE QuadTopology, triangle strip with 2 triangles
const vec4 World[QuadVertices] =
	{ { WorldMin, WorldMin, 0, 1 } // regular origin
	, { WorldMax, WorldMin, 0, 1 }
	, { WorldMin, WorldMax, 0, 1 } // Y-inverted orgin
	, { WorldMax, WorldMax, 0, 1 } };

// Clipping helps with edge artifacts when rasterizing.
// Glyphs share edges in the font image.
const float Clipping = .002f;

// Unnormalized texel size of a quad is [1, 1].
const float StUnormMin = 0.f + Clipping;
const float StUnormMax = 1.f - Clipping;

// Unnormalized texel size of the font image is [FontQuads, FontQuads].
const uint FontQuads = FontPoints / QuadPoints;
const float StNorm = float(QuadPoints) / float(FontPoints);

void main() {
	txtquad quad = txtStorage.quad[gl_InstanceIndex];
	gl_Position = txtUniform.viewproj * quad.model * World[gl_VertexIndex];
	rgba = quad.rgba;

	// Map texture upside down if Y-inverted
	float StUnormMinY;
	float StUnormMaxY;
	if (txtUniform.invertY) {
		StUnormMinY = StUnormMax;
		StUnormMaxY = StUnormMin;
	} else {
		StUnormMinY = StUnormMin;
		StUnormMaxY = StUnormMax;
	}

	vec2 WorldToStUnorm[QuadVertices] =
		{ { StUnormMin, StUnormMinY } // regular origin
		, { StUnormMax, StUnormMinY }
		, { StUnormMin, StUnormMaxY } // Y-inverted origin
		, { StUnormMax, StUnormMaxY } };

	vec2 stUnormOrigin = { quad.value % FontQuads
	                     , quad.value / FontQuads };
	st = (stUnormOrigin + WorldToStUnorm[gl_VertexIndex]) * StNorm;
}

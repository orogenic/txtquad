#version 460

#define StageFragment
#include "config.h"

layout(set = SetFont, binding = BindingFontSampledImage) uniform texture2D FontSampledImage;
layout(set = SetFont, binding = BindingFontSampler     ) uniform sampler   FontSampler;

layout(location = 0) out vec4 color;

void main() {
	// 1-bit color, discard blank pixels, draw colored pixels using quad color
	// NOTE FontImageFormat, tqValidatePBM, colorWriteMask
	if (texture(sampler2D( FontSampledImage
	                     , FontSampler ), st).r == 0.f) discard;
	color = rgba;
}

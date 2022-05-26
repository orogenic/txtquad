#define EngineName "txtquad"

// ENGINE INFO FOR C ONLY
#define MonitorIndex         0
#define PhysicalDeviceIndex  0
#define QueueFamilyIndex     0
#define SwapchainImageCount  4
#define SwapchainPresentMode VK_PRESENT_MODE_FIFO_KHR
#define SwapchainImageFormat VK_FORMAT_B8G8R8A8_UNORM
#define FontImageFormat      VK_FORMAT_R8_UNORM
#define QuadTopology         VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP

// ENGINE INFO FOR GLSL AND C
#define QuadVertices 4 // it's in the name!
#define QuadSize     1.f
#define Quads        0x30000

#define FontPoints 0200
#define QuadPoints 010

// PIPELINE LAYOUT FOR GLSL AND C
#define SetUniform 0
#define SetStorage 1
#define SetFont    2

#define BindingUniform          0
#define BindingStorage          0
#define BindingFontSampledImage 0
#define BindingFontSampler      1

// PIPELINE LAYOUT FOR GLSL ONLY
#if defined(StageVertex) || defined(StageFragment)
# ifdef StageVertex
#  define VertToFrag out
# endif
# ifdef StageFragment
#  define VertToFrag in
# endif
layout(location = 0) VertToFrag vec4 rgba;
layout(location = 1) VertToFrag vec2 st;
#endif

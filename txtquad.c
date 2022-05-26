#pragma STDC FENV_ACCESS ON

#include <assert.h>
#include <errno.h>
#include <fenv.h>
#include <float.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define Input
#define DebugFps
#define DebugInfo
#define DebugRun
#include "txtquad.h"

#define BakeAssets
#define VkValidation

struct tqframe frame = { .ts = 1.f };

void tqDebugLevel (uint8_t level) {
	DebugLevel(level);
}

#if defined(DebugFps)
struct tqfps fps = { 0 };

static void tqDebugFps (void) {
	uint8_t const i = frame.i % RdtSize;
	fps.rdtsum -= fps.rdt[i];
	fps.rdtsum += fps.rdt[i] = frame.rdt;
	fps.sec += frame.rdt;
	if (fps.sec > 1.) fps.sec -= 1.;
	if (!(frame.i % 10)) snprintf(fps.str, 4, "%3u", (unsigned int) AvgFps);
}
#else
	#define tqDebugFps(...)
#endif // defined(DebugFPS)

static struct { bool resize, fbresize, done; } TQfun;

static void TQwindowsizefun
	( GLFWwindow * const __window
	, int          const width
	, int          const height )
{
	info( "Window size from %dx%d to %dx%d screen coordinates\n"
	    , frame.w, frame.h, width, height );
	if (frame.w != width || frame.h != height)
		frame.w = width,
		frame.h = height,
		TQfun.resize = true;
}

static void TQframebuffersizefun
	( GLFWwindow * const __window
	, int          const width
	, int          const height )
{
	info( "Framebuffer size from %dx%d to %dx%d pixels\n"
	    , frame.fbw, frame.fbh, width, height );
	if (frame.fbw != width || frame.fbh != height)
		frame.fbw = width,
		frame.fbh = height,
		TQfun.fbresize = true;
}

#if defined(Input)
struct tqinput input = { 0 };

static void TQcharfun
	( GLFWwindow * const __window
	, unsigned int const unicode )
{ if (input.unicode(unicode)) TQfun.done = true; }

static void TQcursorposfun
	( GLFWwindow * const __window
	, double       const xpos
	, double       const ypos )
{ if (input.cursor(xpos, ypos)) TQfun.done = true; }

static void TQscrollfun
	( GLFWwindow * const __window
	, double       const xoffset
	, double       const yoffset )
{ if (input.scroll(xoffset, yoffset)) TQfun.done = true; }

static void tqInputInit (GLFWwindow * const window) {
	if ( input.unicode ) glfwSetCharCallback      ( window, TQcharfun      );
	if ( input.cursor  ) glfwSetCursorPosCallback ( window, TQcursorposfun );
	if ( input.scroll  ) glfwSetScrollCallback    ( window, TQscrollfun    );
}

static void tqInputPoll (GLFWwindow * const window) {
	for (uint16_t i = 0; i < input.keyCount; ++i) {
		input.key[input.keys[i]] <<= 1;
		input.key[input.keys[i]] |=
			glfwGetKey(window, input.keys[i]);
	}
	for (uint16_t i = 0; i < input.buttonCount; ++i) {
		input.button[input.buttons[i]] <<= 1;
		input.button[input.buttons[i]] |=
			glfwGetMouseButton(window, input.buttons[i]);
	}
}
#else
	#define tqInputInit(...)
	#define tqInputPoll(...)
#endif // defined(Input)

#if DebugOn
static inline char const * str_VkMemoryPropertyFlagBits
	( VkMemoryPropertyFlagBits const flag )
{ switch (flag)
{ case    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
: return "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT"
; case    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
: return "VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT"
; case    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
: return "VK_MEMORY_PROPERTY_HOST_COHERENT_BIT"
; case    VK_MEMORY_PROPERTY_HOST_CACHED_BIT
: return "VK_MEMORY_PROPERTY_HOST_CACHED_BIT"
; case    VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT
: return "VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT"
; case    VK_MEMORY_PROPERTY_PROTECTED_BIT
: return "VK_MEMORY_PROPERTY_PROTECTED_BIT"
; case    VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD
: return "VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD"
; case    VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD
: return "VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD"
; default
: return "Unhandled VkMemoryPropertyFlagBits" ; } }

static inline char const * str_VkPhysicalDeviceType
	( VkPhysicalDeviceType const type )
{ switch (type)
{ case    VK_PHYSICAL_DEVICE_TYPE_OTHER
: return "VK_PHYSICAL_DEVICE_TYPE_OTHER"
; case    VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU
: return "VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU"
; case    VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
: return "VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU"
; case    VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU
: return "VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU"
; case    VK_PHYSICAL_DEVICE_TYPE_CPU
: return "VK_PHYSICAL_DEVICE_TYPE_CPU"
; default
: return "Unhandled VkPhysicalDeviceType" ; } }

static inline void info_VkMemoryPropertyFlags
	( char          const * const fmt
	, VkMemoryPropertyFlags const flags )
{
	VkMemoryPropertyFlagBits flag;
	for ( flag   = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	    ; flag  <= VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD
	    ; flag <<= 1 )
		if (flags & flag) info(fmt, str_VkMemoryPropertyFlagBits(flag));
}

static char const * PRIfiB (size_t const bytes) {
	static char const error[] = "out of range";
	static char str[Max(sizeof "0000.00_iB", sizeof error)];
	if (bytes < 0x1p10) snprintf( str, sizeof "0000B",    "%zuB", bytes          ); else
	if (bytes < 0x1p20) snprintf( str, sizeof     str, "%.2fKiB", bytes / 0x1p10 ); else
	if (bytes < 0x1p30) snprintf( str, sizeof     str, "%.2fMiB", bytes / 0x1p20 ); else
	if (bytes < 0x1p40) snprintf( str, sizeof     str, "%.2fGiB", bytes / 0x1p30 ); else
	if (bytes < 0x1p50) snprintf( str, sizeof     str, "%.2fTiB", bytes / 0x1p40 ); else
	{ static_assert(sizeof str >= sizeof error, "buffer overrun"); strcpy(str, error); }
	return str;
}
#else
	#define str_VkMemoryPropertyFlagBits(...)
	#define str_VkPhysicalDeviceType(...)
	#define info_VkMemoryPropertyFlags(...)
	#define PRIfiB(...)
#endif // DebugOn

typedef struct TqShaderModule {
	size_t          codeSize     ;
	uint32_t      * pCode        ;
	VkShaderModule  shaderModule ;
} TqShaderModule;

static void tqValidateSPV
	( VkDevice         const device
	, size_t           const codeSize // in bytes
	, uint32_t               pCode[static const codeSize / 4] // in 4-byte words
	, TqShaderModule * const pShaderModule )
{
	if (pCode[0] != 0x07230203)
		fail("Invalid SPIR-V Magic Number");

	if (codeSize % 4)
		fail( "SPIR-V code size %zu is not a whole number of 4-byte words"
		    , codeSize );

	VkShaderModule shaderModule;
	vkCreateShaderModule(device, &(VkShaderModuleCreateInfo)
	{ .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO
	, .pNext    = NULL
	, .flags    = 0
	, .codeSize = codeSize
	, .pCode    = pCode
	}, NULL, &shaderModule);

	*pShaderModule = (TqShaderModule)
		{ codeSize
		, pCode
		, shaderModule };
}

#define FontBufferSize (FontPoints * FontPoints)
#define PBMHeaderSize  11
#define PBMDataSize    (FontBufferSize / 8)

static void tqValidatePBM
	( size_t           const size
	, uint8_t          const pPBM[static size]
	, uint8_t const ** const ppData )
{
	if (size != PBMHeaderSize + PBMDataSize)
		fail("Invalid PBM size");
	if (strncmp((char const * const) pPBM, "P4 ", 3) || pPBM[10] != '\n')
		fail("Invalid PBM header");
	if (strncmp((char const * const) pPBM + 3, "128 128", 7))
		fail("PBM must be 128 128 but is %7s", pPBM + 3);

	pPBM += PBMHeaderSize;

	// expand 1 bit of PBM color data to 1 byte of font image color data. NOTE FontImageFormat.
	static uint8_t pData[FontBufferSize];
	for (size_t i = 0; i < PBMDataSize; ++i)
		for (uint8_t j = 0; j < 8; ++j)
			// read bits high to low
			pData[8 * i + j] = pPBM[i] & (1 << (7 - j)) ? 0xFF : 0;

	*ppData = pData;
}

#if defined(BakeAssets)
	// headers generated by f2c.exe, NOTE build.ninja, f2c, ident.
	#include "font.pbm.h"
	#include "vert.spv.h"
	#include "frag.spv.h"

static void tqFontPBM (uint8_t const ** const ppData)
{ tqValidatePBM(sizeof fontPBM, fontPBM, ppData); }

static void tqVertSPV
	( VkDevice         const device
	, TqShaderModule * const pVert )
{ tqValidateSPV(device, sizeof vertSPV, (uint32_t *) vertSPV, pVert); }

static void tqFragSPV
	( VkDevice         const device
	, TqShaderModule * const pFrag )
{ tqValidateSPV(device, sizeof fragSPV, (uint32_t *) fragSPV, pFrag); }

	#define tqFileFree(ptr)
#else
	#define tqFileFree(ptr) free(ptr)

static void * tqFile
	( char const * const filename
	, size_t       const max
	, size_t     * const pSize )
{
	errno = 0;
	FILE * file = fopen(filename, "rb");
	if (errno)
		goto FileError;
	if (fseek(file, 0, SEEK_END))
		goto FileError;
	errno = 0;
	size_t size = (size_t) ftell(file);
	if (errno)
		goto FileError;
	if (size > max) {
		warn( "Warning: truncating read to %zu bytes (file ends at %zu bytes)\n"
		    , max
		    , size );
		size = max;
	}
	if (fseek(file, 0, SEEK_SET))
		goto FileError;
	void * ptr = malloc(size);
	clearerr(file);
	fread(ptr, 1, size, file);
	if (ferror(file))
		goto FileError;
	if (fclose(file))
		goto FileError;
	info("Read file \"%s\" (%zu bytes)\n", filename, size);
	*pSize = size;
	return ptr;
	FileError:
	fail("Error reading file \"%s\"", filename);
}

static void tqFilePBM
	( char    const  * const filename
	, uint8_t const ** const ppData )
{
	size_t size;
	uint8_t * pPBM = tqFile(filename, PBMHeaderSize + PBMDataSize, &size);
	tqValidatePBM(size, pPBM, ppData);
	tqFileFree(pPBM);
}

static void tqFileSPV
	( VkDevice         const device
	, char const     * const filename
	, TqShaderModule * const pShaderModule )
{
	size_t codeSize;
	uint32_t * const pCode = tqFile(filename, 0x100000L, &codeSize);
	tqValidateSPV(device, codeSize, pCode, pShaderModule);
}

static void tqFontPBM (uint8_t const ** const ppData)
{ tqFilePBM("font.pbm", ppData); }

static void tqVertSPV
	( VkDevice         const device
	, TqShaderModule * const pVert )
{ tqFileSPV(device, "vert.spv", pVert); }

static void tqFragSPV
	( VkDevice         const device
	, TqShaderModule * const pFrag )
{ tqFileSPV(device, "frag.spv", pFrag); }
#endif // defined(BakeAssets)

static void tqDestroyShaderModule
	( VkDevice       const device
	, TqShaderModule const shaderModule )
{
	vkDestroyShaderModule(device, shaderModule.shaderModule, NULL);
	tqFileFree(shaderModule.pCode);
}

static VkPhysicalDeviceMemoryProperties memoryProperties;

static uint32_t firstMemoryType
	( uint32_t              const memoryTypeBits // lowest bit is index 0
	, VkMemoryPropertyFlags const flags )
{
	#define verbose 0

	#if verbose
	info("Searching for supported memory type with flags");
	info_VkMemoryPropertyFlags("\n\t%s", flags);
	info("\n\t" PRIb32, _PRIb32(memoryTypeBits)) ;
	#endif

	uint32_t i;
	for (i = 0; i < memoryProperties.memoryTypeCount; ++i)
		// type is supported if index bit is set
		if ( memoryTypeBits & 1 << i
		  && flags == ( flags & memoryProperties
		                      . memoryTypes[i]
		                      . propertyFlags ) )
			goto FoundMemoryType;

	#if verbose
	error(" no supported memory type\n");
	#endif

	return UINT32_MAX;
	FoundMemoryType:

	#if verbose
	info("\n\t%*s* using memory type %u\n", 31 - i, "", i);
	#endif

	return i;

	#undef verbose
}

static uint32_t bestMemoryType
	( uint32_t              const memoryTypeBits
	, VkMemoryPropertyFlags       flags
	, uint8_t               const optionalCount
	, VkMemoryPropertyFlags const optional[static const optionalCount] )
{
	// optional priority: first flag highest, last flag lowest
	uint32_t j;
	for (j = 0; j < optionalCount; ++j)
		flags |= optional[j];

	for (;;) {
		uint32_t const i = firstMemoryType(memoryTypeBits, flags);
		if (   i != UINT32_MAX ) return i;
		if ( --j == UINT32_MAX ) break;
		flags ^= optional[j];
	}

	return UINT32_MAX;
}

typedef struct TqImage {
	VkImage        image     ;
	VkDeviceMemory memory    ;
	VkImageView    imageView ;
} TqImage;

static void tqCreateImage
	( VkDevice               const device
	, VkFormat               const format
	, VkExtent3D             const extent
	, VkSampleCountFlagBits  const samples
	, VkImageUsageFlags      const usage
	, VkImageAspectFlags     const aspectMask
	, TqImage              * const pImage )
{
	VkImage image;
	vkCreateImage(device, &(VkImageCreateInfo)
	{ .sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO
	, .pNext                 = NULL
	, .flags                 = 0
	, .imageType             = VK_IMAGE_TYPE_2D
	, .format                = format
	, .extent                = extent
	, .mipLevels             = 1
	, .arrayLayers           = 1
	, .samples               = samples
	, .tiling                = VK_IMAGE_TILING_OPTIMAL
	, .usage                 = usage
	, .sharingMode           = VK_SHARING_MODE_EXCLUSIVE
	, .queueFamilyIndexCount = 0
	, .pQueueFamilyIndices   = NULL
	, .initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED
	}, NULL, &image);

	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(device, image, &memoryRequirements);

	VkDeviceMemory memory;

	uint32_t memoryTypeIndex = bestMemoryType
		(  memoryRequirements.memoryTypeBits
		// no required flags
		, 0
		// lazily allocated only for transient attachment
		, usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT ? 2 : 1
		, (VkMemoryPropertyFlags[])
			{ VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
			, VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT } );

	if (memoryTypeIndex == UINT32_MAX)
		fail("No supported memory type for image");

	vkAllocateMemory(device, &(VkMemoryAllocateInfo)
	{ .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO
	, .pNext           = NULL
	, .allocationSize  = memoryRequirements.size
	, .memoryTypeIndex = memoryTypeIndex
	}, NULL, &memory);
	vkBindImageMemory(device, image, memory, 0);

	VkImageView imageView;
	vkCreateImageView(device, &(VkImageViewCreateInfo)
	{ .sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO
	, .pNext            = NULL
	, .flags            = 0
	, .image            = image
	, .viewType         = VK_IMAGE_VIEW_TYPE_2D
	, .format           = format
	, .components       =
		{ .r = VK_COMPONENT_SWIZZLE_IDENTITY
		, .g = VK_COMPONENT_SWIZZLE_IDENTITY
		, .b = VK_COMPONENT_SWIZZLE_IDENTITY
		, .a = VK_COMPONENT_SWIZZLE_IDENTITY }
	, .subresourceRange =
		{ .aspectMask     = aspectMask
		, .baseMipLevel   = 0
		, .levelCount     = 1
		, .baseArrayLayer = 0
		, .layerCount     = 1 }
	}, NULL, &imageView);

	*pImage = (TqImage)
		{ image
		, memory
		, imageView };
}

static void tqDestroyImage (VkDevice device, TqImage image) {
	vkDestroyImageView(device, image.imageView, NULL);
	vkFreeMemory(device, image.memory, NULL);
	vkDestroyImage(device, image.image, NULL);
}

typedef struct TqBuffer {
	VkBuffer        buffer ;
	VkDeviceMemory  memory ;
	void          * pData  ;
} TqBuffer;

static void tqCreateBuffer
	( VkDevice            const device
	, VkDeviceSize        const size
	, VkBufferUsageFlags  const usage
	, char const        * const name
	, TqBuffer          * const pBuffer )
{
	VkBuffer buffer;
	vkCreateBuffer(device, &(VkBufferCreateInfo)
	{ .sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO
	, .pNext                 = NULL
	, .flags                 = 0
	, .size                  = size
	, .usage                 = usage
	, .sharingMode           = VK_SHARING_MODE_EXCLUSIVE
	, .queueFamilyIndexCount = 0
	, .pQueueFamilyIndices   = NULL
	}, NULL, &buffer);

	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);

	if (size < memoryRequirements.size)
		info( "Requested buffer size was %s, required size is %s\n"
		    , PRIfiB(size)
		    , PRIfiB(memoryRequirements.size) );
	else
	if (size > memoryRequirements.size)
		fail( "Requested buffer size was %s, required size is %s"
		    , PRIfiB(size)
		    , PRIfiB(memoryRequirements.size) );

	uint32_t memoryTypeIndex = bestMemoryType
		( memoryRequirements.memoryTypeBits
		, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
		| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		, 1
		, (VkMemoryPropertyFlags[1])
			{ VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT } );

	if (memoryTypeIndex == UINT32_MAX)
		fail("Need host-visible, host-coherent memory for buffer");

	VkDeviceMemory memory;
	vkAllocateMemory(device, &(VkMemoryAllocateInfo)
	{ .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO
	, .pNext           = NULL
	, .allocationSize  = memoryRequirements.size
	, .memoryTypeIndex = memoryTypeIndex
	}, NULL, &memory);
	vkBindBufferMemory(device, buffer, memory, 0);

	void * pData;
	vkMapMemory(device, memory, 0, VK_WHOLE_SIZE, 0, &pData);
	info( "Created mapped device buffer for %s (%s)\n"
	    , name
	    , PRIfiB(memoryRequirements.size) );

	*pBuffer = (TqBuffer)
		{ buffer
		, memory
		, pData };
}

static void tqDestroyBuffer
	( VkDevice const device
	, TqBuffer const buffer )
{
	vkDestroyBuffer(device, buffer.buffer, NULL);
	vkUnmapMemory(device, buffer.memory);
	vkFreeMemory(device, buffer.memory, NULL);
}

struct tqconfig config = { 0 };

static void tqCreateWindow (GLFWwindow ** const window) {
	if ( config.width  < 0
	  || config.height < 0 )
		warn("Should specify non-negative values for width and height\n");

	if (config.windowed &&  ( !config.width
	                       || !config.height ))
		fail("Cannot create window of zero size");

	if (!config.windowed &&  ( ( config.width && !config.height)
	                        || (!config.width &&  config.height) ))
		warn("For fullscreen, should specify both width and height, or neither\n");

	if (!glfwInit()) fail("Unable to initialize GLFW");

	int monitorCount;
	GLFWmonitor * const * const monitors = glfwGetMonitors(&monitorCount);
	if (!monitors) fail("No monitors found");
	GLFWmonitor * monitor = monitors[MonitorIndex];
	info( "Found %d monitor(s), using monitor %d named \"%s\"\n"
	    , monitorCount
	    , MonitorIndex
	    , glfwGetMonitorName(monitor) );

	int vidmodeCount;
	GLFWvidmode const * const vidmodes = glfwGetVideoModes(monitor, &vidmodeCount);
	if (!vidmodes) fail("No video modes found");
	info("Found %d video mode(s)", vidmodeCount);

	GLFWvidmode const * const current = glfwGetVideoMode(monitor);
	// last vidmode has highest color bit depth and highest resolution
	// probably highest refresh rate as well, but GLFW doesn't mention it
	GLFWvidmode const selected = current ? *current : vidmodes[vidmodeCount - 1];

	bool supported = false;
	for (int i = 0; i < vidmodeCount; ++i) {
		GLFWvidmode const vidmode = vidmodes[i];
		bool const requested = vidmode.width  == config.width
		                    && vidmode.height == config.height ;
		supported |= requested;
		info( "\n\t%5dx%-5d %4dHz (%.2f)%s%s"
		    , vidmode.width
		    , vidmode.height
		    , vidmode.refreshRate
		    , (float) vidmode.width / vidmode.height
		    , requested ? " <- requested resolution" : ""
		    , vidmode.width       == selected.width
		   && vidmode.height      == selected.height
		   && vidmode.redBits     == selected.redBits
		   && vidmode.greenBits   == selected.greenBits
		   && vidmode.blueBits    == selected.blueBits
		   && vidmode.refreshRate == selected.refreshRate
		    ? current ? " <- current" : "<- best" : "" );
	}
	info("\n");

	if (config.windowed) {
		monitor = NULL;
	} else if (config.width && config.height) {
		if (!supported) warn("Requested fullscreen resolution may not be supported\n");
	} else {
		if (!current) warn("Warning: no current video mode, trying best video mode\n");
		config.width = selected.width;
		config.height = selected.height;
	}

	// using vulkan, disable context creation
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	glfwWindowHint(GLFW_DECORATED, config.decorated);

	// does nothing on wayland, works on windows, does nothing on x.org
	glfwWindowHint(GLFW_MAXIMIZED, config.maximized);

	// never resizable at first, prevents immediate resizing on some window managers
	// Wayland appears to support hints for non-tiling windows, GLFW appears not to expose them
	glfwWindowHint(GLFW_RESIZABLE, false);
	*window = glfwCreateWindow
		( config.width
		, config.height
		, config.title ? config.title : EngineName
		, monitor
		, NULL );
	glfwSetWindowAttrib(*window, GLFW_RESIZABLE, config.resizable);

	if (!config.cursor)
		glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	glfwGetWindowSize(*window, &frame.w, &frame.h);
	glfwGetFramebufferSize(*window, &frame.fbw, &frame.fbh);
	info( "Created window size %dx%d screen coordinates, framebuffer size %dx%d pixels\n"
	    , frame.w, frame.h, frame.fbw, frame.fbh );

	glfwSetFramebufferSizeCallback(*window, TQframebuffersizefun);
	glfwSetWindowSizeCallback(*window, TQwindowsizefun);
}

static void tqCreateInstance
	( char const * const pApplicationName
	, uint32_t     const applicationVersion
	, VkInstance * const pInstance )
{
	uint32_t glfwExtensionCount;
	char const ** const glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	uint32_t extensionPropertyCount;
	vkEnumerateInstanceExtensionProperties
		( NULL
		, &extensionPropertyCount
		, NULL );

	VkExtensionProperties pExtensionProperties[extensionPropertyCount];
	vkEnumerateInstanceExtensionProperties
		( NULL
		, &extensionPropertyCount
		, pExtensionProperties );

	for (uint32_t i = 0; i < glfwExtensionCount; ++i) {
		for (uint32_t j = 0; j < extensionPropertyCount; ++j)
			if (!strcmp( pExtensionProperties[j].extensionName
			           , glfwExtensions[i] )) goto FoundExtension;
		fail("Missing GLFW required instance extension %s", glfwExtensions[i]);
		FoundExtension:
		info("Found GLFW required instance extension %s\n", glfwExtensions[i]);
	}

	static char const * const ppEnabledLayerNames[] =
	{
#ifdef VkValidation
		"VK_LAYER_KHRONOS_validation"
#else
		0
#endif
	};
	uint32_t layerPropertyCount;
	vkEnumerateInstanceLayerProperties(&layerPropertyCount, NULL);
	VkLayerProperties pLayerProperties[layerPropertyCount];
	vkEnumerateInstanceLayerProperties(&layerPropertyCount, pLayerProperties);
	for (uint32_t i = 0; i < ArraySize(ppEnabledLayerNames); ++i) {
		for (uint32_t j = 0; j < layerPropertyCount; ++j)
			if (!strcmp( pLayerProperties[j].layerName
			           , ppEnabledLayerNames[i] )) goto FoundLayer;
		fail("Missing requested instance layer %s", ppEnabledLayerNames[i]);
		FoundLayer:
		info("Found requested instance layer %s\n", ppEnabledLayerNames[i]);
	}

	vkCreateInstance( &(VkInstanceCreateInfo)
	{ .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO
	, .pNext = NULL
	, .flags = 0
	, .pApplicationInfo = &(VkApplicationInfo)
		{ .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO
		, .pNext              = NULL
		, .pApplicationName   = pApplicationName
		, .applicationVersion = applicationVersion
		, .pEngineName        = EngineName
		, .engineVersion      = VK_MAKE_VERSION(0, 2, 0)
		, .apiVersion         = VK_API_VERSION_1_2 }
	, .enabledExtensionCount   = glfwExtensionCount
	, .ppEnabledExtensionNames = glfwExtensions
	, .enabledLayerCount       = ArraySize(ppEnabledLayerNames)
	, .ppEnabledLayerNames     = ppEnabledLayerNames
	}, NULL, pInstance );
	info("Created application \"%s\" using engine \"" EngineName "\"\n", pApplicationName);
}

static VkPhysicalDeviceFeatures enabledFeatures = { 0 };

static void tqCreateDevice
	( VkInstance                   const instance
	, VkSurfaceKHR                 const surface
	, VkPhysicalDevice           * const pPhysicalDevice
	, VkPhysicalDeviceProperties * const pProperties
	, VkDevice                   * const pDevice
	, VkQueue                    * const pQueue )
{
	uint32_t physicalDeviceCount;
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, NULL);
	if (!physicalDeviceCount) fail("No physical device available");
	VkPhysicalDevice pPhysicalDevices[physicalDeviceCount];
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, pPhysicalDevices);
	VkPhysicalDevice physicalDevice = pPhysicalDevices[PhysicalDeviceIndex];
	*pPhysicalDevice = physicalDevice;

	static char const * const ppEnabledExtensionNames[] =
		{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	uint32_t extensionPropertyCount;
	vkEnumerateDeviceExtensionProperties( physicalDevice
	                                    , NULL
	                                    , &extensionPropertyCount
	                                    , NULL );
	VkExtensionProperties pExtensionProperties[extensionPropertyCount];
	vkEnumerateDeviceExtensionProperties( physicalDevice
	                                    , NULL
	                                    , &extensionPropertyCount
	                                    , pExtensionProperties );
	for (uint32_t i = 0; i < ArraySize(ppEnabledExtensionNames); ++i) {
		for (uint32_t j = 0; j < extensionPropertyCount; ++j)
			if (!strcmp( pExtensionProperties[j].extensionName
			           , ppEnabledExtensionNames[i]))
				goto FoundExtension;
		fail("Missing required device extension %s", ppEnabledExtensionNames[i]);
		FoundExtension:
		info("Found required device extension %s\n", ppEnabledExtensionNames[i]);
	}

	uint32_t surfaceFormatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR( physicalDevice
	                                    , surface
	                                    , &surfaceFormatCount
	                                    , NULL );
	VkSurfaceFormatKHR pSurfaceFormats[surfaceFormatCount];
	vkGetPhysicalDeviceSurfaceFormatsKHR( physicalDevice
	                                    , surface
	                                    , &surfaceFormatCount
	                                    , pSurfaceFormats );
	for (uint32_t i = 0; i < surfaceFormatCount; ++i)
		if (pSurfaceFormats[i].format == SwapchainImageFormat)
			goto FoundSwapchainImageFormat;
	fail("Missing requested swapchain image format " Str(SwapchainImageFormat));
	FoundSwapchainImageFormat:
	info("Found requested swapchain image format " Str(SwapchainImageFormat) "\n");

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR( physicalDevice
	                                         , surface
	                                         , &presentModeCount
	                                         , NULL );
	VkPresentModeKHR pPresentModes[presentModeCount];
	vkGetPhysicalDeviceSurfacePresentModesKHR( physicalDevice
	                                         , surface
	                                         , &presentModeCount
	                                         , pPresentModes );
	for (uint32_t i = 0; i < presentModeCount; ++i)
		if (pPresentModes[i] == SwapchainPresentMode)
			goto FoundSwapchainPresentMode;
	fail("Missing requested presentMode " Str(SwapchainPresentMode));
	FoundSwapchainPresentMode:
	info("Found requested presentMode " Str(SwapchainPresentMode) "\n");

	vkGetPhysicalDeviceProperties(physicalDevice, pProperties);
	info( "Physical device %u selected"
	      "\n\tDevice Name \"%s\""
	      "\n\tDevice Type %s"
	      "\n\tAPI Version %u.%u.%u"
	    , PhysicalDeviceIndex
	    , pProperties->deviceName
	    , str_VkPhysicalDeviceType(pProperties->deviceType)
	    , VK_VERSION_MAJOR(pProperties->apiVersion)
	    , VK_VERSION_MINOR(pProperties->apiVersion)
	    , VK_VERSION_PATCH(pProperties->apiVersion) );

	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
	for (uint32_t heapIndex = 0; heapIndex < memoryProperties.memoryHeapCount; ++heapIndex) {
		info( "\n\tMemory heap %u (%s)"
		    , heapIndex
		    , PRIfiB(memoryProperties.memoryHeaps[heapIndex].size) );
		for (uint32_t typeIndex = 0; typeIndex < memoryProperties.memoryTypeCount; ++typeIndex) {
			VkMemoryType const type = memoryProperties.memoryTypes[typeIndex];
			if (type.heapIndex == heapIndex) {
				info("\n\t\tMemory type %u", typeIndex);
				info_VkMemoryPropertyFlags("\n\t\t\t%s", type.propertyFlags);
			}
		}
	}
	info("\n");

	uint32_t queueFamilyPropertyCount;
	vkGetPhysicalDeviceQueueFamilyProperties( physicalDevice
	                                        , &queueFamilyPropertyCount
	                                        , NULL );
	VkQueueFamilyProperties pQueueFamilyProperties[queueFamilyPropertyCount];
	vkGetPhysicalDeviceQueueFamilyProperties( physicalDevice
	                                        , &queueFamilyPropertyCount
	                                        , pQueueFamilyProperties );
	VkBool32 supported;
	vkGetPhysicalDeviceSurfaceSupportKHR( physicalDevice
	                                    , QueueFamilyIndex
	                                    , surface
	                                    , &supported );
	if (!supported) fail( "Queue family %u not supported"
                            , QueueFamilyIndex );

	VkPhysicalDeviceFeatures features;
	vkGetPhysicalDeviceFeatures(physicalDevice, &features);

	// enable multisampling
	enabledFeatures.sampleRateShading |= features.sampleRateShading ;

	vkCreateDevice(physicalDevice, &(VkDeviceCreateInfo)
	{ .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO
	, .pNext                   = NULL
	, .flags                   = 0
	, .queueCreateInfoCount    = 1
	, .pQueueCreateInfos       = (VkDeviceQueueCreateInfo[1])
		{ { .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO
		  , .pNext            = NULL
		  , .queueFamilyIndex = QueueFamilyIndex
		  , .queueCount       = 1
		  , .pQueuePriorities = (float[1]) { 1.f } } }
	, .enabledLayerCount       = 0
	, .ppEnabledLayerNames     = NULL
	, .enabledExtensionCount   = ArraySize(ppEnabledExtensionNames)
	, .ppEnabledExtensionNames = ppEnabledExtensionNames
	, .pEnabledFeatures        = &enabledFeatures
	}, NULL, pDevice);

	vkGetDeviceQueue(*pDevice, QueueFamilyIndex, 0, pQueue);
}

typedef struct TqFont {
	TqImage               image     ;
	VkDescriptorImageInfo imageInfo ;
} TqFont;

static void tqCreateFont
	( VkDevice       const device
	, float          const maxSamplerAnisotropy
	, VkQueue        const queue
	, VkCommandPool  const commandPool
	, TqFont       * const pFont )
{
	TqBuffer buffer;
	tqCreateBuffer( device
	              , FontBufferSize
	              , VK_BUFFER_USAGE_TRANSFER_SRC_BIT
	              , "font"
	              , &buffer );

	uint8_t const * pData;
	tqFontPBM(&pData);
	memcpy(buffer.pData, pData, FontBufferSize);

	VkExtent3D const fontExtent =
		{ .width  = FontPoints
		, .height = FontPoints
		, .depth  = 1 };
	TqImage fontImage;
	tqCreateImage( device
	             , FontImageFormat
	             , fontExtent
	             , VK_SAMPLE_COUNT_1_BIT
	             , VK_IMAGE_USAGE_SAMPLED_BIT
	             | VK_IMAGE_USAGE_TRANSFER_DST_BIT
	             , VK_IMAGE_ASPECT_COLOR_BIT
	             , &fontImage );

	VkSampler sampler;
	vkCreateSampler(device, &(VkSamplerCreateInfo)
	{ .sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO
	, .pNext                   = NULL
	, .flags                   = 0
	, .magFilter               = VK_FILTER_NEAREST
	, .minFilter               = VK_FILTER_NEAREST
	, .mipmapMode              = VK_SAMPLER_MIPMAP_MODE_NEAREST
	, .addressModeU            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER
	, .addressModeV            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER
	, .addressModeW            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER
	, .mipLodBias              = 0.f
	, .anisotropyEnable        = enabledFeatures.samplerAnisotropy
	, .maxAnisotropy           = enabledFeatures.samplerAnisotropy ? maxSamplerAnisotropy : 0.f
	, .compareEnable           = VK_FALSE
	, .compareOp               = VK_COMPARE_OP_NEVER
	, .minLod                  = 0.f
	, .maxLod                  = 0.f
	, .borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK
	, .unnormalizedCoordinates = VK_FALSE
	}, NULL, &sampler);

	VkImageMemoryBarrier memoryBarrier =
	{ .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER
	, .pNext               = NULL
	, .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED
	, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED
	, .image               = fontImage.image
	, .subresourceRange    =
		{ .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT
		, .baseMipLevel   = 0
		, .levelCount     = 1
		, .baseArrayLayer = 0
		, .layerCount     = 1 } };

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &(VkCommandBufferAllocateInfo)
	{ .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO
	, .pNext              = NULL
	, .commandPool        = commandPool
	, .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY
	, .commandBufferCount = 1
	}, &commandBuffer);
	vkBeginCommandBuffer(commandBuffer, &(VkCommandBufferBeginInfo)
	{ .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
	, .pNext            = NULL
	, .flags            = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	, .pInheritanceInfo = NULL
	});

	memoryBarrier.srcAccessMask = 0;
	memoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	memoryBarrier.oldLayout     = VK_IMAGE_LAYOUT_UNDEFINED;
	memoryBarrier.newLayout     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	vkCmdPipelineBarrier( commandBuffer
	                    , VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT
	                    , VK_PIPELINE_STAGE_TRANSFER_BIT
	                    , 0
	                    , 0, NULL
	                    , 0, NULL
	                    , 1, &memoryBarrier );

	vkCmdCopyBufferToImage( commandBuffer
	                      , buffer.buffer
	                      , fontImage.image
	                      , VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
	                      , 1, (VkBufferImageCopy[1])
	{ { .bufferOffset      = 0
	  , .bufferRowLength   = 0
	  , .bufferImageHeight = 0
	  , .imageSubresource  =
		{ .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT
		, .mipLevel       = 0
		, .baseArrayLayer = 0
		, .layerCount     = 1 }
	  , .imageOffset       =
		{ .x = 0
		, .y = 0
		, .z = 0 }
	  , .imageExtent       = fontExtent }
	});

	memoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	memoryBarrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	memoryBarrier.newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	vkCmdPipelineBarrier( commandBuffer
	                    , VK_PIPELINE_STAGE_TRANSFER_BIT
	                    , VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
	                    , 0
	                    , 0, NULL
	                    , 0, NULL
	                    , 1, &memoryBarrier );

	vkEndCommandBuffer(commandBuffer);
	vkQueueSubmit(queue, 1, (VkSubmitInfo[1])
	{ { .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO
	  , .pNext                = NULL
	  , .waitSemaphoreCount   = 0
	  , .pWaitSemaphores      = NULL
	  , .pWaitDstStageMask    = NULL
	  , .commandBufferCount   = 1
	  , .pCommandBuffers      = &commandBuffer
	  , .signalSemaphoreCount = 0
	  , .pSignalSemaphores    = NULL }
	}, NULL);
	vkQueueWaitIdle(queue);
	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
	tqDestroyBuffer(device, buffer);
	*pFont = (TqFont)
		{ .image     = fontImage
		, .imageInfo =
			{ .sampler     = sampler
			, .imageView   = fontImage.imageView
			, .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL } };
}

static void tqDestroyFont
	( VkDevice device
	, TqFont font )
{
	tqDestroyImage(device, font.image);
	vkDestroySampler(device, font.imageInfo.sampler, NULL);
}

typedef struct TqBufferRange {
	TqBuffer     buffer ;
	VkDeviceSize range  ;
} TqBufferRange;

static void tqCreateDescriptorSetLayout
	( VkDevice                             const device
	, uint32_t                             const bindingCount
	, VkDescriptorSetLayoutBinding const * const pBindings
	, VkDescriptorSetLayout              * const pSetLayout )
{
	vkCreateDescriptorSetLayout(device, &(VkDescriptorSetLayoutCreateInfo)
	{ .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO
	, .pNext        = NULL
	, .flags        = 0
	, .bindingCount = bindingCount
	, .pBindings    = pBindings
	}, NULL, pSetLayout);
}

static uint32_t imageIndex;
#define ForImageIndex for (imageIndex = 0; imageIndex < SwapchainImageCount; ++imageIndex)

#define DescriptorSetsUniform   SwapchainImageCount
#define DescriptorSetsStorage   SwapchainImageCount
#define DescriptorSetsFont      1

#define DescriptorWritesUniform 1 * DescriptorSetsUniform
#define DescriptorWritesStorage 1 * DescriptorSetsStorage
#define DescriptorWritesFont    2 * DescriptorSetsFont

#define DescriptorUniform       imageIndex
#define DescriptorStorage       DescriptorSetsUniform + imageIndex
#define DescriptorFont          DescriptorSetsUniform + DescriptorSetsStorage

#define SetLayoutCount 3
static VkDescriptorSetLayout pSetLayouts[SetLayoutCount];

static void tqUpdateDescriptorSets
	( VkDevice           const device
	, TqBufferRange      const uniform
	, TqBufferRange      const storage
	, TqFont             const font
	, VkDescriptorPool * const pDescriptorPool
	, VkPipelineLayout * const pLayout
	, VkDescriptorSet ** const ppDescriptorSets )
{
	vkCreateDescriptorPool(device, &(VkDescriptorPoolCreateInfo)
	{ .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO
	, .pNext         = NULL
	, .flags         = 0
	, .maxSets       = DescriptorSetsUniform
	                 + DescriptorSetsStorage
	                 + DescriptorSetsFont
	, .poolSizeCount = 4
	, .pPoolSizes    = (VkDescriptorPoolSize[4])
		{ { .type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
		  , .descriptorCount = DescriptorSetsUniform }
		, { .type            = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
		  , .descriptorCount = DescriptorSetsStorage }
		, { .type            = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
		  , .descriptorCount = DescriptorSetsFont }
		, { .type            = VK_DESCRIPTOR_TYPE_SAMPLER
		  , .descriptorCount = DescriptorSetsFont } }
	}, NULL, pDescriptorPool);

	tqCreateDescriptorSetLayout(device, 1, (VkDescriptorSetLayoutBinding[1])
	{ { .binding            = BindingUniform
	  , .descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
	  , .descriptorCount    = 1
	  , .stageFlags         = VK_SHADER_STAGE_VERTEX_BIT
	  , .pImmutableSamplers = NULL }
	}, &pSetLayouts[SetUniform]);

	tqCreateDescriptorSetLayout(device, 1, (VkDescriptorSetLayoutBinding[1])
	{ { .binding            = BindingStorage
	  , .descriptorType     = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
	  , .descriptorCount    = 1
	  , .stageFlags         = VK_SHADER_STAGE_VERTEX_BIT
	  , .pImmutableSamplers = NULL }
	}, &pSetLayouts[SetStorage]);

	tqCreateDescriptorSetLayout(device, 2, (VkDescriptorSetLayoutBinding[2])
	{ { .binding            = BindingFontSampledImage
	  , .descriptorType     = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
	  , .descriptorCount    = 1
	  , .stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT
	  , .pImmutableSamplers = NULL }
	, { .binding            = BindingFontSampler
	  , .descriptorType     = VK_DESCRIPTOR_TYPE_SAMPLER
	  , .descriptorCount    = 1
	  , .stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT
	  , .pImmutableSamplers = NULL }
	}, &pSetLayouts[SetFont]);

	vkCreatePipelineLayout(device, &(VkPipelineLayoutCreateInfo)
	{ .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO
	, .pNext                  = NULL
	, .flags                  = 0
	, .setLayoutCount         = SetLayoutCount
	, .pSetLayouts            = pSetLayouts
	, .pushConstantRangeCount = 0
	, .pPushConstantRanges    = NULL
	}, NULL, pLayout);

	static VkDescriptorSet pDescriptorSets[ DescriptorSetsUniform
	                                      + DescriptorSetsStorage
	                                      + DescriptorSetsFont ];
	*ppDescriptorSets = pDescriptorSets;

	VkDescriptorSetLayout pSetLayoutsAllocate[ArraySize(pDescriptorSets)] =
	{ [DescriptorFont] = pSetLayouts[SetFont] };

	ForImageIndex {
		pSetLayoutsAllocate[DescriptorUniform] = pSetLayouts[SetUniform];
		pSetLayoutsAllocate[DescriptorStorage] = pSetLayouts[SetStorage];
	}

	vkAllocateDescriptorSets(device, &(VkDescriptorSetAllocateInfo)
	{ .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO
	, .pNext              = NULL
	, .descriptorPool     = *pDescriptorPool
	, .descriptorSetCount = ArraySize(pDescriptorSets)
	, .pSetLayouts        = pSetLayoutsAllocate
	}, pDescriptorSets);

	VkWriteDescriptorSet pDescriptorWrites[ DescriptorWritesUniform
	                                      + DescriptorWritesStorage
	                                      + DescriptorWritesFont ] =
	{ [DescriptorFont + 0] =
	  { .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
	  , .pNext            = NULL
	  , .dstSet           = pDescriptorSets[DescriptorFont]
	  , .dstBinding       = BindingFontSampledImage
	  , .dstArrayElement  = 0
	  , .descriptorCount  = 1
	  , .descriptorType   = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
	  , .pImageInfo       = &font.imageInfo
	  , .pBufferInfo      = NULL
	  , .pTexelBufferView = NULL }
	, [DescriptorFont + 1] =
	  { .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
	  , .pNext            = NULL
	  , .dstSet           = pDescriptorSets[DescriptorFont]
	  , .dstBinding       = BindingFontSampler
	  , .dstArrayElement  = 0
	  , .descriptorCount  = 1
	  , .descriptorType   = VK_DESCRIPTOR_TYPE_SAMPLER
	  , .pImageInfo       = &font.imageInfo
	  , .pBufferInfo      = NULL
	  , .pTexelBufferView = NULL } };

	VkDescriptorBufferInfo pBufferInfos[ DescriptorWritesUniform
	                                   + DescriptorWritesStorage ];
	ForImageIndex {
		pBufferInfos      [DescriptorUniform] = (VkDescriptorBufferInfo)
		{ .buffer = uniform.buffer.buffer
		, .offset = uniform.range * imageIndex
		, .range  = uniform.range };
		pDescriptorWrites [DescriptorUniform] = (VkWriteDescriptorSet)
		{ .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
		, .pNext            = NULL
		, .dstSet           = pDescriptorSets[DescriptorUniform]
		, .dstBinding       = BindingUniform
		, .dstArrayElement  = 0
		, .descriptorCount  = 1
		, .descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
		, .pImageInfo       = NULL
		, .pBufferInfo      = &pBufferInfos[DescriptorUniform]
		, .pTexelBufferView = NULL };
		pBufferInfos      [DescriptorStorage] = (VkDescriptorBufferInfo)
		{ .buffer = storage.buffer.buffer
		, .offset = storage.range * imageIndex
		, .range  = storage.range };
		pDescriptorWrites [DescriptorStorage] = (VkWriteDescriptorSet)
		{ .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
		, .pNext            = NULL
		, .dstSet           = pDescriptorSets[DescriptorStorage]
		, .dstBinding       = BindingStorage
		, .dstArrayElement  = 0
		, .descriptorCount  = 1
		, .descriptorType   = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
		, .pImageInfo       = NULL
		, .pBufferInfo      = &pBufferInfos[DescriptorStorage]
		, .pTexelBufferView = NULL };
	}

	vkUpdateDescriptorSets( device
	                      , ArraySize(pDescriptorWrites), pDescriptorWrites
	                      , 0, NULL );
}

#define AttachmentColor   0
#define AttachmentResolve 1
#define AttachmentDepth   2
#define AttachmentCount   3

#define Subpass      0
#define SubpassCount 1

static void tqCreateRenderPass
	( VkDevice               const device
	, VkSampleCountFlagBits  const samples
	, VkRenderPass         * const pRenderPass )
{
	vkCreateRenderPass(device, &(VkRenderPassCreateInfo)
	{ .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO
	, .pNext           = NULL
	, .flags           = 0
	, .attachmentCount = AttachmentCount
	, .pAttachments    = (VkAttachmentDescription[AttachmentCount])
		{ [AttachmentColor] =
		  { .flags          = 0
		  , .format         = SwapchainImageFormat
		  , .samples        = samples
		  , .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR
		  , .storeOp        = VK_ATTACHMENT_STORE_OP_STORE
		  , .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE
		  , .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE
		  , .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED
		  , .finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }
		, [AttachmentResolve] =
		  { .flags          = 0
		  , .format         = SwapchainImageFormat
		  , .samples        = VK_SAMPLE_COUNT_1_BIT
		  , .loadOp         = VK_ATTACHMENT_LOAD_OP_DONT_CARE
		  , .storeOp        = VK_ATTACHMENT_STORE_OP_STORE
		  , .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE
		  , .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE
		  , .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED
		  , .finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR }
		, [AttachmentDepth] =
		  { .flags          = 0
		  , .format         = VK_FORMAT_D32_SFLOAT
		  , .samples        = samples
		  , .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR
		  , .storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE
		  , .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE
		  , .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE
		  , .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED
		  , .finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL } }
	, .subpassCount    = SubpassCount
	, .pSubpasses      = (VkSubpassDescription[SubpassCount])
		{ [Subpass] =
		  { .flags                   = 0
		  , .pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS
		  , .inputAttachmentCount    = 0
		  , .pInputAttachments       = NULL
		  , .colorAttachmentCount    = 1
		  , .pColorAttachments       = (VkAttachmentReference[1])
		  	{ { .attachment = AttachmentColor
		  	  , .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL } }
		  , .pResolveAttachments     = (VkAttachmentReference[1])
		  	{ { .attachment = AttachmentResolve
		  	  , .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL } }
		  , .pDepthStencilAttachment = &(VkAttachmentReference)
		  	{ .attachment = AttachmentDepth
		  	, .layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL }
		  , .preserveAttachmentCount = 0
		  , .pPreserveAttachments    = NULL } }
	, .dependencyCount = 0
	, .pDependencies   = NULL
	}, NULL, pRenderPass);
}

static void tqCreatePipeline
	( VkDevice               const device
	, VkSampleCountFlagBits  const rasterizationSamples
	, VkPipelineLayout       const layout
	, VkRenderPass           const renderPass
	, VkShaderModule         const moduleVertex
	, VkShaderModule         const moduleFragment
	, VkPipeline           * const pPipeline )
{
	vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &(VkGraphicsPipelineCreateInfo)
	{ .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO
	, .pNext               = NULL
	, .flags               = 0
	, .stageCount          = 2
	, .pStages = (VkPipelineShaderStageCreateInfo[2])
		{ { .sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO
		  , .pNext               = NULL
		  , .flags               = 0
		  , .stage               = VK_SHADER_STAGE_VERTEX_BIT
		  , .module              = moduleVertex
		  , .pName               = "main"
		  , .pSpecializationInfo = NULL
		  }
		, { .sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO
		  , .pNext               = NULL
		  , .flags               = 0
		  , .stage               = VK_SHADER_STAGE_FRAGMENT_BIT
		  , .module              = moduleFragment
		  , .pName               = "main"
		  , .pSpecializationInfo = NULL } }
	, .pVertexInputState   = &(VkPipelineVertexInputStateCreateInfo)
		{ .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
		, .pNext                           = NULL
		, .flags                           = 0
		, .vertexBindingDescriptionCount   = 0
		, .pVertexBindingDescriptions      = NULL
		, .vertexAttributeDescriptionCount = 0
		, .pVertexAttributeDescriptions    = NULL }
	, .pInputAssemblyState = &(VkPipelineInputAssemblyStateCreateInfo)
		{ .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO
		, .pNext                  = NULL
		, .flags                  = 0
		, .topology               = QuadTopology
		, .primitiveRestartEnable = VK_FALSE }
	, .pTessellationState = NULL
	, .pViewportState     = &(VkPipelineViewportStateCreateInfo)
		{ .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO
		, .pNext         = NULL
		, .flags         = 0
		, .viewportCount = 1
		// NOTE VK_DYNAMIC_STATE_VIEWPORT
		, .pViewports    = NULL
		, .scissorCount  = 1
		// NOTE VK_DYNAMIC_STATE_SCISSOR
		, .pScissors     = NULL }
	, .pRasterizationState = &(VkPipelineRasterizationStateCreateInfo)
		{ .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO
		, .pNext                   = NULL
		, .flags                   = 0
		, .depthClampEnable        = VK_FALSE
		, .rasterizerDiscardEnable = VK_FALSE
		, .polygonMode             = enabledFeatures.fillModeNonSolid ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL
		, .cullMode                = VK_CULL_MODE_NONE
		, .frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE
		, .depthBiasEnable         = VK_FALSE
		, .depthBiasConstantFactor = 0.f
		, .depthBiasClamp          = 0.f
		, .depthBiasSlopeFactor    = 0.f
		, .lineWidth               = enabledFeatures.wideLines ? 10.f : 1.f }
	, .pMultisampleState   = &(VkPipelineMultisampleStateCreateInfo)
		{ .sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO
		, .pNext                 = NULL
		, .flags                 = 0
		, .rasterizationSamples  = rasterizationSamples
		, .sampleShadingEnable   = enabledFeatures.sampleRateShading
		, .minSampleShading      = 1.f
		, .pSampleMask           = NULL
		, .alphaToCoverageEnable = VK_FALSE
		, .alphaToOneEnable      = VK_FALSE }
	, .pDepthStencilState  = &(VkPipelineDepthStencilStateCreateInfo)
		{ .sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO
		, .pNext                 = NULL
		, .flags                 = 0
		, .depthTestEnable       = VK_TRUE
		, .depthWriteEnable      = VK_TRUE
		, .depthCompareOp        = VK_COMPARE_OP_LESS
		, .depthBoundsTestEnable = VK_FALSE
		, .stencilTestEnable     = VK_FALSE
		, .front                 =
			{ .failOp      = VK_STENCIL_OP_KEEP
			, .passOp      = VK_STENCIL_OP_KEEP
			, .depthFailOp = VK_STENCIL_OP_KEEP
			, .compareOp   = VK_COMPARE_OP_NEVER
			, .compareMask = 0
			, .writeMask   = 0
			, .reference   = 0 }
		, .back                  =
			{ .failOp      = VK_STENCIL_OP_KEEP
			, .passOp      = VK_STENCIL_OP_KEEP
			, .depthFailOp = VK_STENCIL_OP_KEEP
			, .compareOp   = VK_COMPARE_OP_NEVER
			, .compareMask = 0
			, .writeMask   = 0
			, .reference   = 0 }
		, .minDepthBounds        = 0.f
		, .maxDepthBounds        = 1.f }
	, .pColorBlendState    = &(VkPipelineColorBlendStateCreateInfo)
		{ .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO
		, .pNext           = NULL
		, .flags           = 0
		, .logicOpEnable   = VK_FALSE
		, .logicOp         = VK_LOGIC_OP_CLEAR
		, .attachmentCount = 1
		, .pAttachments    = (VkPipelineColorBlendAttachmentState[1])
			{ { .blendEnable         = VK_TRUE
			  , .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA
			  , .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
			  , .colorBlendOp        = VK_BLEND_OP_ADD
			  , .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE
			  , .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO
			  , .alphaBlendOp        = VK_BLEND_OP_ADD
			  , .colorWriteMask      = VK_COLOR_COMPONENT_R_BIT
			                         | VK_COLOR_COMPONENT_G_BIT
			                         | VK_COLOR_COMPONENT_B_BIT
			                         | VK_COLOR_COMPONENT_A_BIT
			  } }
		, .blendConstants  = { 0.f, 0.f, 0.f, 0.f } } // RGBA
	, .pDynamicState       = &(VkPipelineDynamicStateCreateInfo)
		{ .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO
		, .pNext             = NULL
		, .flags             = 0
		, .dynamicStateCount = 2
		, .pDynamicStates    = (VkDynamicState[2])
			{ VK_DYNAMIC_STATE_VIEWPORT
			, VK_DYNAMIC_STATE_SCISSOR } }
	, .layout              = layout
	, .renderPass          = renderPass
	, .subpass             = Subpass
	, .basePipelineHandle  = VK_NULL_HANDLE
	, .basePipelineIndex   = -1
	}, NULL, pPipeline);
}

typedef struct TqSwapchain {
	VkExtent2D      imageExtent   ;
	TqImage         colorImage    ;
	TqImage         depthImage    ;
	VkSwapchainKHR  swapchain     ;
	VkImageView   * pImageViews   ;
	VkFramebuffer * pFramebuffers ;
} TqSwapchain;

static void tqCreateSwapchain
	( VkSurfaceKHR           const surface
	, VkPhysicalDevice       const physicalDevice
	, VkDevice               const device
	, VkSampleCountFlagBits  const samples
	, VkRenderPass           const renderPass
	, TqSwapchain          * const pSwapchain )
{
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR( physicalDevice
	                                         , surface
	                                         , &surfaceCapabilities );

	if ( SwapchainImageCount
	   < surfaceCapabilities.minImageCount )
		fail( "SwapchainImageCount must be at least %u"
		    , surfaceCapabilities.minImageCount );

	if ( surfaceCapabilities.maxImageCount
	  && SwapchainImageCount
	   > surfaceCapabilities.maxImageCount )
		fail( "SwapchainImageCount must be at most %u"
		    , surfaceCapabilities.maxImageCount );

/* Quote vkspec.html

"On Wayland, currentExtent is the special value (0xFFFFFFFF, 0xFFFFFFFF), indicating that the surface size will be determined by the extent of a swapchain targeting the surface. Whatever the application sets a swapchain’s imageExtent to will be the size of the window, after the first image is presented. minImageExtent is (1,1), and maxImageExtent is the maximum supported surface size."

"With [Win32/Xcb/Xlib], minImageExtent, maxImageExtent, and currentExtent must always equal the window size.
The currentExtent of a [Win32/Xcb/Xlib] surface must have both width and height greater than 0, or both of them 0.
The window size may become (0, 0) on this platform (e.g. when the window is minimized), and so a swapchain cannot be created until the size changes."
*/
	VkExtent2D imageExtent;
	     if ( surfaceCapabilities.minImageExtent.width  <= frame.fbw
	       && surfaceCapabilities.minImageExtent.height <= frame.fbh
	       && surfaceCapabilities.maxImageExtent.width  >= frame.fbw
	       && surfaceCapabilities.maxImageExtent.height >= frame.fbh )
		imageExtent = (VkExtent2D)
			{ .width  = frame.fbw
			, .height = frame.fbh };
	else if ( surfaceCapabilities.currentExtent.width  != UINT32_MAX
	       && surfaceCapabilities.currentExtent.height != UINT32_MAX )
		imageExtent = surfaceCapabilities.currentExtent;
	else fail("Cannot create swapchain with current framebuffer size");

	VkSwapchainKHR swapchain;
	vkCreateSwapchainKHR(device, &(VkSwapchainCreateInfoKHR)
	{ .sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR
	, .pNext                 = NULL
	, .flags                 = 0
	, .surface               = surface
	, .minImageCount         = SwapchainImageCount
	, .imageFormat           = SwapchainImageFormat
	, .imageColorSpace       = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
	, .imageExtent           = imageExtent
	, .imageArrayLayers      = 1
	, .imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
	, .imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE
	, .queueFamilyIndexCount = 0
	, .pQueueFamilyIndices   = NULL
	, .preTransform          = surfaceCapabilities.currentTransform
	, .compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR
	, .presentMode           = SwapchainPresentMode
	, .clipped               = VK_TRUE
	, .oldSwapchain          = VK_NULL_HANDLE
	}, NULL, &swapchain);
	info("Created swapchain with extent %ux%u\n", imageExtent.width, imageExtent.height);

	uint32_t swapchainImageCount = 0;
	vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, NULL);
	if (SwapchainImageCount != swapchainImageCount)
		fail( "Requested %u images but got %u images instead"
		    , SwapchainImageCount
		    , swapchainImageCount );
	VkImage pSwapchainImages[SwapchainImageCount];
	vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, pSwapchainImages);

	VkExtent3D extent =
		{ .width  = imageExtent.width
		, .height = imageExtent.height
		, .depth  = 1 };

	TqImage colorImage;
	tqCreateImage
		( device
		, SwapchainImageFormat
		, extent
		, samples
		, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
		| VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT
		, VK_IMAGE_ASPECT_COLOR_BIT
		, &colorImage );

	TqImage depthImage;
	tqCreateImage
		( device
		, VK_FORMAT_D32_SFLOAT
		, extent
		, samples
		, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
		| VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT
		, VK_IMAGE_ASPECT_DEPTH_BIT
		, &depthImage );

	static VkImageView   pImageViews   [SwapchainImageCount];
	static VkFramebuffer pFramebuffers [SwapchainImageCount];
	ForImageIndex {
		vkCreateImageView(device, &(VkImageViewCreateInfo)
		{ .sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO
		, .pNext            = NULL
		, .flags            = 0
		, .image            = pSwapchainImages[imageIndex]
		, .viewType         = VK_IMAGE_VIEW_TYPE_2D
		, .format           = SwapchainImageFormat
		, .components       =
			{ .r = VK_COMPONENT_SWIZZLE_IDENTITY
			, .g = VK_COMPONENT_SWIZZLE_IDENTITY
			, .b = VK_COMPONENT_SWIZZLE_IDENTITY
			, .a = VK_COMPONENT_SWIZZLE_IDENTITY }
		, .subresourceRange =
			{ .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT
			, .baseMipLevel   = 0
			, .levelCount     = 1
			, .baseArrayLayer = 0
			, .layerCount     = 1 }
		}, NULL, &pImageViews[imageIndex]);
		vkCreateFramebuffer(device, &(VkFramebufferCreateInfo)
		{ .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO
		, .pNext           = NULL
		, .flags           = 0
		, .renderPass      = renderPass
		, .attachmentCount = AttachmentCount
		, .pAttachments    = (VkImageView[AttachmentCount])
			{ [ AttachmentColor   ] = colorImage.imageView
			, [ AttachmentResolve ] = pImageViews[imageIndex]
			, [ AttachmentDepth   ] = depthImage.imageView }
		, .width           = imageExtent.width
		, .height          = imageExtent.height
		, .layers          = 1
		}, NULL, &pFramebuffers[imageIndex]);
	}

	*pSwapchain = (TqSwapchain)
		{ imageExtent
		, colorImage
		, depthImage
		, swapchain
		, pImageViews
		, pFramebuffers };
}

static void tqDestroySwapchain (VkDevice device, TqSwapchain swapchain) {
	ForImageIndex {
		vkDestroyImageView(device, swapchain.pImageViews[imageIndex], NULL);
		vkDestroyFramebuffer(device, swapchain.pFramebuffers[imageIndex], NULL);
	}
	vkDestroySwapchainKHR(device, swapchain.swapchain, NULL);
	tqDestroyImage(device, swapchain.depthImage);
	tqDestroyImage(device, swapchain.colorImage);
}

#define Txt txts[imageIndex]
static struct tqtxt txts[SwapchainImageCount] = { 0 };

static inline void tqRecordCommandBuffer
	( VkCommandBuffer         const commandBuffer
	, VkRenderPass            const renderPass
	, VkFramebuffer           const framebuffer
	, VkExtent2D              const extent
	, VkPipeline              const pipeline
	, VkPipelineLayout        const layout
	, VkDescriptorSet const * const pDescriptorSets )
{
	vkResetCommandBuffer(commandBuffer, 0);
	vkBeginCommandBuffer(commandBuffer, &(VkCommandBufferBeginInfo)
	{ .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
	, .pNext            = NULL
	, .flags            = 0
	, .pInheritanceInfo = NULL });
	vkCmdBeginRenderPass(commandBuffer, &(VkRenderPassBeginInfo)
	{ .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO
	, .pNext           = NULL
	, .renderPass      = renderPass
	, .framebuffer     = framebuffer
	, .renderArea      =
		{ .offset = { .x = 0, .y = 0 }
		, .extent = extent }
	, .clearValueCount = AttachmentCount
	, .pClearValues    = (VkClearValue[AttachmentCount])
		{ [AttachmentColor] =
		  { .color = { 0.f, 0.f, 0.f, 1.f } } // RGBA
		, [AttachmentResolve] =
		  { .color = { 0.f, 0.f, 0.f, 1.f } } // RGBA
		, [AttachmentDepth] =
		  { .depthStencil = { .depth = 1.f, .stencil = 0 } } }
	}, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	vkCmdBindDescriptorSets
		( commandBuffer
		, VK_PIPELINE_BIND_POINT_GRAPHICS
		, layout
		, 0
		, SetLayoutCount, (VkDescriptorSet[SetLayoutCount])
			{ pDescriptorSets[DescriptorUniform]
			, pDescriptorSets[DescriptorStorage]
			, pDescriptorSets[DescriptorFont] }
		, 0, NULL );
	vkCmdSetViewport(commandBuffer, 0, 1, (VkViewport[1])
	{ { .x        = 0.f
	  , .y        = 0.f
	  , .width    = extent.width
	  , .height   = extent.height
	  , .minDepth = 0.f
	  , .maxDepth = 1.f } });
	vkCmdSetScissor(commandBuffer, 0, 1, (VkRect2D[1])
	{ { .offset = { .x = 0, .y = 0 }
	  , .extent = extent } });
	vkCmdDraw(commandBuffer, QuadVertices, Txt.quads, 0, 0);
	vkCmdEndRenderPass(commandBuffer);
	vkEndCommandBuffer(commandBuffer);
}

struct tqtxt txt;

static void tqRun
	( tqupdate              * const update
	, GLFWwindow            * const window
	, VkSurfaceKHR            const surface
	, VkPhysicalDevice        const physicalDevice
	, VkDevice                const device
	, VkQueue                 const queue
	, VkCommandPool           const commandPool
	, VkPipelineLayout        const layout
	, VkDescriptorSet const * const pDescriptorSets
	, VkSampleCountFlagBits   const samples
	, VkRenderPass            const renderPass
	, VkPipeline              const pipeline
	, TqSwapchain           * const pSwapchain )
{
	VkCommandBuffer pCommandBuffers[SwapchainImageCount];
	vkAllocateCommandBuffers(device, &(VkCommandBufferAllocateInfo)
	{ .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO
	, .pNext              = NULL
	, .commandPool        = commandPool
	, .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY
	, .commandBufferCount = SwapchainImageCount
	}, pCommandBuffers);

	VkFence pWaitFences[2];
	VkFence * const pAcquireFence = &pWaitFences[0];
	VkFence * const  pSubmitFence = &pWaitFences[1];
	vkCreateFence(device, &(VkFenceCreateInfo)
	{ .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO
	, .pNext = NULL
	, .flags = 0
	}, NULL, pAcquireFence);

	VkFence     pFences     [SwapchainImageCount];
	VkSemaphore pSemaphores [SwapchainImageCount];
	ForImageIndex {
		vkCreateFence(device, &(VkFenceCreateInfo)
		{ .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO
		, .pNext = NULL
		, .flags = VK_FENCE_CREATE_SIGNALED_BIT
		}, NULL, &pFences[imageIndex]);
		vkCreateSemaphore(device, &(VkSemaphoreCreateInfo)
		{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
		, .pNext = NULL
		, .flags = 0
		}, NULL, &pSemaphores[imageIndex]);
	}

	VkSubmitInfo submitInfo =
	{ .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO
	, .pNext                = NULL
	, .waitSemaphoreCount   = 0
	, .pWaitSemaphores      = NULL
	, .pWaitDstStageMask    = NULL
	, .commandBufferCount   = 1
	, .signalSemaphoreCount = 1 };
	VkResult result;
	VkPresentInfoKHR presentInfo =
	{ .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR
	, .pNext              = NULL
	, .waitSemaphoreCount = 1
	, .swapchainCount     = 1
	, .pSwapchains        = &pSwapchain->swapchain
	, .pImageIndices      = &imageIndex
	, .pResults           = &result };

	info("Starting timer and entering loop\n");

	TQfun.resize = true;
	TQfun.done = false;

	double prt;       // previous real time
	bool   recreated; // prevent swapchain being recreated twice
	glfwSetTime(frame.rt);
	for (;;) {
		recreated = false;
		glfwPollEvents();
		if (glfwWindowShouldClose(window)) break;

		frame.resized = TQfun.resize;
		if (TQfun.resize) TQfun.resize = false;

		result = vkAcquireNextImageKHR
			( device
			, pSwapchain->swapchain
			, UINT64_MAX
			, VK_NULL_HANDLE
			, *pAcquireFence
			, &imageIndex );
		if ( result == VK_ERROR_OUT_OF_DATE_KHR )
			goto RecreateSwapchain;
		else
		if ( result != VK_SUCCESS
		  && result != VK_SUBOPTIMAL_KHR )
			fail("Unable to acquire next image (VkResult %d)", result);

		tqInputPoll(window);

		frame.imageIndex = imageIndex;
		txt = Txt;
		if (update()) { TQfun.done = true; goto ResetFences; }

		++frame.i;
		prt = frame.rt;
		frame.rt = glfwGetTime();
		frame.rdt = frame.rt - prt;
		if ( frame.dtmax
		  && frame.dtmax < frame.rdt )
			frame.dt = frame.dtmax;
		else
			frame.dt = frame.rdt;

		// just a little bit of silly logical timer code for fun
		// 1e7 still causes FE_INEXACT with usual range of dt, but good enough lol
		if (frame.ts > 1e7) frame.ts = 1e7;
		frame.dt *= frame.ts;

		feclearexcept(FE_INEXACT);
		frame.t += frame.dt;
		if (fetestexcept(FE_INEXACT)) {
			while (                   frame.t > 1e8 ) frame.t -= 1e8, frame.it += 1e8;
			while ( frame.dt < 1e7 && frame.t > 1e7 ) frame.t -= 1e7, frame.it += 1e7;
			while ( frame.dt < 1e6 && frame.t > 1e6 ) frame.t -= 1e6, frame.it += 1e6;
			while ( frame.dt < 1e5 && frame.t > 1e5 ) frame.t -= 1e5, frame.it += 1e5;
			while ( frame.dt < 1e4 && frame.t > 1e4 ) frame.t -= 1e4, frame.it += 1e4;
			while ( frame.dt < 1e3 && frame.t > 1e3 ) frame.t -= 1e3, frame.it += 1e3;
			while ( frame.dt < 1e2 && frame.t > 1e2 ) frame.t -= 1e2, frame.it += 1e2;
			while ( frame.dt < 1e1 && frame.t > 1e1 ) frame.t -= 1e1, frame.it += 1e1;
		}

		tqDebugFps();

		if (txt.quads > Quads) {
			warn( "%u quads exceeds max of %d"
			      "\nAre you clobbering memory? Truncating read.\n"
			    , txt.quads
			    , Quads );
			txt.quads = Quads;
		}

		// Clear out old quad memory!
		if (Txt.quads > txt.quads)
			memset( txt.quad + txt.quads
			      , 0
			      , sizeof (txtquad) * (Txt.quads - txt.quads) );

		Txt.quads = txt.quads;

		ResetFences:
		*pSubmitFence = pFences[imageIndex];
		vkWaitForFences(device, ArraySize(pWaitFences), pWaitFences, VK_TRUE, UINT64_MAX);
		vkResetFences(device, ArraySize(pWaitFences), pWaitFences);

		if (TQfun.done) break;

		tqRecordCommandBuffer
			( pCommandBuffers[imageIndex]
			, renderPass
			, pSwapchain->pFramebuffers[imageIndex]
			, pSwapchain->imageExtent
			, pipeline
			, layout
			, pDescriptorSets );

		submitInfo.pCommandBuffers   = &pCommandBuffers [imageIndex];
		submitInfo.pSignalSemaphores = &pSemaphores     [imageIndex];

		if ( ( result = vkQueueSubmit(queue, 1, &submitInfo, *pSubmitFence) )
		  != VK_SUCCESS )
			fail("Unable to submit (VkResult %d)", result);

		presentInfo.pWaitSemaphores = &pSemaphores[imageIndex];
		vkQueuePresentKHR(queue, &presentInfo);
		if ( result == VK_ERROR_OUT_OF_DATE_KHR
		  || result == VK_SUBOPTIMAL_KHR ) {
			RecreateSwapchain:
			if (recreated) continue;
			else recreated = true;
			for (;;) {
				glfwPollEvents();
				if (glfwWindowShouldClose(window))
				{ TQfun.done = true; goto ResetFences; }
				if (frame.fbw && frame.fbh) break;
				else { info("Minimized..."); glfwWaitEvents(); }
			}
			vkDeviceWaitIdle(device);
			tqDestroySwapchain(device, *pSwapchain);
			tqCreateSwapchain
				( surface
				, physicalDevice
				, device
				, samples
				, renderPass
				, pSwapchain );
		}
		else
		if ( result != VK_SUCCESS )
			fail("Unable to present (VkResult %d)", result);

		if (frame.resized) { frame.resized = false; goto RecreateSwapchain; }
	}

	info("Cleaning up...");
	vkDeviceWaitIdle(device);
	ForImageIndex {
		vkDestroyFence(device, pFences[imageIndex], NULL);
		vkDestroySemaphore(device, pSemaphores[imageIndex], NULL);
	}
	vkDestroyFence(device, *pAcquireFence, NULL);
}

void tqstart (tqupdate update) {
	GLFWwindow * window;
	tqCreateWindow(&window);
	tqInputInit(window);

	VkInstance instance;
	tqCreateInstance
		( config.title
		, config.version
		, &instance );

	VkSurfaceKHR surface;
	glfwCreateWindowSurface
		( instance
		, window
		, NULL
		, &surface );

	VkPhysicalDevice           physicalDevice;
	VkPhysicalDeviceProperties properties;
	VkDevice                   device;
	VkQueue                    queue;
	tqCreateDevice
		( instance
		, surface
		, &physicalDevice
		, &properties
		, &device
		, &queue );

	VkSampleCountFlagBits samples      = VK_SAMPLE_COUNT_1_BIT;
	VkSampleCountFlags    sampleCounts = properties.limits.framebufferColorSampleCounts
	                                   & properties.limits.framebufferDepthSampleCounts;

	info( "Multisampling supported sample count bits " PRIb8 "\n"
	    , _PRIb8(sampleCounts) );

	// get highest supported sample count (bad idea)
//	while (sampleCounts >>= 1) samples <<= 1;

	// Even with NVIDIA GTX 1050, let's not go overboard...
	if ( sampleCounts & VK_SAMPLE_COUNT_4_BIT
	  && properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU )
		samples = VK_SAMPLE_COUNT_4_BIT;

	// Intel UHD 620 pretty much tanks at more than 2 samples
	else
	if ( sampleCounts & VK_SAMPLE_COUNT_2_BIT )
		samples = VK_SAMPLE_COUNT_2_BIT;

	info("Multisampling using %u samples\n", samples);

	// uniform and storage are updated at the same rate.
	TqBufferRange uniform, storage;
	uniform.range = AlignSize( properties.limits.minUniformBufferOffsetAlignment
	                         , sizeof (m4) + sizeof (u32) );
	storage.range = AlignSize( properties.limits.minStorageBufferOffsetAlignment
	                         , sizeof (txtquad) * Quads );
	tqCreateBuffer( device
	              , uniform.range * SwapchainImageCount
	              , VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
	              , "uniform"
	              , &uniform.buffer );
	tqCreateBuffer( device
	              , storage.range * SwapchainImageCount
	              , VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
	              , "storage"
	              , &storage.buffer );
	ForImageIndex Txt = (struct tqtxt)
		{ .viewproj =      (m4 *) PtrAddBytes( uniform.buffer.pData
		                                     , uniform.range * imageIndex )
		, .invertY  =     (u32 *) PtrAddBytes( uniform.buffer.pData
		                                     , uniform.range * imageIndex + sizeof (m4) )
		, .quad     = (txtquad *) PtrAddBytes( storage.buffer.pData
	                                             , storage.range * imageIndex ) };

	VkCommandPool commandPool;
	vkCreateCommandPool(device, &(VkCommandPoolCreateInfo)
	{ .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO
	, .pNext            = NULL
	, .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
	, .queueFamilyIndex = QueueFamilyIndex
	}, NULL, &commandPool);

	TqFont font;
	tqCreateFont
		( device
		, properties.limits.maxSamplerAnisotropy
		, queue
		, commandPool
		, &font );

	VkDescriptorPool  descriptorPool;
	VkPipelineLayout  layout;
	VkDescriptorSet * pDescriptorSets;
	tqUpdateDescriptorSets
		( device
		, uniform
		, storage
		, font
		, &descriptorPool
		, &layout
		, &pDescriptorSets );

	VkRenderPass renderPass;
	tqCreateRenderPass(device, samples, &renderPass);

	TqShaderModule vert, frag;
	tqVertSPV(device, &vert);
	tqFragSPV(device, &frag);

	VkPipeline pipeline;
	tqCreatePipeline
		( device
		, samples
		, layout
		, renderPass
		, vert.shaderModule
		, frag.shaderModule
		, &pipeline );

	TqSwapchain swapchain;
	tqCreateSwapchain
		( surface
		, physicalDevice
		, device
		, samples
		, renderPass
		, &swapchain );

	tqRun
		( update
		, window
		, surface
		, physicalDevice
		, device
		, queue
		, commandPool
		, layout
		, pDescriptorSets
		, samples
		, renderPass
		, pipeline
		, &swapchain );

	tqDestroySwapchain           ( device  , swapchain      )        ;
	vkDestroyPipeline            ( device  , pipeline       , NULL ) ;
	tqDestroyShaderModule        ( device  , frag           )        ;
	tqDestroyShaderModule        ( device  , vert           )        ;
	vkDestroyRenderPass          ( device  , renderPass     , NULL ) ;
	vkDestroyPipelineLayout      ( device  , layout         , NULL ) ;
	for (size_t i = 0; i < ArraySize(pSetLayouts); ++i)
	vkDestroyDescriptorSetLayout ( device  , pSetLayouts[i] , NULL ) ;
//	vkFreeDescriptorSets        // all descriptor sets freed when descriptor pool is destroyed
	vkDestroyDescriptorPool      ( device  , descriptorPool , NULL ) ;
	tqDestroyFont                ( device  , font           )        ;
//	vkFreeCommandBuffers        // all command buffers freed when command pool is destroyed
	vkDestroyCommandPool         (device   , commandPool    , NULL ) ;
	tqDestroyBuffer              (device   , storage.buffer )        ;
	tqDestroyBuffer              (device   , uniform.buffer )        ;
	vkDestroyDevice              (device   , NULL           )        ;
	vkDestroySurfaceKHR          (instance , surface        , NULL ) ;
	vkDestroyInstance            (instance , NULL           )        ;

	glfwDestroyWindow(window);
	glfwTerminate();
	info("success\n");
}

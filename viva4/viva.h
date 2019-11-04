#pragma once

// Viva engine by Maciej Szpakowski

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <ctime>
#include <functional>

// image loading library
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// windows + vulkan
#include <windows.h>
#define VK_USE_PLATFORM_WIN32_KHR
#include "c:/VulkanSDK/1.1.108.0/Include/vulkan/vulkan.h"
//#pragma comment(linker, "/subsystem:windows")
#pragma comment(lib, "C:/VulkanSDK/1.1.108.0/Lib/vulkan-1.lib")

#define ENABLE_VALIDATION
#define VIDBG
#define UNIFORM_BUFFER_SIZE 1024 * 1024
#define INDIRECT_BUFFER_SIZE sizeof(VkDrawIndirectCommand)
#define TEXTURE_COUNT 40
#define KEYBOARD_KEY_COUNT 256
#define WND_CLASSNAME "mywindow"

typedef unsigned char byte;
typedef unsigned int uint;

namespace vi::memory
{
	const int MAX_HEAP_BLOCKS_COUNT = 100;

	struct heap
	{
		uint size;
		void* blocks[MAX_HEAP_BLOCKS_COUNT];
	};

	void heapInit(heap* h)
	{
		for (int i = 0; i < MAX_HEAP_BLOCKS_COUNT; i++)
			h->blocks[i] = nullptr;

		h->size = 0;
	}

	bool heapIsEmpty(heap* h)
	{
		for (int i = 0; i < MAX_HEAP_BLOCKS_COUNT; i++)
		{
			if (h->blocks[i] != nullptr)
				return false;
		}

		return true;
	}

	void* heapAlloc(heap* h, size_t size)
	{
#ifdef VIDBG
		if (h->size == MAX_HEAP_BLOCKS_COUNT)
		{
			fprintf(stderr, "Heap is full\n");
			exit(1);
		}
#endif

		void* block = malloc(size);
		h->blocks[h->size++] = block;
		return block;
	}

	void heapFree(heap* h, void* block)
	{
		for (int i = 0; i < MAX_HEAP_BLOCKS_COUNT; i++)
		{
			if (h->blocks[i] == block)
			{
				free(h->blocks[i]);
				h->blocks[i] = h->blocks[i];
				h->blocks[i] = h->blocks[--h->size];
				return;
			}
		}

#ifdef VIDBG
		fprintf(stderr, "Heap does not contain that block\n");
		exit(1);
#endif
	}

	// free all blocks
	void heapClear(heap* h)
	{
		for (int i = 0; i < MAX_HEAP_BLOCKS_COUNT; i++)
		{
			if (h->blocks[i] != nullptr)
			{
				free(h->blocks[i]);
				h->blocks[i] = nullptr;
			}
		}

		h->size = 0;
	}
}

namespace vi::time
{
    struct timer
    {
        float gameTime;
        float frameTime;
        long long ticksPerSecond;
        long long startTime;
        long long prevTick;
    };

    void initTimer(timer* t)
    {
        t->gameTime = 0;
        t->frameTime = 0;
        LARGE_INTEGER li;
        BOOL result = ::QueryPerformanceFrequency(&li);

#ifdef VIDBG
        if (!result)
        {
            fprintf(stderr, "QueryPerformanceFrequency failed");
            exit(1);
        }
#endif

        t->ticksPerSecond = li.QuadPart;
        ::QueryPerformanceCounter(&li);
        t->startTime = li.QuadPart;
        t->prevTick = li.QuadPart;
    }

    // this updates the timer so it must be called once per frame
    void updateTimer(timer* t)
    {
        LARGE_INTEGER currentTime;
        ::QueryPerformanceCounter(&currentTime);

        long long frameDelta = currentTime.QuadPart - t->prevTick;
        long long gameDelta = currentTime.QuadPart - t->startTime;
        t->prevTick = currentTime.QuadPart;
        t->frameTime = (float)((double)frameDelta / (double)t->ticksPerSecond);
        t->gameTime = (float)((double)gameDelta / (double)t->ticksPerSecond);
    }

    // get frame time in seconds
    float getFrameTimeSec(timer* t)
    {
        return t->frameTime;
    }

    // get time since game started in seconds
    float getGameTimeSec(timer* t)
    {
        return t->gameTime;
    }
}

namespace vi::util
{
	size_t getFileSize(FILE* file)
	{
		size_t retval = 0;
		fseek(file, SEEK_SET, 0);

		while (!feof(file))
		{
			fgetc(file);
			retval++;
		}

		fseek(file, SEEK_SET, 0);

		return retval;
	}

	byte* readFile(const char* filename, vi::memory::heap* h, size_t* outSize)
	{
		FILE* file = fopen(filename, "rb");

#ifdef VIDBG
		if (!file)
		{
			fprintf(stderr, "Could not open %s\n", filename);
			exit(1);
		}
#endif

		size_t size = getFileSize(file);
		byte* block = (byte*)vi::memory::heapAlloc(h, size + 1);
		size_t it = 0;

		while (true)
		{
			int c = fgetc(file);

			if (c == EOF)
				break;

			block[it++] = c;
		}

		fclose(file);
		block[it++] = 0;

        if (outSize != nullptr)
            * outSize = size - 1;

		return block;
	}
}

namespace vi::math
{
    const float PI = 3.1415926f;
    const float TWO_PI = PI * 2;
    const float HALF_PI = PI / 2;
    const float THIRD_PI = PI / 3;
    const float FORTH_PI = PI / 4;
    const float DEGREE = PI / 180;
}

// win32vk
namespace vi::graphics
{
    struct camera
    {
        float aspectRatio;
        float x;
        float y;
        float rotation;
        float scale;
        byte padding[12];
    };

	struct engine
	{
		HINSTANCE hinstance;
		HWND window;
        VkInstance instance;
        VkDevice device;
        VkPhysicalDeviceMemoryProperties memProperties;
        VkCommandPool commandPool;
        VkQueue queue;
        VkCommandBuffer drawCommands[10];
        VkSwapchainKHR swapChain;
        VkSemaphore imageAvailableSemaphore;
        VkSemaphore renderFinishedSemaphore;
        uint frameBufferCount;
        uint currentFrameIndex;
        VkBuffer uniformBuffer;
        VkDeviceMemory uniformBufferMemory;
        VkBuffer drawIndirectBuffer;
        VkDeviceMemory drawIndirectBufferMemory;
        // tutorial used 3 sets (1 per frame) but since I only render one frame at a time
        // create only 1 set
        VkDescriptorSet descriptorSet;
        VkRenderPass renderPass;
        VkPipeline graphicsPipeline;
        VkPipelineLayout pipelineLayout;
        VkFramebuffer swapChainFramebuffers[10];
        VkExtent2D swapChainExtent;
        VkImage depthBuffer;
        VkDeviceMemory depthBufferMemory;
        VkImageView depthBufferView;
        VkSurfaceKHR surface;
        VkImageView swapChainImageViews[10];
        VkDescriptorSetLayout descriptorSetLayout;
        VkSampler textureSampler;
        VkDescriptorPool descriptorPool;
	};

	struct engineInfo
	{
		uint width;
		uint height;
		const char* title;
	};

    // struct passed to uniform buffer must be multiple of 16bytes
    struct drawInfo
    {
        // transform
        float x;
        float y;
        float z;
        float sx;
        float sy;
        float rot;
        float ox;
        float oy;
        // uv, color, texture
        int textureIndex;
        float left;
        float top;
        float right;
        float bottom;
        float r;
        float g;
        float b;
    };

    struct texture
    {
        int index;
        int width;
        int height;
        VkImage image;
        VkImageView imageView;
        VkDeviceMemory memory;
    };

	LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
        case WM_SYSKEYDOWN:
        {
            if (wParam == VK_MENU)//ignore left alt stop
            {
            }
            else
                return DefWindowProc(hwnd, uMsg, wParam, lParam); // this makes ALT+F4 work
            break;
        }
		case WM_CLOSE:
		{
			PostQuitMessage(0);
			break;
		}
		default:
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}

		return 0;
	}

	void _win32InitWindow(engine* g, engineInfo* info)
	{
		g->hinstance = GetModuleHandle(0);
		HBRUSH bg = CreateSolidBrush(RGB(255, 0, 0));
		uint32_t width = info->width;
		uint32_t height = info->height;

		WNDCLASS wc = { };
		ZeroMemory(&wc, sizeof(WNDCLASS));
		wc.lpfnWndProc = WindowProc;
		wc.hInstance = g->hinstance;
		wc.lpszClassName = WND_CLASSNAME;
		wc.hbrBackground = bg;
		RegisterClass(&wc);

		DWORD wndStyle = WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX;
		RECT r = { 0, 0, (LONG)width, (LONG)height };
		// this tells you what should be the window size if r is rect for client
		// IMPORTANT. window client, swap chain and VkImages (render target) dimensions must match
		AdjustWindowRect(&r, wndStyle, false);
		g->window = CreateWindowEx(0, WND_CLASSNAME, info->title, wndStyle, 100, 100,
			r.right - r.left, r.bottom - r.top, 0, 0, g->hinstance, 0);
#ifdef VIDBG
		if (g->window == nullptr)
		{
			fprintf(stderr, "Failed to create window");
			exit(1);
		}
#endif
		ShowWindow(g->window, SW_SHOW);
	}

	void _vkCreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkDevice device, VkBuffer* buffer,
		VkMemoryPropertyFlags memoryFlags, VkPhysicalDeviceMemoryProperties memProperties, VkDeviceMemory* bufferMemory)
	{
		VkBufferCreateInfo stagingBufferInfo = {};
		stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		stagingBufferInfo.size = size;
		stagingBufferInfo.usage = usage;
		stagingBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		VkResult result = vkCreateBuffer(device, &stagingBufferInfo, nullptr, buffer);

#ifdef VIDBG
		if (result != VK_SUCCESS)
		{
			fprintf(stderr, "vkCreateBuffer failed\n");
			exit(1);
		}
#endif

		// memoryTypeBits is a bitmask and contains one bit set for every supported memory type for the resource. 
		// Bit i is set if and only if the memory type i in the VkPhysicalDeviceMemoryProperties structure for the physical device is supported for the resource
		VkMemoryRequirements memRequirementsForStagingBuffer;
		vkGetBufferMemoryRequirements(device, *buffer, &memRequirementsForStagingBuffer);

		uint32_t stagingBufferMemoryTypeIndex = -1;

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			// expression (A & B) == B
			// means that A must have at least all bits of B set (can have more)
			if ((memRequirementsForStagingBuffer.memoryTypeBits & (1 << i)) &&
				(memProperties.memoryTypes[i].propertyFlags & memoryFlags) == memoryFlags)
			{
				stagingBufferMemoryTypeIndex = i;
				break;
			}
		}

#ifdef VIDBG
		if (stagingBufferMemoryTypeIndex == -1)
		{
			fprintf(stderr, "Could not find required memory for vkAllocateMemory\n");
			exit(1);
		}
#endif

		VkMemoryAllocateInfo stagingBufferMemoryAllocInfo = {};
		stagingBufferMemoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		stagingBufferMemoryAllocInfo.allocationSize = memRequirementsForStagingBuffer.size;
		stagingBufferMemoryAllocInfo.memoryTypeIndex = stagingBufferMemoryTypeIndex;

		// WARNING: this call should be keept to minimum, allocate a bunch of memory at once and then use offset to use one chunk for multiple buffers
        // TODO make minimum allocs
		result = vkAllocateMemory(device, &stagingBufferMemoryAllocInfo, nullptr, bufferMemory);

#ifdef VIDBG
		if (result != VK_SUCCESS)
		{
			fprintf(stderr, "vkAllocateMemory failed\n");
			exit(1);
		}
#endif

		vkBindBufferMemory(device, *buffer, *bufferMemory, 0);
	}

    void _vkCreateImage(uint width, uint height, VkFormat format, VkBufferUsageFlags usage, VkDevice device, VkImage* image,
        VkMemoryPropertyFlags memoryFlags, VkPhysicalDeviceMemoryProperties memProperties, VkDeviceMemory* imageMemory)
    {
        VkImageCreateInfo imageCreateInfo = {};
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.extent.width = width;
        imageCreateInfo.extent.height = height;
        imageCreateInfo.extent.depth = 1;
        imageCreateInfo.mipLevels = 1;
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.format = format;
        imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageCreateInfo.usage = usage;
        imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;

        VkResult vkResult = vkCreateImage(device, &imageCreateInfo, nullptr, image);

#ifdef VIDBG
        if (vkResult != VK_SUCCESS)
        {
            fprintf(stderr, "vkCreateImage failed\n");
            exit(1);
        }
#endif

        VkMemoryRequirements memRequirementsForTextureImageMemory;
        vkGetImageMemoryRequirements(device, *image, &memRequirementsForTextureImageMemory);

        VkMemoryPropertyFlags desiredImageMemoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        uint32_t imageMemoryTypeIndex = -1;

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        {
            if ((memRequirementsForTextureImageMemory.memoryTypeBits & (1 << i)) &&
                (memProperties.memoryTypes[i].propertyFlags & desiredImageMemoryFlags) == desiredImageMemoryFlags)
            {
                imageMemoryTypeIndex = i;
                break;
            }
        }

#ifdef VIDBG
        if (imageMemoryTypeIndex == -1)
        {
            fprintf(stderr, "Could not find appropriate memory type for image\n");
            exit(1);
        }
#endif

        VkMemoryAllocateInfo textureImageMemoryAllocInfo = {};
        textureImageMemoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        textureImageMemoryAllocInfo.allocationSize = memRequirementsForTextureImageMemory.size;
        textureImageMemoryAllocInfo.memoryTypeIndex = imageMemoryTypeIndex;

        vkResult = vkAllocateMemory(device, &textureImageMemoryAllocInfo, nullptr, imageMemory);

        vkBindImageMemory(device, *image, *imageMemory, 0);
    }

	void _vkInitStep1(engine* g, engineInfo* info)
	{
		memory::heap helperHeap;
		memory::heapInit(&helperHeap);

        for (uint i = 0; i < 10; i++)
            g->drawCommands[i] = VK_NULL_HANDLE;

		/**************************************************************************
		VkInstance
		Purpose: check if vulkan driver is present and to query for physical devices
		it also creates win32 surface
		*/
		VkResult vkResult;
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;

		// all optional
		appInfo.pApplicationName = "VivaApp";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Viva";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		const char* ext[] = { "VK_KHR_surface", "VK_KHR_win32_surface" };
		const char* layers[] = { "VK_LAYER_KHRONOS_validation" };

		VkInstanceCreateInfo vkInstanceArgs = {};
		vkInstanceArgs.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		vkInstanceArgs.pApplicationInfo = &appInfo;
		vkInstanceArgs.enabledExtensionCount = 2;
		vkInstanceArgs.ppEnabledExtensionNames = ext;

		// enable this to see some diagnostic
		// requires vulkan 1.1.106 or higher, prints to stdout by default
#ifdef ENABLE_VALIDATION
		vkInstanceArgs.enabledLayerCount = 1;
		vkInstanceArgs.ppEnabledLayerNames = layers;
#endif

		vkResult = vkCreateInstance(&vkInstanceArgs, nullptr, &g->instance);

#ifdef VIDBG
		if (vkResult != VK_SUCCESS)
		{
			fprintf(stderr, "vkCreateInstance failed\n");
			exit(1);
		}
#endif

		/**************************************************************************
		Physical device and queue index
		Purpose: needed to create logical device and queue
		*/
		uint32_t queueIndex = -1;
		uint32_t gpuCount = 0;
		uint32_t queueFamilyCount = 0;
		VkPhysicalDeviceProperties gpuProperties;
		VkPhysicalDeviceFeatures gpuFeatures;
		VkPhysicalDevice gpus[10];
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		VkQueueFamilyProperties queueFamilies[10];

		vkEnumeratePhysicalDevices(g->instance, &gpuCount, nullptr);
		gpuCount = min(10, gpuCount);
		vkEnumeratePhysicalDevices(g->instance, &gpuCount, gpus);

		for (uint32_t i = 0; i < gpuCount; i++)
		{
			vkGetPhysicalDeviceProperties(gpus[i], &gpuProperties);
			vkGetPhysicalDeviceFeatures(gpus[i], &gpuFeatures);
			vkGetPhysicalDeviceQueueFamilyProperties(gpus[i], &queueFamilyCount, nullptr);
			queueFamilyCount = min(10, queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(gpus[i], &queueFamilyCount, queueFamilies);

			// check if VK_KHR_SWAPCHAIN_EXTENSION_NAME is supported
			uint32_t extensionCount;
			vkEnumerateDeviceExtensionProperties(gpus[i], nullptr, &extensionCount, nullptr);
			VkExtensionProperties* availableExtensions = (VkExtensionProperties*)memory::heapAlloc(&helperHeap, extensionCount * sizeof(VkExtensionProperties));

			vkEnumerateDeviceExtensionProperties(gpus[i], nullptr, &extensionCount, availableExtensions);

			bool swapChainSupported = false;
			for (uint32_t j = 0; j < extensionCount; j++)
			{
				if (strcmp(availableExtensions[j].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) != 0)
				{
					swapChainSupported = true;
					break;
				}
			}

			if (!swapChainSupported)
				continue;

			for (uint32_t j = 0; j < queueFamilyCount; j++)
			{
				// lets limit this to discrete GPU
				// queue must have graphics VK_QUEUE_GRAPHICS_BIT and present bit and 
				// VK_QUEUE_TRANSFER_BIT (its guaranteed that if graphics is supported then transfer is supported)
				if (gpuProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
					queueFamilies[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				{
					queueIndex = j;
					// physical device doesnt have to be created
					physicalDevice = gpus[i];
					break;
				}
			}

			if (physicalDevice != VK_NULL_HANDLE)
				break;
		}

#ifdef VIDBG
		if (queueIndex == -1)
		{
			fprintf(stderr, "Could not find required queue\n");
			exit(1);
		}
#endif

#ifdef VIDBG
		if (physicalDevice == VK_NULL_HANDLE)
		{
			fprintf(stderr, "Could not find satisfactory physical device\n");
			exit(1);
		}
#endif

		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &g->memProperties);

		/**************************************************************************
		Logical device and command queue
		Purpose: device needed for nearly everything in vulkan, queue needed to execute commands
		*/
		float queuePriority = 1.0f;

		// its possible that more than one queue is needed so its would require multiple VkDeviceQueueCreateInfo
		VkDeviceQueueCreateInfo queueArgs = {};
		queueArgs.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueArgs.queueFamilyIndex = queueIndex;
		queueArgs.queueCount = 1;
		queueArgs.pQueuePriorities = &queuePriority;

		VkPhysicalDeviceFeatures deviceFeatures = {};

		const char* extensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		VkDeviceCreateInfo deviceArgs = {};
		deviceArgs.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceArgs.pQueueCreateInfos = &queueArgs;
		deviceArgs.queueCreateInfoCount = 1;
		deviceArgs.pEnabledFeatures = &deviceFeatures;
		deviceArgs.enabledExtensionCount = 1;
		deviceArgs.ppEnabledExtensionNames = extensions;

		// create device creates logical device and all the queues
		vkResult = vkCreateDevice(physicalDevice, &deviceArgs, nullptr, &g->device);

#ifdef VIDBG
		if (vkResult != VK_SUCCESS)
		{
			fprintf(stderr, "vkCreateDevice failed\n");
			exit(1);
		}
#endif

		// queues are created, query for one found before (queueIndex)
		vkGetDeviceQueue(g->device, queueIndex, 0, &g->queue);

        /**************************************************************************
        Command pool
        */
        VkCommandPoolCreateInfo commandPoolCreateInfo = {};
        commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolCreateInfo.queueFamilyIndex = queueIndex;
        // this can be used to indicate that commands will be shortlived
        //commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

        vkResult = vkCreateCommandPool(g->device, &commandPoolCreateInfo, nullptr, &g->commandPool);

#ifdef VIDBG
        if (vkResult != VK_SUCCESS)
        {
            fprintf(stderr, "vkCreateCommandPool failed\n");
            exit(1);
        }
#endif

		/**************************************************************************
		Surface
		Purpose: to connect vulkan (more specifically swapchain) with window
		*/
		VkWin32SurfaceCreateInfoKHR surfaceArgs = {};
		surfaceArgs.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surfaceArgs.hinstance = g->hinstance;
		surfaceArgs.hwnd = g->window;

		vkResult = vkCreateWin32SurfaceKHR(g->instance, &surfaceArgs, nullptr, &g->surface);

#ifdef VIDBG
		if (vkResult != VK_SUCCESS)
		{
			fprintf(stderr, "vkCreateWin32SurfaceKHR failed\n");
			exit(1);
		}
#endif

		// check if the queue support presentation, its possbile that there is a different queue for this
		// for now, lets hope that selected queue supports it
		VkBool32 physicalDeviceSupportSurface = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueIndex, g->surface, &physicalDeviceSupportSurface);

#ifdef VIDBG
		if (physicalDeviceSupportSurface != VK_TRUE)
		{
			fprintf(stderr, "Insufficient device swapchain support\n");
			exit(1);
		}
#endif

		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, g->surface, &surfaceCapabilities);

		uint32_t formatCount;
		VkSurfaceFormatKHR surfaceFormats[300];
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, g->surface, &formatCount, nullptr);
		formatCount = min(300, formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, g->surface, &formatCount, surfaceFormats);

		// see if format available
		VkSurfaceFormatKHR surfaceFormat = {};
		for (uint32_t i = 0; i < formatCount; i++)
		{
			// this is the most optimal combination
			if (surfaceFormats[i].format == VK_FORMAT_B8G8R8A8_UNORM && surfaceFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				surfaceFormat = surfaceFormats[i];
				break;
			}
		}

#ifdef VIDBG
		if (surfaceFormat.format != VK_FORMAT_B8G8R8A8_UNORM)
		{
			fprintf(stderr, "Could not find optimal surface format\n");
			exit(1);
		}
#endif

		// VK_PRESENT_MODE_FIFO_KHR always exists
		VkPresentModeKHR surfacePresentationMode = VK_PRESENT_MODE_FIFO_KHR; // vsync;
		//VkPresentModeKHR surfacePresentationMode = VK_PRESENT_MODE_IMMEDIATE_KHR; // no vsync;

		/**************************************************************************
		Swap Chain
		Purpose: actually surface doesnt connect to the vulkan, swap chain does and
		surface connects to swap chain.
		Creating swapchain with vulkan creates VkImage as well which is actual vulkan object
		where vulkan renders pixels to
		*/
		g->swapChainExtent = { info->width ,info->height };
		// recommendation is minimum + 1
		uint32_t frameBufferCount = surfaceCapabilities.minImageCount + 1;
        g->frameBufferCount = frameBufferCount;

		// check if max is not exceeded
		if (surfaceCapabilities.maxImageCount > 0 && frameBufferCount > surfaceCapabilities.maxImageCount)
			frameBufferCount = surfaceCapabilities.maxImageCount;

		VkSwapchainCreateInfoKHR swapChainArgs = {};
		swapChainArgs.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapChainArgs.surface = g->surface;
		swapChainArgs.minImageCount = frameBufferCount;
		swapChainArgs.imageFormat = surfaceFormat.format;
		swapChainArgs.imageColorSpace = surfaceFormat.colorSpace;
		swapChainArgs.imageExtent = g->swapChainExtent;
		// this is 1 unless your render is more than 2D
		swapChainArgs.imageArrayLayers = 1;
		// idk what that is
		swapChainArgs.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		// this flag has best performance if there is only one queue
		swapChainArgs.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		// this needs to be set if there is more than one queue, e.g. one for graphics and one for present
		//swapChainArgs.queueFamilyIndexCount = 2;
		//swapChainArgs.pQueueFamilyIndices = queueFamilyIndices;
		// idk what that is
		swapChainArgs.preTransform = surfaceCapabilities.currentTransform;
		// idk what that is
		swapChainArgs.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapChainArgs.presentMode = surfacePresentationMode;
		// idk what that is
		swapChainArgs.clipped = VK_TRUE;

		vkResult = vkCreateSwapchainKHR(g->device, &swapChainArgs, nullptr, &g->swapChain);

#ifdef VIDBG
		if (vkResult != VK_SUCCESS)
		{
			fprintf(stderr, "vkCreateSwapchainKHR failed\n");
			exit(1);
		}
#endif

		// images were created with swapchain
		// count was specified above but vulkan might have created more images than that
#ifdef VIDBG
		if (frameBufferCount > 10)
		{
			fprintf(stderr, "Max frame buffer count is hardcoded to 10 but queried result is greater than 10\n");
			exit(1);
		}
#endif

		vkGetSwapchainImagesKHR(g->device, g->swapChain, &frameBufferCount, nullptr);
		VkImage swapChainImages[10];
		vkGetSwapchainImagesKHR(g->device, g->swapChain, &frameBufferCount, swapChainImages);

		/**************************************************************************
		VkImageView
		Purpose: some helper object for VkImage (used here as render target)
		*/

		for (size_t i = 0; i < frameBufferCount; i++)
		{
			VkImageViewCreateInfo imageViewArgs = {};
			imageViewArgs.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imageViewArgs.image = swapChainImages[i];
			imageViewArgs.viewType = VK_IMAGE_VIEW_TYPE_2D;
			imageViewArgs.format = surfaceFormat.format;
			imageViewArgs.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewArgs.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewArgs.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewArgs.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewArgs.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageViewArgs.subresourceRange.baseMipLevel = 0;
			imageViewArgs.subresourceRange.levelCount = 1;
			imageViewArgs.subresourceRange.baseArrayLayer = 0;
			imageViewArgs.subresourceRange.layerCount = 1;

			vkResult = vkCreateImageView(g->device, &imageViewArgs, nullptr, &g->swapChainImageViews[i]);

#ifdef VIDBG
			if (vkResult != VK_SUCCESS)
			{
				fprintf(stderr, "vkCreateImageView failed\n");
				exit(1);
			}
#endif
		}

		/************************************************************************************
		Descriptor Layout
		Purpose: idk, some helper object for descriptor
		*/
		VkDescriptorSetLayoutBinding uboLayoutBinding = {};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutBinding imageLayoutBinding = {};
        imageLayoutBinding.binding = 2;
        imageLayoutBinding.descriptorCount = TEXTURE_COUNT;
        imageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        imageLayoutBinding.pImmutableSamplers = nullptr;
        imageLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding descriptorSetBindings[] = { uboLayoutBinding, samplerLayoutBinding, imageLayoutBinding };
		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
		descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorSetLayoutCreateInfo.bindingCount = 3;
		descriptorSetLayoutCreateInfo.pBindings = descriptorSetBindings;

		vkResult = vkCreateDescriptorSetLayout(g->device, &descriptorSetLayoutCreateInfo, nullptr, &g->descriptorSetLayout);

#ifdef VIDBG
		if (vkResult != VK_SUCCESS)
		{
			fprintf(stderr, "vkCreateDescriptorSetLayout failed\n");
			exit(1);
		}
#endif

		/**************************************************************************
		Shaders
		*/
		size_t vsCodeSize;
		size_t psCodeSize;
		byte* vsCode = util::readFile("vert.spv", &helperHeap, &vsCodeSize);
		byte* psCode = util::readFile("frag.spv", &helperHeap, &psCodeSize);

		VkShaderModuleCreateInfo shaderCreateInfo = {};
		shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderCreateInfo.codeSize = vsCodeSize;
		// WARNING pCode is pointer to int so bytes must be aligned to 4byte
		shaderCreateInfo.pCode = (const uint32_t*)vsCode;

		VkShaderModule vsModule;
		vkResult = vkCreateShaderModule(g->device, &shaderCreateInfo, nullptr, &vsModule);

#ifdef VIDBG
		if (vkResult != VK_SUCCESS)
		{
			fprintf(stderr, "vkCreateShaderModule failed\n");
			exit(1);
		}
#endif

		shaderCreateInfo.codeSize = psCodeSize;
		// WARNING pCode is pointer to int so bytes must be aligned to 4byte
		shaderCreateInfo.pCode = (const uint32_t*)psCode;

		VkShaderModule psModule;
		vkResult = vkCreateShaderModule(g->device, &shaderCreateInfo, nullptr, &psModule);

#ifdef VIDBG
		if (vkResult != VK_SUCCESS)
		{
			fprintf(stderr, "vkCreateShaderModule failed\n");
			exit(1);
		}
#endif

		VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vsModule;
		vertShaderStageInfo.pName = "main";
		// you can set constants in shaders with this
		//vertShaderStageInfo.pSpecializationInfo = nullptr;

		VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = psModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

        /**************************************************************************
        Depth Buffer
        */
        _vkCreateImage(g->swapChainExtent.width, g->swapChainExtent.height, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            g->device, &g->depthBuffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, g->memProperties, &g->depthBufferMemory);

        VkImageViewCreateInfo depthBufferImageViewArgs = {};
        depthBufferImageViewArgs.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        depthBufferImageViewArgs.image = g->depthBuffer;
        depthBufferImageViewArgs.viewType = VK_IMAGE_VIEW_TYPE_2D;
        depthBufferImageViewArgs.format = VK_FORMAT_D32_SFLOAT;
        //depthBufferImageViewArgs.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        //depthBufferImageViewArgs.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        //depthBufferImageViewArgs.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        //depthBufferImageViewArgs.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        depthBufferImageViewArgs.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        depthBufferImageViewArgs.subresourceRange.baseMipLevel = 0;
        depthBufferImageViewArgs.subresourceRange.levelCount = 1;
        depthBufferImageViewArgs.subresourceRange.baseArrayLayer = 0;
        depthBufferImageViewArgs.subresourceRange.layerCount = 1;

        vkResult = vkCreateImageView(g->device, &depthBufferImageViewArgs, nullptr, &g->depthBufferView);

		/**************************************************************************
		Vertex Input
		*/
		/*VkVertexInputBindingDescription vertexInputBindingDescription = {};
		vertexInputBindingDescription.binding = 0;
		vertexInputBindingDescription.stride = 7 * sizeof(float);
		vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		VkVertexInputAttributeDescription vertexInputAttributeDescription[3] = { {},{}, {} };
		vertexInputAttributeDescription[0].binding = 0;
		vertexInputAttributeDescription[0].location = 0;
		vertexInputAttributeDescription[0].format = VK_FORMAT_R32G32_SFLOAT;
		vertexInputAttributeDescription[0].offset = 0;
		vertexInputAttributeDescription[1].binding = 0;
		vertexInputAttributeDescription[1].location = 1;
		vertexInputAttributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		vertexInputAttributeDescription[1].offset = sizeof(float) * 2;
		vertexInputAttributeDescription[2].binding = 0;
		vertexInputAttributeDescription[2].location = 2;
		vertexInputAttributeDescription[2].format = VK_FORMAT_R32G32_SFLOAT;
		vertexInputAttributeDescription[2].offset = sizeof(float) * 5;*/

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		/**************************************************************************
		Viewport
		*/
		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)g->swapChainExtent.width;
		viewport.height = (float)g->swapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = g->swapChainExtent;

		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		/**************************************************************************
		Rasterizer
		*/
		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

		/**************************************************************************
		Multisampling
		*/
		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		/**************************************************************************
		Blend
		*/
		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;

		/**************************************************************************
		Render pass and subpass
		*/
		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = surfaceFormat.format;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		// Optimal for presentation
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		// Optimal as attachment for writing colors from the fragment shader
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription depthAttachment = {};
        depthAttachment.format = VK_FORMAT_D32_SFLOAT;
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef = {};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        VkAttachmentDescription renderPassAttachments[] = { colorAttachment, depthAttachment };

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 2;
		renderPassInfo.pAttachments = renderPassAttachments;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		
		vkResult = vkCreateRenderPass(g->device, &renderPassInfo, nullptr, &g->renderPass);

#ifdef VIDBG
		if (vkResult != VK_SUCCESS)
		{
			fprintf(stderr, "vkCreateRenderPass failed\n");
			exit(1);
		}
#endif

		/**************************************************************************
		Pipeline
		*/
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &g->descriptorSetLayout;

		
		vkResult = vkCreatePipelineLayout(g->device, &pipelineLayoutInfo, nullptr, &g->pipelineLayout);

#ifdef VIDBG
		if (vkResult != VK_SUCCESS)
		{
			fprintf(stderr, "vkCreatePipelineLayout failed\n");
			exit(1);
		}
#endif

		VkPipelineInputAssemblyStateCreateInfo pipelineInputAssembly = {};
		pipelineInputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		pipelineInputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		pipelineInputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineDepthStencilStateCreateInfo depthStencil = {};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f; // Optional
        depthStencil.maxDepthBounds = 1.0f; // Optional
        depthStencil.stencilTestEnable = VK_FALSE;
        depthStencil.front = {}; // Optional
        depthStencil.back = {}; // Optional

		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &pipelineInputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.layout = g->pipelineLayout;
		pipelineInfo.renderPass = g->renderPass;
		pipelineInfo.subpass = 0;
        pipelineInfo.pDepthStencilState = &depthStencil;

		
		vkResult = vkCreateGraphicsPipelines(g->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &g->graphicsPipeline);

#ifdef VIDBG
		if (vkResult != VK_SUCCESS)
		{
			fprintf(stderr, "vkCreateGraphicsPipelines failed\n");
			exit(1);
		}
#endif

		// it can be destroyed now because modelues were compiled to machine code and stored in the pipeline
		vkDestroyShaderModule(g->device, psModule, nullptr);
		vkDestroyShaderModule(g->device, vsModule, nullptr);

		/**************************************************************************
		Frame buffer
		*/
		for (size_t i = 0; i < frameBufferCount; i++)
		{
            VkImageView frameBufferAttachments[] = { g->swapChainImageViews[i], g->depthBufferView };

			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = g->renderPass;
			framebufferInfo.attachmentCount = 2;
			framebufferInfo.pAttachments = frameBufferAttachments;
			framebufferInfo.width = g->swapChainExtent.width;
			framebufferInfo.height = g->swapChainExtent.height;
			framebufferInfo.layers = 1;

			vkResult = vkCreateFramebuffer(g->device, &framebufferInfo, nullptr, &g->swapChainFramebuffers[i]);

#ifdef VIDBG
			if (vkResult != VK_SUCCESS)
			{
				fprintf(stderr, "vkCreateFramebuffer failed\n");
				exit(1);
			}
#endif
		}

		/**************************************************************************
		Sampler
		*/
		VkSamplerCreateInfo samplerCreateInfo = {};
		samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.magFilter = VK_FILTER_NEAREST; // VK_FILTER_LINEAR;
		samplerCreateInfo.minFilter = VK_FILTER_NEAREST; // VK_FILTER_LINEAR;
		samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		// this doesnt matter in VK_SAMPLER_ADDRESS_MODE_REPEAT mode but it has to be specified (i think)
		samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
		// im not using that
		// you must check if pysical devices has this feature enabled
		//samplerCreateInfo.anisotropyEnable = VK_TRUE;
		//samplerCreateInfo.maxAnisotropy = 16;
		// unnormalizedCoordinates uses coordinates in pixels for texture instead of [0,1]
		samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
		// this is some advance feature
		samplerCreateInfo.compareEnable = VK_FALSE;
		samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		// dont care about mips
		samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		samplerCreateInfo.mipLodBias = 0.0f;
		samplerCreateInfo.minLod = 0.0f;
		samplerCreateInfo.maxLod = 0.0f;

		vkResult = vkCreateSampler(g->device, &samplerCreateInfo, nullptr, &g->textureSampler);

#ifdef VIDBG
		if (vkResult != VK_SUCCESS)
		{
			fprintf(stderr, "vkCreateSampler failed\n");
			exit(1);
		}
#endif

        /**************************************************************************
        Indirect draw buffer
        */
        _vkCreateBuffer(INDIRECT_BUFFER_SIZE, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, g->device, &g->drawIndirectBuffer,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, g->memProperties, &g->drawIndirectBufferMemory);

		/**************************************************************************
		Uniform buffer (and descriptor pool and set to bind them)
		Purpose: to set data for shaders every frame
		Note: A uniform buffer is a buffer that is made accessible in a read-only fashion to shaders so that the shaders can read constant parameter data.
		*/
		_vkCreateBuffer(UNIFORM_BUFFER_SIZE, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, g->device, &g->uniformBuffer,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, g->memProperties, &g->uniformBufferMemory);

		// descriptor pool and sets
		VkDescriptorPoolSize descriptorPoolSizeForUniformBuffer = {};
		descriptorPoolSizeForUniformBuffer.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorPoolSizeForUniformBuffer.descriptorCount = 1;

		VkDescriptorPoolSize descriptorPoolSizeForSampler = {};
		descriptorPoolSizeForSampler.type = VK_DESCRIPTOR_TYPE_SAMPLER;
		descriptorPoolSizeForSampler.descriptorCount = 1;

        VkDescriptorPoolSize descriptorPoolSizeForImage = {};
        descriptorPoolSizeForImage.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        descriptorPoolSizeForImage.descriptorCount = TEXTURE_COUNT;

		VkDescriptorPoolSize descriptorPoolSizes[] = { descriptorPoolSizeForUniformBuffer, descriptorPoolSizeForSampler, descriptorPoolSizeForImage };

		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
		descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolCreateInfo.poolSizeCount = 3;
		descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes;
		// i think this is how many do you need on gpu
		// i need only 1 because i render one frame at a time
		descriptorPoolCreateInfo.maxSets = 1;

		vkResult = vkCreateDescriptorPool(g->device, &descriptorPoolCreateInfo, nullptr, &g->descriptorPool);

#ifdef VIDBG
        if (vkResult != VK_SUCCESS)
        {
            fprintf(stderr, "vkCreateDescriptorPool failed\n");
            exit(1);
        }
#endif

		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
		descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocateInfo.descriptorPool = g->descriptorPool;
		descriptorSetAllocateInfo.descriptorSetCount = 1;
		descriptorSetAllocateInfo.pSetLayouts = &g->descriptorSetLayout;

		
		// You don't need to explicitly clean up descriptor sets, because they will be automatically freed when the descriptor pool is destroyed
		vkResult = vkAllocateDescriptorSets(g->device, &descriptorSetAllocateInfo, &g->descriptorSet);

#ifdef VIDBG
        if (vkResult != VK_SUCCESS)
        {
            fprintf(stderr, "vkAllocateDescriptorSets failed\n");
            exit(1);
        }
#endif

        // associate vkbuffer with descriptor set I guess
		VkDescriptorBufferInfo descriptorUniformBufferInfo = {};
		descriptorUniformBufferInfo.buffer = g->uniformBuffer;
		descriptorUniformBufferInfo.offset = 0;
		descriptorUniformBufferInfo.range = UNIFORM_BUFFER_SIZE;

		VkWriteDescriptorSet descriptorWriteForUniformBuffer = {};
		descriptorWriteForUniformBuffer.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWriteForUniformBuffer.dstSet = g->descriptorSet;
		descriptorWriteForUniformBuffer.dstBinding = 0;
		descriptorWriteForUniformBuffer.dstArrayElement = 0;
		descriptorWriteForUniformBuffer.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWriteForUniformBuffer.descriptorCount = 1;
		descriptorWriteForUniformBuffer.pBufferInfo = &descriptorUniformBufferInfo;

        vkUpdateDescriptorSets(g->device, 1, &descriptorWriteForUniformBuffer, 0, nullptr);

        // set sampler because it will never change
        VkDescriptorImageInfo descriptorImageInfo = {};
        descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        descriptorImageInfo.sampler = g->textureSampler;

        VkWriteDescriptorSet descriptorWriteForImage = {};
        descriptorWriteForImage.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWriteForImage.dstSet = g->descriptorSet;
        descriptorWriteForImage.dstBinding = 1;
        descriptorWriteForImage.dstArrayElement = 0;
        descriptorWriteForImage.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        descriptorWriteForImage.descriptorCount = 1;
        descriptorWriteForImage.pImageInfo = &descriptorImageInfo;

        vkUpdateDescriptorSets(g->device, 1, &descriptorWriteForImage, 0, nullptr);

        /**************************************************************************
        Semaphores for draw command
        */
        VkSemaphoreCreateInfo semaphoreCreateInfo = {};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        vkResult = vkCreateSemaphore(g->device, &semaphoreCreateInfo, nullptr, &g->imageAvailableSemaphore);

#ifdef VIDBG
        if (vkResult != VK_SUCCESS)
        {
            fprintf(stderr, "vkCreateSemaphore failed\n");
            exit(1);
        }
#endif

        vkResult = vkCreateSemaphore(g->device, &semaphoreCreateInfo, nullptr, &g->renderFinishedSemaphore);

#ifdef VIDBG
        if (vkResult != VK_SUCCESS)
        {
            fprintf(stderr, "vkCreateSemaphore failed\n");
            exit(1);
        }
#endif

		memory::heapClear(&helperHeap);
	}

	void graphicsInit(engine* g, engineInfo* info)
	{
		_win32InitWindow(g, info);
		_vkInitStep1(g, info);
	}

	void graphicsDestroy(engine* g)
	{
        vkQueueWaitIdle(g->queue);

        vkDestroySemaphore(g->device, g->renderFinishedSemaphore, nullptr);
        vkDestroySemaphore(g->device, g->imageAvailableSemaphore, nullptr);
        vkDestroyDescriptorPool(g->device, g->descriptorPool, nullptr);
        vkFreeMemory(g->device, g->uniformBufferMemory, nullptr);
        vkDestroyBuffer(g->device, g->uniformBuffer, nullptr);
        vkFreeMemory(g->device, g->drawIndirectBufferMemory, nullptr);
        vkDestroyBuffer(g->device, g->drawIndirectBuffer, nullptr);
        vkDestroySampler(g->device, g->textureSampler, nullptr);

        for (size_t i = 0; i < g->frameBufferCount; i++)
            vkDestroyFramebuffer(g->device, g->swapChainFramebuffers[i], nullptr);

        vkDestroyPipeline(g->device, g->graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(g->device, g->pipelineLayout, nullptr);
        vkDestroyRenderPass(g->device, g->renderPass, nullptr);
        vkDestroyImageView(g->device, g->depthBufferView, nullptr);
        vkFreeMemory(g->device, g->depthBufferMemory, nullptr);
        vkDestroyImage(g->device, g->depthBuffer, nullptr);
        vkDestroyDescriptorSetLayout(g->device, g->descriptorSetLayout, nullptr);
        vkDestroySwapchainKHR(g->device, g->swapChain, nullptr);

        for (size_t i = 0; i < g->frameBufferCount; i++)
            vkDestroyImageView(g->device, g->swapChainImageViews[i], nullptr);

        vkDestroySurfaceKHR(g->instance, g->surface, nullptr);
        vkDestroyCommandPool(g->device, g->commandPool, nullptr);
        vkDestroyDevice(g->device, nullptr);
        vkDestroyInstance(g->instance, nullptr);
        DestroyWindow(g->window);
        UnregisterClass(WND_CLASSNAME, g->hinstance);
	}

    // size in bytes of the texture will be 4 * width * height
    void createTexture(engine* g, byte* data, uint width, uint height, texture* result)
    {
        VkResult vkResult;

        // staging buffer
        VkBuffer textureStagingBuffer;
        VkDeviceMemory textureStagingBufferMemory;
        size_t size = width * height * 4;

        _vkCreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, g->device, &textureStagingBuffer,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, g->memProperties, &textureStagingBufferMemory);

        void* mappedStagingTextureBufferMemory;
        vkMapMemory(g->device, textureStagingBufferMemory, 0, size, 0, &mappedStagingTextureBufferMemory);
        memcpy(mappedStagingTextureBufferMemory, data, size);
        vkUnmapMemory(g->device, textureStagingBufferMemory);

        // image
        // it's possible to write texture data to VkBuffer but VkImage has more utility and it's faster
        // VkImage is VkBuffer but for images
        VkImage textureImage;
        VkDeviceMemory textureImageMemory;

        _vkCreateImage(width, height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            g->device, &textureImage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, g->memProperties, &textureImageMemory);

        // copy from staging to image

        // staging buffer now contains image in linear format (first row, then second row etc.)
        // however, image has specified tiling VK_IMAGE_TILING_OPTIMAL which is implementation specific (meaning undefiend for programmer) layout
        // liner has to be transformed to this optimal
        // image layout transition

        VkCommandBufferAllocateInfo stagingToImageCopyCommandAllocInfo = {};
        stagingToImageCopyCommandAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        stagingToImageCopyCommandAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        stagingToImageCopyCommandAllocInfo.commandPool = g->commandPool;
        stagingToImageCopyCommandAllocInfo.commandBufferCount = 1;

        VkCommandBuffer stagingToImageCopyCommand;
        vkResult = vkAllocateCommandBuffers(g->device, &stagingToImageCopyCommandAllocInfo, &stagingToImageCopyCommand);

#ifdef VIDBG
        if (vkResult != VK_SUCCESS)
        {
            fprintf(stderr, "vkAllocateCommandBuffers failed\n");
            exit(1);
        }
#endif

        VkCommandBufferBeginInfo stagingToImageCopyCommandBeginInfo = {};
        stagingToImageCopyCommandBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        stagingToImageCopyCommandBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkResult = vkBeginCommandBuffer(stagingToImageCopyCommand, &stagingToImageCopyCommandBeginInfo);

#ifdef VIDBG
        if (vkResult != VK_SUCCESS)
        {
            fprintf(stderr, "vkBeginCommandBuffer failed\n");
            exit(1);
        }
#endif

        // Transition the texture image layout to transfer target, so we can safely copy our buffer data to it.
        VkImageMemoryBarrier imageMemoryBarrier = {};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.image = textureImage;
        imageMemoryBarrier.srcAccessMask = 0;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        // The sub resource range describes the regions of the image that will be transitioned using the memory barriers below
        // Image only contains color data
        imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        // Start at first mip level
        imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
        // We will transition on all mip levels (there is 1 only)
        imageMemoryBarrier.subresourceRange.levelCount = 1;
        imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
        // The 2D texture only has one layer
        imageMemoryBarrier.subresourceRange.layerCount = 1;

        // Insert a memory dependency at the proper pipeline stages that will execute the image layout transition 
        // Source pipeline stage is host write/read exection (VK_PIPELINE_STAGE_HOST_BIT)
        // Destination pipeline stage is copy command exection (VK_PIPELINE_STAGE_TRANSFER_BIT)
        vkCmdPipelineBarrier(stagingToImageCopyCommand, VK_PIPELINE_STAGE_HOST_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

        // this needs to be declared for every mip level
        // and then in vkCmdCopyBufferToImage you send array of these
        // fortunetely i dont care and im doing only one
        VkBufferImageCopy bufferToImage = {};
        bufferToImage.bufferOffset = 0;
        bufferToImage.bufferRowLength = 0;
        bufferToImage.bufferImageHeight = 0;
        bufferToImage.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        bufferToImage.imageSubresource.mipLevel = 0;
        bufferToImage.imageSubresource.baseArrayLayer = 0;
        bufferToImage.imageSubresource.layerCount = 1;
        bufferToImage.imageOffset = { 0, 0, 0 };
        bufferToImage.imageExtent = { width, height, 1 };

        vkCmdCopyBufferToImage(stagingToImageCopyCommand, textureStagingBuffer, textureImage,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferToImage);

        // Once the data has been uploaded we transfer to the texture image to the shader read layout, so it can be sampled from
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // Insert a memory dependency at the proper pipeline stages that will execute the image layout transition 
        // Source pipeline stage stage is copy command exection (VK_PIPELINE_STAGE_TRANSFER_BIT)
        // Destination pipeline stage fragment shader access (VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)
        vkCmdPipelineBarrier(stagingToImageCopyCommand, VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

        vkResult = vkEndCommandBuffer(stagingToImageCopyCommand);

#ifdef VIDBG
        if (vkResult != VK_SUCCESS)
        {
            fprintf(stderr, "vkEndCommandBuffer failed\n");
            exit(1);
        }
#endif

        VkSubmitInfo stagingToImageCopyCommandSubmitInfo = {};
        stagingToImageCopyCommandSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        stagingToImageCopyCommandSubmitInfo.commandBufferCount = 1;
        stagingToImageCopyCommandSubmitInfo.pCommandBuffers = &stagingToImageCopyCommand;

        vkQueueSubmit(g->queue, 1, &stagingToImageCopyCommandSubmitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(g->queue);

        // this stuff is no longer needed
        vkFreeCommandBuffers(g->device, g->commandPool, 1, &stagingToImageCopyCommand);
        vkDestroyBuffer(g->device, textureStagingBuffer, nullptr);
        vkFreeMemory(g->device, textureStagingBufferMemory, nullptr);

        // image view for texture
        VkImageViewCreateInfo textureImageViewCreateInfo = {};
        textureImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        textureImageViewCreateInfo.image = textureImage;
        textureImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        textureImageViewCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
        textureImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        textureImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        textureImageViewCreateInfo.subresourceRange.levelCount = 1;
        textureImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        textureImageViewCreateInfo.subresourceRange.layerCount = 1;

        VkImageView textureImageView;
        vkResult = vkCreateImageView(g->device, &textureImageViewCreateInfo, nullptr, &textureImageView);

#ifdef VIDBG
        if (vkResult != VK_SUCCESS)
        {
            fprintf(stderr, "vkCreateImageView failed\n");
            exit(1);
        }
#endif

        result->height = height;
        result->width = width;
        result->image = textureImage;
        result->imageView = textureImageView;
        result->memory = textureImageMemory;
    }

    void createTexture(engine* g, const char* filename, texture* result)
    {
        int x = -1, y = -1, n = -1;
        const int components = 4; // components means how many elements from 'RGBA'
                                  // you want to return, I want 4 (RGBA) even in not all 4 are present
        byte* data = stbi_load(filename, &x, &y, &n, components);

#ifdef VIDBG
        if (data == nullptr)
        {
            fprintf(stderr, "createTexture could not open the file\n");
            exit(1);
        }
#endif

        createTexture(g, data, x, y, result);
        free(data);
    }

    void destroyTexture(engine* g, texture* t)
    {
        vkDestroyImageView(g->device, t->imageView, nullptr);
        vkDestroyImage(g->device, t->image, nullptr);
        vkFreeMemory(g->device, t->memory, nullptr);
    }

    void draw(engine* g, drawInfo* t, uint instances, camera* c)
    {
        VkDrawIndirectCommand drawParams;
        drawParams.firstInstance = 0;
        drawParams.firstVertex = 0;
        drawParams.instanceCount = instances;
        drawParams.vertexCount = 4;

        void* mappedIndirectBufferMemory = nullptr;
        vkMapMemory(g->device, g->drawIndirectBufferMemory, 0, INDIRECT_BUFFER_SIZE, 0, &mappedIndirectBufferMemory);
        memcpy(mappedIndirectBufferMemory, &drawParams, INDIRECT_BUFFER_SIZE);
        vkUnmapMemory(g->device, g->drawIndirectBufferMemory);

        void* mappedUniformBufferMemory = nullptr;
        vkMapMemory(g->device, g->uniformBufferMemory, 0, UNIFORM_BUFFER_SIZE, 0, &mappedUniformBufferMemory);
        memcpy(mappedUniformBufferMemory, c, sizeof(camera));
        memcpy((byte*)mappedUniformBufferMemory + sizeof(camera), t, sizeof(drawInfo) * instances);
        vkUnmapMemory(g->device, g->uniformBufferMemory);

        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

        VkSubmitInfo drawCommandSubmitInfo = {};
        drawCommandSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        drawCommandSubmitInfo.waitSemaphoreCount = 1;
        drawCommandSubmitInfo.pWaitSemaphores = &g->imageAvailableSemaphore;
        drawCommandSubmitInfo.pWaitDstStageMask = waitStages;
        drawCommandSubmitInfo.commandBufferCount = 1;
        drawCommandSubmitInfo.pCommandBuffers = &g->drawCommands[g->currentFrameIndex];
        drawCommandSubmitInfo.signalSemaphoreCount = 1;
        drawCommandSubmitInfo.pSignalSemaphores = &g->renderFinishedSemaphore;

        VkResult result = vkQueueSubmit(g->queue, 1, &drawCommandSubmitInfo, VK_NULL_HANDLE);

#ifdef VIDBG
        if (result != VK_SUCCESS)
        {
            fprintf(stderr, "vkQueueSubmit failed\n");
            exit(1);
        }
#endif
    }

    // 1. Have to update descriptor set
    // 2. Have to create draw command
    void pushTextures(engine* g, texture* textures, uint count)
    {
#ifdef VIDBG
        if (count > 40)
        {
            fprintf(stderr, "pushTextures max count is 40\n");
            exit(1);
        }
#endif

        VkDescriptorImageInfo descriptorImageInfo[40];

        for (uint i = 0; i < count; i++)
        {
            textures[i].index = i;
            descriptorImageInfo[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            descriptorImageInfo[i].imageView = textures[i].imageView;
        }

        VkWriteDescriptorSet descriptorWriteForImage = {};
        descriptorWriteForImage.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWriteForImage.dstSet = g->descriptorSet;
        descriptorWriteForImage.dstBinding = 2;
        descriptorWriteForImage.dstArrayElement = 0;
        descriptorWriteForImage.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        descriptorWriteForImage.descriptorCount = count;
        descriptorWriteForImage.pImageInfo = descriptorImageInfo;

        // TODO make sure that nothing is rendering while this is called
        // TODO this doesnt work
        vkUpdateDescriptorSets(g->device, 1, &descriptorWriteForImage, 0, nullptr);

        VkCommandBufferAllocateInfo drawCommandAllocInfo = {};
        drawCommandAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        drawCommandAllocInfo.commandPool = g->commandPool;
        drawCommandAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        drawCommandAllocInfo.commandBufferCount = g->frameBufferCount;

        // free old commands before allocating new ones
        if(g->drawCommands[0] != VK_NULL_HANDLE)
            vkFreeCommandBuffers(g->device, g->commandPool, g->frameBufferCount, g->drawCommands);

        VkResult vkResult = vkAllocateCommandBuffers(g->device, &drawCommandAllocInfo, g->drawCommands);

#ifdef VIDBG
        if (vkResult != VK_SUCCESS)
        {
            fprintf(stderr, "vkAllocateCommandBuffers failed\n");
            exit(1);
        }
#endif

        for (uint32_t i = 0; i < g->frameBufferCount; i++)
        {
            VkCommandBufferBeginInfo drawCommandBeginInfo = {};
            drawCommandBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            drawCommandBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

            vkResult = vkBeginCommandBuffer(g->drawCommands[i], &drawCommandBeginInfo);

#ifdef VIDBG
            if (vkResult != VK_SUCCESS)
            {
                fprintf(stderr, "vkBeginCommandBuffer failed\n");
                exit(1);
            }
#endif

            VkRenderPassBeginInfo renderPassBeginInfo = {};
            renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassBeginInfo.renderPass = g->renderPass;
            renderPassBeginInfo.framebuffer = g->swapChainFramebuffers[i];
            renderPassBeginInfo.renderArea.offset = { 0, 0 };
            renderPassBeginInfo.renderArea.extent = g->swapChainExtent;

            // one is to clear color data and other is to clear depth buffer
            VkClearValue clearColor[] = { { 1.0f, 1.0f, 1.0f, 1.0f }, {1.0f, 0} };
            renderPassBeginInfo.clearValueCount = 2;
            renderPassBeginInfo.pClearValues = clearColor;

            vkCmdBeginRenderPass(g->drawCommands[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(g->drawCommands[i], VK_PIPELINE_BIND_POINT_GRAPHICS, g->graphicsPipeline);
            vkCmdBindDescriptorSets(g->drawCommands[i], VK_PIPELINE_BIND_POINT_GRAPHICS, g->pipelineLayout, 0, 1, &g->descriptorSet, 0, nullptr);
            vkCmdDrawIndirect(g->drawCommands[i], g->drawIndirectBuffer, 0, 1, 0);
            vkCmdEndRenderPass(g->drawCommands[i]);

            vkResult = vkEndCommandBuffer(g->drawCommands[i]);

#ifdef VIDBG
            if (vkResult != VK_SUCCESS)
            {
                fprintf(stderr, "vkEndCommandBuffer failed\n");
                exit(1);
            }
#endif
        }

        // i'm not sure if this is needed
        vkQueueWaitIdle(g->queue);
    }

    // if returned false skip drawing for this frame, you are fine otherwise
    bool beginScene(engine* g)
    {
        // vkAcquireNextImageKHR returns non success if surface changes (more accurately, if surface is not available for presenting) for example when window is resized or minimalized
        // im only handling minimalization by stoping this draw call
        VkResult result = vkAcquireNextImageKHR(g->device, g->swapChain, LLONG_MAX, g->imageAvailableSemaphore, VK_NULL_HANDLE, &g->currentFrameIndex);
        return result == VK_SUCCESS;
    }

    void endScene(engine* g)
    {
        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &g->renderFinishedSemaphore;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &g->swapChain;
        presentInfo.pImageIndices = &g->currentFrameIndex;

        vkQueuePresentKHR(g->queue, &presentInfo);

        // this is NOT OPTIMAL way to wait until queue is done before continuing
        // without this it's possible that draw() function will be called faster than gpu can actually render and this will make program grow in memory usage
        vkQueueWaitIdle(g->queue);
    }

    void initCamera(engine* g, camera* c)
    {
        c->aspectRatio = g->swapChainExtent.width / (float)g->swapChainExtent.height;
        c->rotation = 0;
        c->scale = 1;
        c->x = 0;
        c->y = 0;
    }
}

namespace vi::input
{
    enum class key : int
    {

    };

    struct keyboard
    {
        bool buf1[KEYBOARD_KEY_COUNT];
        bool buf2[KEYBOARD_KEY_COUNT];
        bool* curState;
        bool* prevState;
    };

    void initKeyboard(keyboard* k)
    {
        k->curState = k->buf1;
        k->prevState = k->buf2;
        memset(k->buf1, 0, KEYBOARD_KEY_COUNT);
        memset(k->buf2, 0, KEYBOARD_KEY_COUNT);
    }

    void updateKeyboardState(keyboard* k)
    {
        // swap states
        bool* temp = k->prevState;
        k->prevState = k->curState;
        k->curState = temp;

        // get button states
        for (int i = 0; i < KEYBOARD_KEY_COUNT; i++)
        {
            k->curState[i] = (::GetAsyncKeyState(i) & 0x8000) && true;
        }
    }

    bool isKeyDown(keyboard* k, int _key)
    {
        return k->curState[_key];
    }

    bool isKeyPressed(keyboard* k, int _key)
    {
        // TODO, return false if this is the first frame
        //return k->curState[(int)_key] && !k->prevState[(int)_key] && engine->GetFrame() != 0;
        return k->curState[_key] && !k->prevState[_key];
    }

    bool isKeyReleased(keyboard* k, int _key)
    {
        return !k->curState[_key] && k->prevState[_key];
    }
}

namespace vi::network
{

}

namespace vi::system
{
	void loop(std::function<void()> activity)
	{
		MSG msg;

		while (true)
		{
			while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			if (msg.message == WM_QUIT)
				break;

            activity();
		}
	}
}

namespace vi
{
    struct viva
    {
        input::keyboard keyboard;
        graphics::engine graphics;
        graphics::camera camera;
        memory::heap heap;
        time::timer timer;
    };

    void initViva(viva* v, graphics::engineInfo* info)
    {
        input::initKeyboard(&v->keyboard);
        memory::heapInit(&v->heap);
        graphics::graphicsInit(&v->graphics, info);
        graphics::initCamera(&v->graphics, &v->camera);
        time::initTimer(&v->timer);
    }
}
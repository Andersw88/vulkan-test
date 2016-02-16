#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"


#define VKCPP_ENHANCED_MODE
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vk_cpp.h>



#include <windows.h>
#undef max
#undef min
#undef assert

#include <functional>
#include <assert.h>
#include <string>
#include <memory>


//static std::function<void (vk::Device device, const char* pName)> g_gdpa = nullptr;
//#define GET_DEVICE_PROC_ADDR(dev, entrypoint)                                  \
//    {                                                                          \
//        if (!g_gdpa)                                                           \
//            g_gdpa = (vk::getDeviceProcAddr)vk::getInstanceProcAddr(           \
//                inst, "vkGetDeviceProcAddr");                                  \
//        fp##entrypoint =                                                       \
//            (PFN_vk##entrypoint)g_gdpa(dev, "vk" #entrypoint);                 \
//        if (fp##entrypoint == nullptr) {                                          \
//            ERR_EXIT("vkGetDeviceProcAddr failed to find vk" #entrypoint,      \
//                     "vkGetDeviceProcAddr Failure");                           \
//        }                                                                      \
//    }

static const float g_vertex_buffer_data[] = {
	-1.0f,-1.0f,-1.0f,  // -X side
	-1.0f,-1.0f, 1.0f,
	-1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f,-1.0f,
	-1.0f,-1.0f,-1.0f,

	-1.0f,-1.0f,-1.0f,  // -Z side
	1.0f, 1.0f,-1.0f,
	1.0f,-1.0f,-1.0f,
	-1.0f,-1.0f,-1.0f,
	-1.0f, 1.0f,-1.0f,
	1.0f, 1.0f,-1.0f,

	-1.0f,-1.0f,-1.0f,  // -Y side
	1.0f,-1.0f,-1.0f,
	1.0f,-1.0f, 1.0f,
	-1.0f,-1.0f,-1.0f,
	1.0f,-1.0f, 1.0f,
	-1.0f,-1.0f, 1.0f,

	-1.0f, 1.0f,-1.0f,  // +Y side
	-1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f,-1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f, 1.0f,-1.0f,

	1.0f, 1.0f,-1.0f,  // +X side
	1.0f, 1.0f, 1.0f,
	1.0f,-1.0f, 1.0f,
	1.0f,-1.0f, 1.0f,
	1.0f,-1.0f,-1.0f,
	1.0f, 1.0f,-1.0f,

	-1.0f, 1.0f, 1.0f,  // +Z side
	-1.0f,-1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	-1.0f,-1.0f, 1.0f,
	1.0f,-1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
};

static const float g_uv_buffer_data[] = {
	0.0f, 0.0f,  // -X side
	1.0f, 0.0f,
	1.0f, 1.0f,
	1.0f, 1.0f,
	0.0f, 1.0f,
	0.0f, 0.0f,

	1.0f, 0.0f,  // -Z side
	0.0f, 1.0f,
	0.0f, 0.0f,
	1.0f, 0.0f,
	1.0f, 1.0f,
	0.0f, 1.0f,

	1.0f, 1.0f,  // -Y side
	1.0f, 0.0f,
	0.0f, 0.0f,
	1.0f, 1.0f,
	0.0f, 0.0f,
	0.0f, 1.0f,

	1.0f, 1.0f,  // +Y side
	0.0f, 1.0f,
	0.0f, 0.0f,
	1.0f, 1.0f,
	0.0f, 0.0f,
	1.0f, 0.0f,

	1.0f, 1.0f,  // +X side
	0.0f, 1.0f,
	0.0f, 0.0f,
	0.0f, 0.0f,
	1.0f, 0.0f,
	1.0f, 1.0f,

	0.0f, 1.0f,  // +Z side
	0.0f, 0.0f,
	1.0f, 1.0f,
	0.0f, 0.0f,
	1.0f, 0.0f,
	1.0f, 1.0f,
};


struct Demo;

std::unique_ptr<Demo> demo;

struct SwapchainBuffers {
	vk::Image image;
	vk::CommandBuffer cmd;
	vk::ImageView view;
};



struct Demo {

#define APP_NAME_STR_LEN 80
	HINSTANCE connection;        // hInstance - Windows Instance
	std::wstring name; // Name to put on the window/icon
	HWND window;                 // hWnd - window handle

	vk::SurfaceKHR surface;
	bool prepared;
	bool use_staging_buffer;

	vk::Instance inst;
	vk::PhysicalDevice gpu;
	vk::Device device;
	vk::Queue queue;
	vk::CommandPoolCreateFlags graphics_queue_node_index;
	vk::PhysicalDeviceProperties gpu_props;
	std::vector<std::shared_ptr<vk::QueueFamilyProperties>> queue_props;
	vk::PhysicalDeviceMemoryProperties memory_properties;

	uint32_t enabled_extension_count;
	uint32_t enabled_layer_count;
	char *extension_names[64];
	char *device_validation_layers[64];

	int width, height;
	vk::Format format;
	vk::ColorSpaceKHR color_space;

	//PFN_vkGetPhysicalDeviceSurfaceSupportKHR
	//	fpGetPhysicalDeviceSurfaceSupportKHR;
	//PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR
	//	fpGetPhysicalDeviceSurfaceCapabilitiesKHR;
	//PFN_vkGetPhysicalDeviceSurfaceFormatsKHR
	//	fpGetPhysicalDeviceSurfaceFormatsKHR;
	//PFN_vkGetPhysicalDeviceSurfacePresentModesKHR
	//	fpGetPhysicalDeviceSurfacePresentModesKHR;
	//PFN_vkCreateSwapchainKHR fpCreateSwapchainKHR;
	//PFN_vkDestroySwapchainKHR fpDestroySwapchainKHR;
	//PFN_vkGetSwapchainImagesKHR fpGetSwapchainImagesKHR;
	//PFN_vkAcquireNextImageKHR fpAcquireNextImageKHR;
	//PFN_vkQueuePresentKHR fpQueuePresentKHR;
	uint32_t swapchainImageCount;
	vk::SwapchainKHR swapchain;
	std::vector<SwapchainBuffers> swapchainBuffers;

	vk::CommandPool cmd_pool;

	struct {
		vk::Format format;

		vk::Image image;
		vk::MemoryAllocateInfo mem_alloc;
		vk::DeviceMemory mem;
		vk::ImageView view;
	} depth;

	struct texture_object {
		vk::Sampler sampler;

		vk::Image image;
		vk::ImageLayout imageLayout;

		vk::MemoryAllocateInfo mem_alloc;
		vk::DeviceMemory mem;
		vk::ImageView view;
		int32_t tex_width, tex_height;
	}texture;

	struct {
		vk::Buffer buf;
		vk::MemoryAllocateInfo mem_alloc;
		vk::DeviceMemory mem;
		vk::DescriptorBufferInfo buffer_info;
	} uniform_data;

	vk::CommandBuffer cmd; // Buffer for initialization commands
	vk::PipelineLayout pipeline_layout;
	vk::DescriptorSetLayout desc_layout;
	vk::PipelineCache pipelineCache;
	vk::RenderPass render_pass;
	vk::Pipeline pipeline;

	glm::mat4 projection_matrix;
	glm::mat4 view_matrix;
	glm::mat4 model_matrix;

	float spin_angle;
	float spin_increment;
	bool pause;

	vk::ShaderModule vert_shader_module;
	vk::ShaderModule frag_shader_module;

	vk::DescriptorPool desc_pool;
	vk::DescriptorSet desc_set;

	std::vector<std::shared_ptr<vk::Framebuffer>> framebuffers;

	bool quit;
	int32_t curFrame;
	int32_t frameCount;
	bool validate;
	bool use_break;
	//PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallback;
	//PFN_vkDestroyDebugReportCallbackEXT DestroyDebugReportCallback;



	vk::DebugReportCallbackEXT msg_callback;

	std::function<void(vk::Instance instance, 
		vk::DebugReportFlagsEXT flags, 
		vk::DebugReportObjectTypeEXT objectType, 
		uint64_t object, 
		size_t location, 
		int32_t messageCode, 
		const char* pLayerPrefix, 
		const char* pMessage)> DebugReportMessage;

	uint32_t current_buffer;
	uint32_t queue_count;


	void demo_init_vk() {
		vk::Result err;
		char *extension_names[64];
		uint32_t instance_extension_count = 0;
		uint32_t instance_layer_count = 0;
		uint32_t device_validation_layer_count = 0;
		uint32_t enabled_extension_count = 0;
		uint32_t enabled_layer_count = 0;

		char *instance_validation_layers[] = {
			"VK_LAYER_LUNARG_threading",      "VK_LAYER_LUNARG_mem_tracker",
			"VK_LAYER_LUNARG_object_tracker", "VK_LAYER_LUNARG_draw_state",
			"VK_LAYER_LUNARG_param_checker",  "VK_LAYER_LUNARG_swapchain",
			"VK_LAYER_LUNARG_device_limits",  "VK_LAYER_LUNARG_image",
			"VK_LAYER_GOOGLE_unique_objects",
		};

		device_validation_layers[0] = "VK_LAYER_LUNARG_threading";
		device_validation_layers[1] = "VK_LAYER_LUNARG_mem_tracker";
		device_validation_layers[2] = "VK_LAYER_LUNARG_object_tracker";
		device_validation_layers[3] = "VK_LAYER_LUNARG_draw_state";
		device_validation_layers[4] = "VK_LAYER_LUNARG_param_checker";
		device_validation_layers[5] = "VK_LAYER_LUNARG_swapchain";
		device_validation_layers[6] = "VK_LAYER_LUNARG_device_limits";
		device_validation_layers[7] = "VK_LAYER_LUNARG_image";
		device_validation_layers[8] = "VK_LAYER_GOOGLE_unique_objects";
		device_validation_layer_count = 9;

		/* Look for validation layers */
		vk::Bool32 validation_found = 0;
		err = vk::enumerateInstanceLayerProperties(&instance_layer_count, nullptr);
		assert(err == vk::Result::eVkSuccess);


		if (validate && !validation_found) {
			printf("vkEnumerateInstanceLayerProperties failed to find"
				"required validation layer.\n\nPlease look at the Getting Started guide for additional "
				"information.\n,vkCreateInstance Failure");
		}

		/* Look for instance extensions */
		vk::Bool32 surfaceExtFound = 0;
		vk::Bool32 platformSurfaceExtFound = 0;
		memset(extension_names, 0, sizeof(extension_names));

		err = vk::enumerateInstanceExtensionProperties(nullptr, &instance_extension_count, nullptr);
		assert(err == vk::Result::eVkSuccess);

//		if (instance_extension_count > 0) {
//			VkExtensionProperties *instance_extensions =
//				malloc(sizeof(VkExtensionProperties) * instance_extension_count);
//			err = vkEnumerateInstanceExtensionProperties(
//				nullptr, &instance_extension_count, instance_extensions);
//			assert(err == vk::Result::eVkSuccess);
//			for (uint32_t i = 0; i < instance_extension_count; i++) {
//				if (!strcmp(VK_KHR_SURFACE_EXTENSION_NAME,
//					instance_extensions[i].extensionName)) {
//					surfaceExtFound = 1;
//					extension_names[enabled_extension_count++] =
//						VK_KHR_SURFACE_EXTENSION_NAME;
//				}
//#ifdef _WIN32
//				if (!strcmp(VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
//					instance_extensions[i].extensionName)) {
//					platformSurfaceExtFound = 1;
//					extension_names[enabled_extension_count++] =
//						VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
//				}
//				if (!strcmp(VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
//					instance_extensions[i].extensionName)) {
//					if (validate) {
//						extension_names[enabled_extension_count++] =
//							VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
//					}
//				}
//				assert(enabled_extension_count < 64);
//			}
//
//			free(instance_extensions);
		}

	void demo_init() {
		glm::vec3 eye = { 0.0f, 3.0f, 5.0f };
		glm::vec3 origin = { 0, 0, 0 };
		glm::vec3 up = { 0.0f, 1.0f, 0.0 };

		frameCount = std::numeric_limits<int>::max();


		//use_staging_buffer = true;
		//use_break = true;
		//validate = true;


		//demo_init_connection(Demo);
		demo_init_vk();

		width = 500;
		height = 500;

		spin_angle = 0.01f;
		spin_increment = 0.01f;
		pause = false;


		projection_matrix = glm::perspective(glm::quarter_pi<float>(),1.0f, 0.1f, 100.0f);
		view_matrix = glm::lookAt(eye, origin, up);
		model_matrix = glm::mat4();
	}

	void demo_resize() {

		// Don't react to resize until after first initialization.
		if (!prepared) {
			return;
		}
		// In order to properly resize the window, we must re-create the swapchain
		// AND redo the command buffers, etc.
		//
		// First, perform part of the demo_cleanup() function:
		prepared = false;

		for (size_t i = 0; i < framebuffers.size(); i++) {
			vk::destroyFramebuffer(device, *framebuffers[i], nullptr);
		}
		framebuffers.clear();
		vk::destroyDescriptorPool(device, desc_pool, nullptr);

		vk::destroyPipeline(device, pipeline, nullptr);
		vk::destroyPipelineCache(device, pipelineCache, nullptr);
		vk::destroyRenderPass(device, render_pass, nullptr);
		vk::destroyPipelineLayout(device, pipeline_layout, nullptr);
		vk::destroyDescriptorSetLayout(device, desc_layout, nullptr);


		vk::destroyImageView(device, texture.view, nullptr);
		vk::destroyImage(device, texture.image, nullptr);
		vk::freeMemory(device, texture.mem, nullptr);
		vk::destroySampler(device, texture.sampler, nullptr);


		vk::destroyImageView(device, depth.view, nullptr);
		vk::destroyImage(device, depth.image, nullptr);
		vk::freeMemory(device, depth.mem, nullptr);

		vk::destroyBuffer(device, uniform_data.buf, nullptr);
		vk::freeMemory(device, uniform_data.mem, nullptr);

		for (size_t i = 0; i < swapchainImageCount; i++) {
			vk::destroyImageView(device, swapchainBuffers[i].view, nullptr);
			vk::freeCommandBuffers(device, cmd_pool, 1,
				&swapchainBuffers[i].cmd);
		}
		vk::destroyCommandPool(device, cmd_pool, nullptr);
		swapchainBuffers.clear();

		// Second, re-perform the demo_prepare() function, which will re-create the
		// swapchain:
		//demo_prepare(demo);
	}

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		switch (uMsg) {
		case WM_CLOSE:
			PostQuitMessage(0);
			break;
		case WM_PAINT:
			//demo->demo_run();
			break;
		case WM_SIZE:
			demo->width = lParam & 0xffff;
			demo->height = lParam & 0xffff0000 >> 16;
			demo->demo_resize();
			break;
		default:
			break;
		}
		return (DefWindowProc(hWnd, uMsg, wParam, lParam));
	}

	void demo_prepare_buffers() {
		vk::Result err;
		vk::SwapchainKHR oldSwapchain = swapchain;

		// Check the surface capabilities and formats
		vk::SurfaceCapabilitiesKHR surfCapabilities;
		err = vk::getPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &surfCapabilities);
		assert(err == vk::Result::eVkSuccess);

		uint32_t presentModeCount;
		err = vk::getPhysicalDeviceSurfacePresentModesKHR(
			gpu, surface, &presentModeCount, NULL);
		assert(err == vk::Result::eVkSuccess);
		std::vector<vk::PresentModeKHR> presentModes(presentModeCount);
		//vk::PresentModeKHR *presentModes =
		//	(VkT::PresentModeKHR *)malloc(presentModeCount * sizeof(VkPresentModeKHR));
		err = vk::getPhysicalDeviceSurfacePresentModesKHR(
			gpu, surface, &presentModeCount, presentModes.data());
		assert(err == vk::Result::eVkSuccess);

		vk::Extent2D swapchainExtent;
		// width and height are either both -1, or both not -1.
		if (surfCapabilities.currentExtent.width == (uint32_t)-1) {
			// If the surface size is undefined, the size is set to
			// the size of the images requested.
			swapchainExtent.width = width;
			swapchainExtent.height = height;
		}
		else {
			// If the surface size is defined, the swap chain size must match
			swapchainExtent = surfCapabilities.currentExtent;
			width = surfCapabilities.currentExtent.width;
			height = surfCapabilities.currentExtent.height;
		}

		// If mailbox mode is available, use it, as is the lowest-latency non-
		// tearing mode.  If not, try IMMEDIATE which will usually be available,
		// and is fastest (though it tears).  If not, fall back to FIFO which is
		// always available.
		vk::PresentModeKHR swapchainPresentMode = vk::PresentModeKHR::eVkPresentModeImmediateKhr;
		for (size_t i = 0; i < presentModeCount; i++) {
			if (presentModes[i] == vk::PresentModeKHR::eVkPresentModeMailboxKhr) {
				swapchainPresentMode = vk::PresentModeKHR::eVkPresentModeMailboxKhr;
				break;
			}
			if ((swapchainPresentMode != vk::PresentModeKHR::eVkPresentModeMailboxKhr) &&
				(presentModes[i] == vk::PresentModeKHR::eVkPresentModeImmediateKhr)) {
				swapchainPresentMode = vk::PresentModeKHR::eVkPresentModeImmediateKhr;
			}
		}

		// Determine the number of VkImage's to use in the swap chain (we desire to
		// own only 1 image at a time, besides the images being displayed and
		// queued for display):
		uint32_t desiredNumberOfSwapchainImages =
			surfCapabilities.minImageCount + 1;
		if ((surfCapabilities.maxImageCount > 0) &&
			(desiredNumberOfSwapchainImages > surfCapabilities.maxImageCount)) {
			// Application must settle for fewer images than desired:
			desiredNumberOfSwapchainImages = surfCapabilities.maxImageCount;
		}

		vk::SurfaceTransformFlagBitsKHR preTransform;
		if (surfCapabilities.supportedTransforms &
			vk::SurfaceTransformFlagBitsKHR::eIdentity) {
			preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
		}
		else {
			preTransform = surfCapabilities.currentTransform;
		}

		const vk::SwapchainCreateInfoKHR swapchain = vk::SwapchainCreateInfoKHR(0,surface,desiredNumberOfSwapchainImages,format,color_space, vk::Extent2D(swapchainExtent.width,swapchainExtent.height), (uint32_t)1,
			vk::ImageUsageFlagBits::eColorAttachment, vk::SharingMode::eExclusive, (uint32_t)0, (uint32_t*)nullptr, preTransform, vk::CompositeAlphaFlagBitsKHR::eOpaque, swapchainPresentMode, vk::Bool32(true), oldSwapchain );

		err = vk::createSwapchainKHR(device, &swapchain, nullptr, &this->swapchain);
		assert(err == vk::Result::eVkSuccess);

		// If we just re-created an existing swapchain, we should destroy the old
		// swapchain at this point.
		// Note: destroying the swapchain also cleans up all its associated
		// presentable images once the platform is done with them.
		if (oldSwapchain != VK_NULL_HANDLE) {
			vk::destroySwapchainKHR(device, oldSwapchain, nullptr);
		}

		err = vk::getSwapchainImagesKHR(device, this->swapchain, &swapchainImageCount, nullptr);
		assert(err == vk::Result::eVkSuccess);

		vk::Image *swapchainImages =
			(vk::Image *)malloc(swapchainImageCount * sizeof(vk::Image));
		assert(swapchainImages);
		err = vk::getSwapchainImagesKHR(device, this->swapchain,
			&swapchainImageCount,
			swapchainImages);
		assert(err == vk::Result::eVkSuccess);

		swapchainBuffers.resize(swapchainImageCount);
			//(SwapchainBuffers *)malloc(sizeof(SwapchainBuffers) *swapchainImageCount);
		//assert(swbuffers != nullptr);

		for (uint32_t i = 0; i < swapchainImageCount; i++) {
			vk::ImageViewCreateInfo color_image_view = {
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.pNext = nullptr,
				.format = format,
				.components =
			{
				.r = VK_COMPONENT_SWIZZLE_R,
				.g = VK_COMPONENT_SWIZZLE_G,
				.b = VK_COMPONENT_SWIZZLE_B,
				.a = VK_COMPONENT_SWIZZLE_A,
			},
				.subresourceRange = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1 },
				.viewType = VK_IMAGE_VIEW_TYPE_2D,
				.flags = 0,
			};

			buffers[i].image = swapchainImages[i];

			// Render loop will expect image to have been used before and in
			// VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
			// layout and will change to COLOR_ATTACHMENT_OPTIMAL, so init the image
			// to that state
			demo_set_image_layout(
				demo, buffers[i].image, VK_IMAGE_ASPECT_COLOR_BIT,
				VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

			color_image_view.image = buffers[i].image;

			err = vkCreateImageView(device, &color_image_view, nullptr,
				&buffers[i].view);
			assert(err == vk::Result::eVkSuccess);
		}

		if (nullptr != presentModes) {
			free(presentModes);
		}
	}

	void demo_create_window() {
		WNDCLASSEX win_class;

		// Initialize the window class structure:
		win_class.cbSize = sizeof(WNDCLASSEX);
		win_class.style = CS_HREDRAW | CS_VREDRAW;
		win_class.lpfnWndProc = WndProc;
		win_class.cbClsExtra = 0;
		win_class.cbWndExtra = 0;
		win_class.hInstance = connection; // hInstance
		win_class.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
		win_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
		win_class.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		win_class.lpszMenuName = nullptr;
		win_class.lpszClassName = name.c_str();
		win_class.hIconSm = LoadIcon(nullptr, IDI_WINLOGO);
		// Register window class:
		if (!RegisterClassEx(&win_class)) {
			// It didn't work, so try to give a useful error:
			printf("Unexpected error trying to start the application!\n");
			exit(1);
		}
		// Create window with the registered class:
		RECT wr = { 0, 0, width, height };
		AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
		window = CreateWindowEx(0,
			name.c_str(),           // class name
			name.c_str(),           // app name
			WS_OVERLAPPEDWINDOW | // window style
			WS_VISIBLE | WS_SYSMENU,
			100, 100,           // x/y coords
			wr.right - wr.left, // width
			wr.bottom - wr.top, // height
			nullptr,               // handle to parent
			nullptr,               // handle to menu
			connection,   // hInstance
			nullptr);              // no extra parameters
		if (!window) {
			// It didn't work, so try to give a useful error:
			printf("Cannot create a window in which to draw!\n");
			exit(1);
		}
	}

	void demo_init_vk_swapchain() {
		vk::Result err;
		uint32_t i;

		// Create a WSI surface for the window:
		vk::Win32SurfaceCreateInfoKHR createInfo = {0, connection, window };

		err = vk::createWin32SurfaceKHR(inst, &createInfo, nullptr, &surface);

		// Iterate over each queue to learn whether it supports presenting:
		std::vector<vk::Bool32> supportsPresent(queue_count);

		for (i = 0; i < queue_count; i++) {
			vk::getPhysicalDeviceSurfaceSupportKHR(gpu, i, surface, &supportsPresent[i]);
		}

		// Search for a graphics and a present queue in the array of queue
		// families, try to find one that supports both
		uint32_t graphicsQueueNodeIndex = UINT32_MAX;
		uint32_t presentQueueNodeIndex = UINT32_MAX;
		for (i = 0; i < queue_count; i++) {
			if ((queue_props[i]->queueFlags() & VK_QUEUE_GRAPHICS_BIT) != 0) {
				if (graphicsQueueNodeIndex == UINT32_MAX) {
					graphicsQueueNodeIndex = i;
				}

				if (supportsPresent[i] == VK_TRUE) {
					graphicsQueueNodeIndex = i;
					presentQueueNodeIndex = i;
					break;
				}
			}
		}
		if (presentQueueNodeIndex == UINT32_MAX) {
			// If didn't find a queue that supports both graphics and present, then
			// find a separate present queue.
			for (uint32_t i = 0; i < queue_count; ++i) {
				if (supportsPresent[i] == VK_TRUE) {
					presentQueueNodeIndex = i;
					break;
				}
			}
		}


		// Generate error if could not find both a graphics and a present queue
		//if (graphicsQueueNodeIndex == UINT32_MAX ||
		//	presentQueueNodeIndex == UINT32_MAX) {
		//	ERR_EXIT("Could not find a graphics and a present queue\n",
		//		"Swapchain Initialization Failure");
		//}

		// TODO: Add support for separate queues, including presentation,
		//       synchronization, and appropriate tracking for QueueSubmit.
		// NOTE: While it is possible for an application to use a separate graphics
		//       and a present queues, this demo program assumes it is only using
		//       one:
		//if (graphicsQueueNodeIndex != presentQueueNodeIndex) {
		//	ERR_EXIT("Could not find a common graphics and a present queue\n",
		//		"Swapchain Initialization Failure");
		//}

		graphics_queue_node_index = vk::Flags<vk::CommandPoolCreateFlagBits>(graphicsQueueNodeIndex);

		//demo_create_device(demo);
		//#define GET_DEVICE_PROC_ADDR(dev, entrypoint)  
		//if (!g_gdpa)                                                           
		//	g_gdpa = vk::getInstanceProcAddr(inst, "vkGetDeviceProcAddr");                                  
		//	auto t =  g_gdpa(device, "vk" ,vk::createSwapchainKHR);
  //      if (fp##entrypoint == nullptr) {                                          
  //          ERR_EXIT("vkGetDeviceProcAddr failed to find vk" #entrypoint,      
  //                   "vkGetDeviceProcAddr Failure");                           
  //      }  

		//GET_DEVICE_PROC_ADDR(device, vk::CreateSwapchainKHR);
		//GET_DEVICE_PROC_ADDR(device, vk::DestroySwapchainKHR);
		//GET_DEVICE_PROC_ADDR(device, vk::GetSwapchainImagesKHR);
		//GET_DEVICE_PROC_ADDR(device, vk::AcquireNextImageKHR);
		//GET_DEVICE_PROC_ADDR(device, vk::QueuePresentKHR);

		vk::getDeviceQueue(device, graphics_queue_node_index, 0, &queue);

		// Get the list of VkFormat's that are supported:
		uint32_t formatCount;
		err = vk::getPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, nullptr);
		assert(err == vk::Result::eVkSuccess);

		std::vector<vk::SurfaceFormatKHR> surfFormats(formatCount);
		//vk::SurfaceFormatKHR *surfFormats =
		//	(VkSurfaceFormatKHR *)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
		err = vk::getPhysicalDeviceSurfaceFormatsKHR(gpu, surface,&formatCount, surfFormats.data());

		assert(err == vk::Result::eVkSuccess);
		// If the format list includes just one entry of VK_FORMAT_UNDEFINED,
		// the surface has no preferred format.  Otherwise, at least one
		// supported format will be returned.
		if (formatCount == 1 && surfFormats[0].format() == vk::Format::eUndefined) {
			format = vk::Format::eB8G8R8A8Unorm;
		}
		else {
			assert(formatCount >= 1);
			format = surfFormats[0].format();
		}
		color_space = surfFormats[0].colorSpace();

		quit = false;
		curFrame = 0;

		// Get Memory information and properties
		vk::getPhysicalDeviceMemoryProperties(gpu, &memory_properties);
	}

	void demo_prepare() {
		vk::Result err;

		const vk::CommandPoolCreateInfo cmd_pool_info = {
			graphics_queue_node_index,
			0,
		};
		err = vk::createCommandPool(device, &cmd_pool_info, nullptr, &cmd_pool);
		assert(err == vk::Result::eVkSuccess);

		const vk::CommandBufferAllocateInfo cmd = {
			cmd_pool,
			vk::CommandBufferLevel::ePrimary,
			1,
		};

		demo_prepare_buffers();
		//demo_prepare_depth(demo);
		//demo_prepare_textures(demo);
		//demo_prepare_cube_data_buffer(demo);

		//demo_prepare_descriptor_layout(demo);
		//demo_prepare_render_pass(demo);
		//demo_prepare_pipeline(demo);

		for (uint32_t i = 0; i < swapchainImageCount; i++) {
			err =
				vk::allocateCommandBuffers(device, &cmd, &swapchainBuffers[i].cmd);
			assert(err == vk::Result::eVkSuccess);
		}

		//demo_prepare_descriptor_pool(demo);
		//demo_prepare_descriptor_set(demo);

		//demo_prepare_framebuffers(demo);

		//for (uint32_t i = 0; i < swapchainImageCount; i++) {
		//	current_buffer = i;
		//	demo_draw_build_cmd(demo, buffers[i].cmd);
		//}

		/*
		* Prepare functions above may generate pipeline commands
		* that need to be flushed before beginning the render loop.
		*/
		//demo_flush_init_cmd(demo);

		current_buffer = 0;
		prepared = true;
	}

};




int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) 
{
	demo = std::make_unique<Demo>();
	demo->connection = hInstance;
	demo->name = L"cube";



	demo.reset();
    return 0;
}
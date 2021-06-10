#include "VKPCH.h"
#define REGISTRATION(name) PFN_##name name = VK_NULL_HANDLE;
#include "VKImports.h"

VKFactory* Factory;
bool RHIInitialize()
{
	 Factory = bear_new<VKFactory>();
	if (!Factory->Empty())
	{
		GFactory = Factory;
		GStats = bear_new<VKStats>();
		BEAR_CHECK(GFactory);
		return true;
	}
	bear_delete(Factory);
	GFactory = 0;
	return false;
}
void VkError(VkResult result)
{
    const bchar* Text = TEXT("UNKOWN");
	switch (result)
	{
    case VK_SUCCESS:
        return;
    case VK_NOT_READY:
        Text = TEXT("VK_NOT_READY");
        break;
    case VK_TIMEOUT:
        Text = TEXT("VK_TIMEOUT");
        break;
    case VK_EVENT_SET:
        Text = TEXT("VK_EVENT_SET");
        break;
    case VK_EVENT_RESET:
        Text = TEXT("VK_EVENT_RESET");
        break;
    case VK_INCOMPLETE:
        Text = TEXT("VK_INCOMPLETE");
        break;
    case VK_ERROR_OUT_OF_HOST_MEMORY:
        Text = TEXT("VK_ERROR_OUT_OF_HOST_MEMORY");
        break;
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
        Text = TEXT("VK_ERROR_OUT_OF_DEVICE_MEMORY");
        break;
    case VK_ERROR_INITIALIZATION_FAILED:
        Text = TEXT("VK_ERROR_INITIALIZATION_FAILED");
        break;
    case VK_ERROR_DEVICE_LOST:
        Text = TEXT("VK_ERROR_DEVICE_LOST");
        break;
    case VK_ERROR_MEMORY_MAP_FAILED:
        Text = TEXT("VK_ERROR_MEMORY_MAP_FAILED");
        break;
    case VK_ERROR_LAYER_NOT_PRESENT:
        Text = TEXT("VK_ERROR_LAYER_NOT_PRESENT");
        break;
    case VK_ERROR_EXTENSION_NOT_PRESENT:
        Text = TEXT("VK_ERROR_EXTENSION_NOT_PRESENT");
        break;
    case VK_ERROR_FEATURE_NOT_PRESENT:
        Text = TEXT("VK_ERROR_FEATURE_NOT_PRESENT");
        break;
    case VK_ERROR_INCOMPATIBLE_DRIVER:
        Text = TEXT("VK_ERROR_INCOMPATIBLE_DRIVER");
        break;
    case VK_ERROR_TOO_MANY_OBJECTS:
        Text = TEXT("VK_ERROR_TOO_MANY_OBJECTS");
        break;
    case VK_ERROR_FORMAT_NOT_SUPPORTED:
        Text = TEXT("VK_ERROR_FORMAT_NOT_SUPPORTED");
        break;
    case VK_ERROR_FRAGMENTED_POOL:
        Text = TEXT("VK_ERROR_FRAGMENTED_POOL");
        break;
    case VK_ERROR_OUT_OF_POOL_MEMORY:
        Text = TEXT("VK_ERROR_OUT_OF_POOL_MEMORY");
        break;
    case VK_ERROR_INVALID_EXTERNAL_HANDLE:
        Text = TEXT("VK_ERROR_INVALID_EXTERNAL_HANDLE");
        break;
    case VK_ERROR_SURFACE_LOST_KHR:
        Text = TEXT("VK_ERROR_SURFACE_LOST_KHR");
        break;
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
        Text = TEXT("VK_ERROR_NATIVE_WINDOW_IN_USE_KHR");
        break;
    case VK_SUBOPTIMAL_KHR:
        Text = TEXT("VK_SUBOPTIMAL_KHR");
        break;
    case VK_ERROR_OUT_OF_DATE_KHR:
        Text = TEXT("VK_ERROR_OUT_OF_DATE_KHR");
        break;
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
        Text = TEXT("VK_ERROR_INCOMPATIBLE_DISPLAY_KHR");
        break;
    case VK_ERROR_VALIDATION_FAILED_EXT:
        Text = TEXT("VK_ERROR_VALIDATION_FAILED_EXT");
        break;
    case VK_ERROR_INVALID_SHADER_NV:
        Text = TEXT("VK_ERROR_INVALID_SHADER_NV");
        break;
    case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
        Text = TEXT("VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT");
        break;
    case VK_ERROR_FRAGMENTATION_EXT:
        Text = TEXT("VK_ERROR_FRAGMENTATION_EXT");
        break;
    case VK_ERROR_NOT_PERMITTED_EXT:
        Text = TEXT("VK_ERROR_NOT_PERMITTED_EXT");
        break;
    default:
        
        break;
	}
    BEAR_PRINTF(TEXT("Vulkan ERROR:" BEAR_PRINT_STR_CURRENT), Text);
    BEAR_CHECK(0);
}
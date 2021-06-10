#ifndef REGISTRATION
#define REGISTRATION(name) extern PFN_##name name;
#endif
#ifndef REGISTRATION_DEVICE
#define REGISTRATION_DEVICE(name) REGISTRATION(name)
#endif

#ifndef REGISTRATION_INSTANCE
#define REGISTRATION_INSTANCE(name) REGISTRATION(name)
#endif
REGISTRATION(vkEnumerateDeviceExtensionProperties);
REGISTRATION(vkEnumerateInstanceExtensionProperties);
REGISTRATION(vkGetInstanceProcAddr);
REGISTRATION(vkEnumerateInstanceLayerProperties);
REGISTRATION(vkDestroyRenderPass);
REGISTRATION(vkDestroySampler);
REGISTRATION(vkDestroySemaphore);
REGISTRATION(vkDestroyFence);
REGISTRATION(vkFreeCommandBuffers);
REGISTRATION(vkDestroyCommandPool);
REGISTRATION(vkDestroyPipelineLayout);
REGISTRATION(vkDestroyPipelineCache);
REGISTRATION(vkDestroyDevice);
REGISTRATION(vkDestroyInstance);
REGISTRATION(vkGetPhysicalDeviceProperties);
REGISTRATION(vkResetFences);
REGISTRATION(vkWaitForFences);
REGISTRATION(vkCreateSemaphore);
REGISTRATION(vkCreateFence);
REGISTRATION(vkCreateRenderPass);
REGISTRATION(vkCreateSampler);
REGISTRATION(vkAllocateCommandBuffers);
REGISTRATION(vkCreateCommandPool);
REGISTRATION(vkCreatePipelineCache);
REGISTRATION(vkCreatePipelineLayout);
REGISTRATION(vkGetDeviceQueue);
REGISTRATION(vkCreateDevice);
REGISTRATION(vkGetPhysicalDeviceMemoryProperties);
REGISTRATION(vkGetPhysicalDeviceQueueFamilyProperties);
REGISTRATION(vkGetPhysicalDeviceFeatures);
REGISTRATION(vkEnumeratePhysicalDevices);
REGISTRATION(vkCreateInstance);
REGISTRATION(vkQueueSubmit);
REGISTRATION(vkBeginCommandBuffer);
REGISTRATION(vkEndCommandBuffer);
REGISTRATION(vkCreateBuffer);
REGISTRATION(vkGetBufferMemoryRequirements);
REGISTRATION(vkAllocateMemory);
REGISTRATION(vkBindBufferMemory);
REGISTRATION(vkCmdCopyBuffer);
REGISTRATION(vkCmdPipelineBarrier);
REGISTRATION(vkCmdCopyBufferToImage);
REGISTRATION(vkCmdCopyImageToBuffer);
REGISTRATION(vkCreateDescriptorPool);
REGISTRATION(vkAllocateDescriptorSets);
REGISTRATION(vkDestroyDescriptorPool);
REGISTRATION(vkCmdBindDescriptorSets);
REGISTRATION(vkUpdateDescriptorSets);
REGISTRATION(vkCmdEndRenderPass);
REGISTRATION(vkCmdBeginRenderPass);
REGISTRATION(vkCmdBindVertexBuffers);
REGISTRATION(vkCmdBindIndexBuffer);
REGISTRATION(vkCmdSetStencilReference);
REGISTRATION(vkCmdSetViewport);
REGISTRATION(vkCmdSetScissor);
REGISTRATION(vkCmdDraw);
REGISTRATION(vkCmdDrawIndexed);
REGISTRATION(vkCmdCopyImage);
REGISTRATION(vkMapMemory);
REGISTRATION(vkUnmapMemory);
REGISTRATION(vkCreateFramebuffer);
REGISTRATION(vkDestroyFramebuffer);
REGISTRATION(vkFreeMemory);
REGISTRATION(vkDestroyBuffer);
REGISTRATION(vkCreateGraphicsPipelines);
REGISTRATION(vkDestroyPipeline);
REGISTRATION(vkCmdBindPipeline);
REGISTRATION(vkCreateDescriptorSetLayout);
REGISTRATION(vkDestroyDescriptorSetLayout);
REGISTRATION(vkCreateImage);
REGISTRATION(vkGetImageMemoryRequirements);
REGISTRATION(vkDestroyImage);
REGISTRATION(vkDestroyImageView);
REGISTRATION(vkCreateImageView);
REGISTRATION(vkBindImageMemory);
REGISTRATION(vkDestroyShaderModule);
REGISTRATION(vkCreateSwapchainKHR);
REGISTRATION(vkCreateWin32SurfaceKHR);
REGISTRATION(vkAcquireNextImageKHR);
REGISTRATION(vkDestroySurfaceKHR)
REGISTRATION(vkQueuePresentKHR);
REGISTRATION(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
REGISTRATION(vkGetPhysicalDeviceSurfaceFormatsKHR);
REGISTRATION(vkGetPhysicalDeviceSurfacePresentModesKHR)
REGISTRATION(vkGetPhysicalDeviceSurfaceSupportKHR)
REGISTRATION(vkDestroySwapchainKHR)
REGISTRATION(vkDeviceWaitIdle)
REGISTRATION(vkGetSwapchainImagesKHR)
REGISTRATION(vkCreateShaderModule)
REGISTRATION(vkGetDeviceProcAddr)
REGISTRATION(vkCreateBufferView)
#ifdef DEVELOPER_VERSION
REGISTRATION_INSTANCE(vkDestroyDebugUtilsMessengerEXT)
REGISTRATION_INSTANCE(vkCmdBeginDebugUtilsLabelEXT);
REGISTRATION_INSTANCE(vkCmdEndDebugUtilsLabelEXT);
REGISTRATION_INSTANCE(vkCreateDebugUtilsMessengerEXT)
#endif
#ifdef RTX
REGISTRATION(vkGetPhysicalDeviceProperties2)
REGISTRATION_DEVICE(vkCmdBuildAccelerationStructureNV)
REGISTRATION_DEVICE(vkCreateRayTracingPipelinesNV);
REGISTRATION_DEVICE(vkCreateAccelerationStructureNV);
REGISTRATION_DEVICE(vkGetAccelerationStructureMemoryRequirementsNV);
REGISTRATION_DEVICE(vkBindAccelerationStructureMemoryNV);
REGISTRATION_DEVICE(vkGetAccelerationStructureHandleNV);
REGISTRATION_DEVICE(vkGetRayTracingShaderGroupHandlesNV);
REGISTRATION_DEVICE(vkCmdTraceRaysNV)
REGISTRATION_DEVICE(vkDestroyAccelerationStructureNV);
#endif
#undef REGISTRATION
#undef REGISTRATION_DEVICE
#undef REGISTRATION_INSTANCE
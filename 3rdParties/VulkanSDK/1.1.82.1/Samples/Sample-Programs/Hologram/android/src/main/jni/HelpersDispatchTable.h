// This file is generated.
#ifndef HELPERSDISPATCHTABLE_H
#define HELPERSDISPATCHTABLE_H

#include <vulkan/vulkan.h>

namespace vk {

// VK_core
extern PFN_vkCreateInstance CreateInstance;
extern PFN_vkDestroyInstance DestroyInstance;
extern PFN_vkEnumeratePhysicalDevices EnumeratePhysicalDevices;
extern PFN_vkGetPhysicalDeviceFeatures GetPhysicalDeviceFeatures;
extern PFN_vkGetPhysicalDeviceFormatProperties GetPhysicalDeviceFormatProperties;
extern PFN_vkGetPhysicalDeviceImageFormatProperties GetPhysicalDeviceImageFormatProperties;
extern PFN_vkGetPhysicalDeviceProperties GetPhysicalDeviceProperties;
extern PFN_vkGetPhysicalDeviceQueueFamilyProperties GetPhysicalDeviceQueueFamilyProperties;
extern PFN_vkGetPhysicalDeviceMemoryProperties GetPhysicalDeviceMemoryProperties;
extern PFN_vkGetInstanceProcAddr GetInstanceProcAddr;
extern PFN_vkGetDeviceProcAddr GetDeviceProcAddr;
extern PFN_vkCreateDevice CreateDevice;
extern PFN_vkDestroyDevice DestroyDevice;
extern PFN_vkEnumerateInstanceExtensionProperties EnumerateInstanceExtensionProperties;
extern PFN_vkEnumerateDeviceExtensionProperties EnumerateDeviceExtensionProperties;
extern PFN_vkEnumerateInstanceLayerProperties EnumerateInstanceLayerProperties;
extern PFN_vkEnumerateDeviceLayerProperties EnumerateDeviceLayerProperties;
extern PFN_vkGetDeviceQueue GetDeviceQueue;
extern PFN_vkQueueSubmit QueueSubmit;
extern PFN_vkQueueWaitIdle QueueWaitIdle;
extern PFN_vkDeviceWaitIdle DeviceWaitIdle;
extern PFN_vkAllocateMemory AllocateMemory;
extern PFN_vkFreeMemory FreeMemory;
extern PFN_vkMapMemory MapMemory;
extern PFN_vkUnmapMemory UnmapMemory;
extern PFN_vkFlushMappedMemoryRanges FlushMappedMemoryRanges;
extern PFN_vkInvalidateMappedMemoryRanges InvalidateMappedMemoryRanges;
extern PFN_vkGetDeviceMemoryCommitment GetDeviceMemoryCommitment;
extern PFN_vkBindBufferMemory BindBufferMemory;
extern PFN_vkBindImageMemory BindImageMemory;
extern PFN_vkGetBufferMemoryRequirements GetBufferMemoryRequirements;
extern PFN_vkGetImageMemoryRequirements GetImageMemoryRequirements;
extern PFN_vkGetImageSparseMemoryRequirements GetImageSparseMemoryRequirements;
extern PFN_vkGetPhysicalDeviceSparseImageFormatProperties GetPhysicalDeviceSparseImageFormatProperties;
extern PFN_vkQueueBindSparse QueueBindSparse;
extern PFN_vkCreateFence CreateFence;
extern PFN_vkDestroyFence DestroyFence;
extern PFN_vkResetFences ResetFences;
extern PFN_vkGetFenceStatus GetFenceStatus;
extern PFN_vkWaitForFences WaitForFences;
extern PFN_vkCreateSemaphore CreateSemaphore;
extern PFN_vkDestroySemaphore DestroySemaphore;
extern PFN_vkCreateEvent CreateEvent;
extern PFN_vkDestroyEvent DestroyEvent;
extern PFN_vkGetEventStatus GetEventStatus;
extern PFN_vkSetEvent SetEvent;
extern PFN_vkResetEvent ResetEvent;
extern PFN_vkCreateQueryPool CreateQueryPool;
extern PFN_vkDestroyQueryPool DestroyQueryPool;
extern PFN_vkGetQueryPoolResults GetQueryPoolResults;
extern PFN_vkCreateBuffer CreateBuffer;
extern PFN_vkDestroyBuffer DestroyBuffer;
extern PFN_vkCreateBufferView CreateBufferView;
extern PFN_vkDestroyBufferView DestroyBufferView;
extern PFN_vkCreateImage CreateImage;
extern PFN_vkDestroyImage DestroyImage;
extern PFN_vkGetImageSubresourceLayout GetImageSubresourceLayout;
extern PFN_vkCreateImageView CreateImageView;
extern PFN_vkDestroyImageView DestroyImageView;
extern PFN_vkCreateShaderModule CreateShaderModule;
extern PFN_vkDestroyShaderModule DestroyShaderModule;
extern PFN_vkCreatePipelineCache CreatePipelineCache;
extern PFN_vkDestroyPipelineCache DestroyPipelineCache;
extern PFN_vkGetPipelineCacheData GetPipelineCacheData;
extern PFN_vkMergePipelineCaches MergePipelineCaches;
extern PFN_vkCreateGraphicsPipelines CreateGraphicsPipelines;
extern PFN_vkCreateComputePipelines CreateComputePipelines;
extern PFN_vkDestroyPipeline DestroyPipeline;
extern PFN_vkCreatePipelineLayout CreatePipelineLayout;
extern PFN_vkDestroyPipelineLayout DestroyPipelineLayout;
extern PFN_vkCreateSampler CreateSampler;
extern PFN_vkDestroySampler DestroySampler;
extern PFN_vkCreateDescriptorSetLayout CreateDescriptorSetLayout;
extern PFN_vkDestroyDescriptorSetLayout DestroyDescriptorSetLayout;
extern PFN_vkCreateDescriptorPool CreateDescriptorPool;
extern PFN_vkDestroyDescriptorPool DestroyDescriptorPool;
extern PFN_vkResetDescriptorPool ResetDescriptorPool;
extern PFN_vkAllocateDescriptorSets AllocateDescriptorSets;
extern PFN_vkFreeDescriptorSets FreeDescriptorSets;
extern PFN_vkUpdateDescriptorSets UpdateDescriptorSets;
extern PFN_vkCreateFramebuffer CreateFramebuffer;
extern PFN_vkDestroyFramebuffer DestroyFramebuffer;
extern PFN_vkCreateRenderPass CreateRenderPass;
extern PFN_vkDestroyRenderPass DestroyRenderPass;
extern PFN_vkGetRenderAreaGranularity GetRenderAreaGranularity;
extern PFN_vkCreateCommandPool CreateCommandPool;
extern PFN_vkDestroyCommandPool DestroyCommandPool;
extern PFN_vkResetCommandPool ResetCommandPool;
extern PFN_vkAllocateCommandBuffers AllocateCommandBuffers;
extern PFN_vkFreeCommandBuffers FreeCommandBuffers;
extern PFN_vkBeginCommandBuffer BeginCommandBuffer;
extern PFN_vkEndCommandBuffer EndCommandBuffer;
extern PFN_vkResetCommandBuffer ResetCommandBuffer;
extern PFN_vkCmdBindPipeline CmdBindPipeline;
extern PFN_vkCmdSetViewport CmdSetViewport;
extern PFN_vkCmdSetScissor CmdSetScissor;
extern PFN_vkCmdSetLineWidth CmdSetLineWidth;
extern PFN_vkCmdSetDepthBias CmdSetDepthBias;
extern PFN_vkCmdSetBlendConstants CmdSetBlendConstants;
extern PFN_vkCmdSetDepthBounds CmdSetDepthBounds;
extern PFN_vkCmdSetStencilCompareMask CmdSetStencilCompareMask;
extern PFN_vkCmdSetStencilWriteMask CmdSetStencilWriteMask;
extern PFN_vkCmdSetStencilReference CmdSetStencilReference;
extern PFN_vkCmdBindDescriptorSets CmdBindDescriptorSets;
extern PFN_vkCmdBindIndexBuffer CmdBindIndexBuffer;
extern PFN_vkCmdBindVertexBuffers CmdBindVertexBuffers;
extern PFN_vkCmdDraw CmdDraw;
extern PFN_vkCmdDrawIndexed CmdDrawIndexed;
extern PFN_vkCmdDrawIndirect CmdDrawIndirect;
extern PFN_vkCmdDrawIndexedIndirect CmdDrawIndexedIndirect;
extern PFN_vkCmdDispatch CmdDispatch;
extern PFN_vkCmdDispatchIndirect CmdDispatchIndirect;
extern PFN_vkCmdCopyBuffer CmdCopyBuffer;
extern PFN_vkCmdCopyImage CmdCopyImage;
extern PFN_vkCmdBlitImage CmdBlitImage;
extern PFN_vkCmdCopyBufferToImage CmdCopyBufferToImage;
extern PFN_vkCmdCopyImageToBuffer CmdCopyImageToBuffer;
extern PFN_vkCmdUpdateBuffer CmdUpdateBuffer;
extern PFN_vkCmdFillBuffer CmdFillBuffer;
extern PFN_vkCmdClearColorImage CmdClearColorImage;
extern PFN_vkCmdClearDepthStencilImage CmdClearDepthStencilImage;
extern PFN_vkCmdClearAttachments CmdClearAttachments;
extern PFN_vkCmdResolveImage CmdResolveImage;
extern PFN_vkCmdSetEvent CmdSetEvent;
extern PFN_vkCmdResetEvent CmdResetEvent;
extern PFN_vkCmdWaitEvents CmdWaitEvents;
extern PFN_vkCmdPipelineBarrier CmdPipelineBarrier;
extern PFN_vkCmdBeginQuery CmdBeginQuery;
extern PFN_vkCmdEndQuery CmdEndQuery;
extern PFN_vkCmdResetQueryPool CmdResetQueryPool;
extern PFN_vkCmdWriteTimestamp CmdWriteTimestamp;
extern PFN_vkCmdCopyQueryPoolResults CmdCopyQueryPoolResults;
extern PFN_vkCmdPushConstants CmdPushConstants;
extern PFN_vkCmdBeginRenderPass CmdBeginRenderPass;
extern PFN_vkCmdNextSubpass CmdNextSubpass;
extern PFN_vkCmdEndRenderPass CmdEndRenderPass;
extern PFN_vkCmdExecuteCommands CmdExecuteCommands;

// VK_KHR_surface
extern PFN_vkDestroySurfaceKHR DestroySurfaceKHR;
extern PFN_vkGetPhysicalDeviceSurfaceSupportKHR GetPhysicalDeviceSurfaceSupportKHR;
extern PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR GetPhysicalDeviceSurfaceCapabilitiesKHR;
extern PFN_vkGetPhysicalDeviceSurfaceFormatsKHR GetPhysicalDeviceSurfaceFormatsKHR;
extern PFN_vkGetPhysicalDeviceSurfacePresentModesKHR GetPhysicalDeviceSurfacePresentModesKHR;

// VK_KHR_swapchain
extern PFN_vkCreateSwapchainKHR CreateSwapchainKHR;
extern PFN_vkDestroySwapchainKHR DestroySwapchainKHR;
extern PFN_vkGetSwapchainImagesKHR GetSwapchainImagesKHR;
extern PFN_vkAcquireNextImageKHR AcquireNextImageKHR;
extern PFN_vkQueuePresentKHR QueuePresentKHR;

// VK_KHR_display
extern PFN_vkGetPhysicalDeviceDisplayPropertiesKHR GetPhysicalDeviceDisplayPropertiesKHR;
extern PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR GetPhysicalDeviceDisplayPlanePropertiesKHR;
extern PFN_vkGetDisplayPlaneSupportedDisplaysKHR GetDisplayPlaneSupportedDisplaysKHR;
extern PFN_vkGetDisplayModePropertiesKHR GetDisplayModePropertiesKHR;
extern PFN_vkCreateDisplayModeKHR CreateDisplayModeKHR;
extern PFN_vkGetDisplayPlaneCapabilitiesKHR GetDisplayPlaneCapabilitiesKHR;
extern PFN_vkCreateDisplayPlaneSurfaceKHR CreateDisplayPlaneSurfaceKHR;

// VK_KHR_display_swapchain
extern PFN_vkCreateSharedSwapchainsKHR CreateSharedSwapchainsKHR;

#ifdef VK_USE_PLATFORM_XLIB_KHR
// VK_KHR_xlib_surface
extern PFN_vkCreateXlibSurfaceKHR CreateXlibSurfaceKHR;
extern PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR GetPhysicalDeviceXlibPresentationSupportKHR;
#endif

#ifdef VK_USE_PLATFORM_XCB_KHR
// VK_KHR_xcb_surface
extern PFN_vkCreateXcbSurfaceKHR CreateXcbSurfaceKHR;
extern PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR GetPhysicalDeviceXcbPresentationSupportKHR;
#endif

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
// VK_KHR_wayland_surface
extern PFN_vkCreateWaylandSurfaceKHR CreateWaylandSurfaceKHR;
extern PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR GetPhysicalDeviceWaylandPresentationSupportKHR;
#endif

#ifdef VK_USE_PLATFORM_MIR_KHR
// VK_KHR_mir_surface
extern PFN_vkCreateMirSurfaceKHR CreateMirSurfaceKHR;
extern PFN_vkGetPhysicalDeviceMirPresentationSupportKHR GetPhysicalDeviceMirPresentationSupportKHR;
#endif

#ifdef VK_USE_PLATFORM_ANDROID_KHR
// VK_KHR_android_surface
extern PFN_vkCreateAndroidSurfaceKHR CreateAndroidSurfaceKHR;
#endif

#ifdef VK_USE_PLATFORM_WIN32_KHR
// VK_KHR_win32_surface
extern PFN_vkCreateWin32SurfaceKHR CreateWin32SurfaceKHR;
extern PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR GetPhysicalDeviceWin32PresentationSupportKHR;
#endif

// VK_EXT_debug_report
extern PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallbackEXT;
extern PFN_vkDestroyDebugReportCallbackEXT DestroyDebugReportCallbackEXT;
extern PFN_vkDebugReportMessageEXT DebugReportMessageEXT;

void init_dispatch_table_top(PFN_vkGetInstanceProcAddr get_instance_proc_addr);
void init_dispatch_table_middle(VkInstance instance, bool include_bottom);
void init_dispatch_table_bottom(VkInstance instance, VkDevice dev);

}  // namespace vk

#endif  // HELPERSDISPATCHTABLE_H

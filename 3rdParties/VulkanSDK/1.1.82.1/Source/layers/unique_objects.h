/* Copyright (c) 2015-2016 The Khronos Group Inc.
 * Copyright (c) 2015-2016 Valve Corporation
 * Copyright (c) 2015-2016 LunarG, Inc.
 * Copyright (C) 2015-2016 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Mark Lobodzinski <mark@lunarg.com>
 */

#include "vulkan/vulkan.h"

#include <string>
#include <unordered_map>
#include <unordered_set>

#include "vk_layer_data.h"
#include "vk_safe_struct.h"
#include "vk_layer_utils.h"
#include "mutex"

#pragma once

namespace unique_objects {

// All increments must be guarded by global_lock
static uint64_t global_unique_id = 1;
static std::unordered_map<uint64_t, uint64_t> unique_id_mapping;  // Map uniqueID to actual object handle

struct TEMPLATE_STATE {
    VkDescriptorUpdateTemplateKHR desc_update_template;
    safe_VkDescriptorUpdateTemplateCreateInfo create_info;

    TEMPLATE_STATE(VkDescriptorUpdateTemplateKHR update_template, safe_VkDescriptorUpdateTemplateCreateInfo *pCreateInfo)
        : desc_update_template(update_template), create_info(*pCreateInfo) {}
};

struct instance_layer_data {

    VkInstance instance;

    debug_report_data *report_data;
    std::vector<VkDebugReportCallbackEXT> logging_callback;
    std::vector<VkDebugUtilsMessengerEXT> logging_messenger;
    VkLayerInstanceDispatchTable dispatch_table = {};

    // The following are for keeping track of the temporary callbacks that can
    // be used in vkCreateInstance and vkDestroyInstance:
    uint32_t num_tmp_report_callbacks;
    VkDebugReportCallbackCreateInfoEXT *tmp_report_create_infos;
    VkDebugReportCallbackEXT *tmp_report_callbacks;
    uint32_t num_tmp_debug_messengers;
    VkDebugUtilsMessengerCreateInfoEXT *tmp_messenger_create_infos;
    VkDebugUtilsMessengerEXT *tmp_debug_messengers;
};

struct layer_data {
    instance_layer_data *instance_data;

    debug_report_data *report_data;
    VkLayerDispatchTable dispatch_table = {};

    std::unordered_map<uint64_t, std::unique_ptr<TEMPLATE_STATE>> desc_template_map;
    std::unordered_set<std::string> device_extension_set;

    bool wsi_enabled;
    VkPhysicalDevice gpu;

    struct SubpassesUsageStates {
        std::unordered_set<uint32_t> subpasses_using_color_attachment;
        std::unordered_set<uint32_t> subpasses_using_depthstencil_attachment;
    };
    // Uses unwrapped handles
    std::unordered_map<VkRenderPass, SubpassesUsageStates> renderpasses_states;

    // Map of wrapped swapchain handles to arrays of wrapped swapchain image IDs
    // Each swapchain has an immutable list of wrapped swapchain image IDs -- always return these IDs if they exist
    std::unordered_map<VkSwapchainKHR, std::vector<VkImage>> swapchain_wrapped_image_handle_map;

    layer_data() : wsi_enabled(false), gpu(VK_NULL_HANDLE){};
};

static std::unordered_map<void *, instance_layer_data *> instance_layer_data_map;
static std::unordered_map<void *, layer_data *> layer_data_map;

static std::mutex global_lock;  // Protect map accesses and unique_id increments

struct GenericHeader {
    VkStructureType sType;
    void *pNext;
};

template <typename T>
bool ContainsExtStruct(const T *target, VkStructureType ext_type) {
    assert(target != nullptr);

    const GenericHeader *ext_struct = reinterpret_cast<const GenericHeader *>(target->pNext);

    while (ext_struct != nullptr) {
        if (ext_struct->sType == ext_type) {
            return true;
        }

        ext_struct = reinterpret_cast<const GenericHeader *>(ext_struct->pNext);
    }

    return false;
}

/* Unwrap a handle. */
// must hold lock!
template <typename HandleType>
HandleType Unwrap(HandleType wrappedHandle) {
    // TODO: don't use operator[] here.
    return (HandleType)unique_id_mapping[reinterpret_cast<uint64_t const &>(wrappedHandle)];
}

// Wrap a newly created handle with a new unique ID, and return the new ID -- must hold lock!
template <typename HandleType>
HandleType WrapNew(HandleType newlyCreatedHandle) {
    auto unique_id = global_unique_id++;
    unique_id_mapping[unique_id] = reinterpret_cast<uint64_t const &>(newlyCreatedHandle);
    return (HandleType)unique_id;
}

}  // namespace unique_objects

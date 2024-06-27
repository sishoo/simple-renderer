#include "renderer.hpp"

#define RENDERER_NFRAMES_IN_FLIGHT 2

#define VK_TRY(try)                                           \
do                                                            \
{                                                             \
        if ((try) != VK_SUCCESS)                              \
        {                                                     \
                fprintf(stderr, #try" was not VK_SUCCESS\n"); \
                abort();                                      \
        }                                                     \
} while(0);


void renderer_init_vulkan(renderer_t *const r)
{
        vkb::InstanceBuilder builder{};
        auto instance_result = builder
                .set_app_name("Hephaestus Renderer")
                .set_engine_name("Hephaestus Engine")
                .require_api_version(1, 3, 0)
                .use_default_debug_messenger()
              //.set_debug_callback(debug_callback)
                .request_validation_layers()
                .build();
        VK_TRY(instance_result.vk_result());
        r->vkb_instance = instance_result.value();
        r->instance = r->vkb_instance.instance;

        #warning DONT DELETE THIS UNTIL YOU FIX THE DAMN PICK FIRST DEVICE UNCONDITIONALLY!!!
        vkb::PhysicalDeviceSelector selector(r->vkb_instance);
        auto pdevice_result = selector
                       .set_surface(r->surface)
                       .select_first_device_unconditionally()
                       .select();
        VK_TRY(pdevice_result.vk_result());
        r->vkb_pdevice = pdevice_result.value();
        r->pdevice = r->vkb_pdevice.physical_device;

        VkPhysicalDeviceSynchronization2Features sync_features = {};
        sync_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
        sync_features.synchronization2 = VK_TRUE;

        vkb::DeviceBuilder builder{r->vkb_pdevice};
        auto ldevice_result = builder
                       .add_pNext(&sync_features)
                       .build();
        VK_TRY(ldevice_result.vk_result());
        r->vkb_ldevice = ldevice_result.value();
        r->ldevice = r->vkb_ldevice.device;


}

void renderer_init_window(renderer_t *const r, char *const name)
{
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        r->window = glfwCreateWindow(r->window_width, r->window_height, name, NULL, NULL);
        if (r->window == NULL)
        {       
                fprintf(stderr, "Window pointer was NULL.\n");
                abort();
        }
}

void renderer_init_surface(renderer_t *const r)
{
        VK_TRY(glfwCreatewindowSurface(r->instance, r->window, NULL, &r->surface));
}

void renderer_init_frame_infos(renderer_t *const r)
{
        r->frame_infos = malloc(sizeof(frame_infos_t) * RENDERER_NFRAMES_IN_FLIGHT + 1);

        VkFenceCreateInfo fence_create_info = {
                .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,

        };

        VkSemaphoreCreateInfo semaphore_create_info = {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
        };

        VkCommandBufferAllocateInfo allocate_info = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool = r->command_pool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1
        };

        for (uint32_t i = 0; i < RENDERER_NFRAMES_IN_FLIGHT + 1; i++)
        {
                frame_infos_t frame_infos = &r->frame_infos[i];
                VK_TRY(vkCreateFence(r->ldevice, &fence_create_info, NULL, &frame_infos.complete_fence));
                VK_TRY(vkCreateSemaphore(r->ldevice, &semaphore_create_info, NULL, &frame_infos.complete_semaphore));
                VK_TRY(vkAllocateCommandBuffers(r->ldevice, &allocate_info, &frame_infos.command_buffer));
        }
}

void renderer_init_graphics_pipeline(renderer_t *const r)
{
         /* Pipeline rendering create info */
        VkFormat color_attachment_format = VK_FORMAT_R32G32B32A32_SFLOAT;
        VkPipelineRenderingCreateInfo pipeline_rendering_create_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
                .depthAttachmentFormat = VK_FORMAT_R32G32B32_SFLOAT,
                .colorAttachmentCount = 1,
                .pColorAttachmentFormats = &color_attachment_format,
                .stencilAttachmentFormat = VK_FORMAT_R32G32B32_SFLOAT,
                .viewMask = 0
        };

        /* Grahpics pipeline create info */
        VkPipelineShaderStageCreateInfo *shader_stage_create_infos = (VkPipelineShaderStageCreateInfo *)HALLOC(2 * sizeof(VkPipelineShaderStageCreateInfo));
        /* Vertex shader*/
        shader_stage_create_infos[0] = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .flags = VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT,
                .stage = VK_SHADER_STAGE_VERTEX_BIT,
                .module = r->vertex_shader_module,
                .pName = "main"
        };
        /* Fragment shader */
        shader_stage_create_infos[1] = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .flags = VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT,
                .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                .module = r->fragment_shader_module,
                .pName = "main"
        };

        /* Vertex input state infos */
        VkVertexInputAttributeDescription vertex_input_attribute_descriptions[2];
        VkVertexInputBindingDescription vertex_input_binding_descriptions[2];

        /* Vertex input ATTRIBUTE descriptions */
        /* Position */
        vertex_input_attribute_descriptions[0] = {
                .binding = HEPH_RENDERER_VERTEX_INPUT_ATTR_DESC_BINDING_POSITION,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .location = 0,
                .offset = offsetof(heph_vertex_t, position)
        };
        /* U/V */
        vertex_input_attribute_descriptions[1] = {
                .binding = ,
                .format = VK_FORMAT_R32G32_SFLOAT,
                .location = ,
                .offset = offsetof(heph_vertex_t, uv)
        };

        /* Vertex input BINDING descriptions */
        /* Position */
        vertex_input_binding_descriptions[0] = {
                .binding = HEPH_RENDERER_VERTEX_INPUT_ATTR_DESC_BINDING_POSITION,
                .stride = sizeof(heph_vector3_t),
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
        };
        /* U/V */
        vertex_input_binding_descriptions[1] = {
                .binding = ,
                .stride = sizeof(heph_vector2_t),
                .inputeRate = VK_VERTEX_INPUT_RATE_VERTEX
        };

        VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                .vertexBindingDescriptionCount = 2,
                .pVertexBindingDescriptions = vertex_input_binding_descriptions,
                .vertexAttributeDescriptionCount = 2,
                .pVertexAttributeDescriptions = vertex_input_attribute_descriptions,
        };

        /* Input assembly state info */
        VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                .primitiveRestartEnable = VK_FALSE
        };

        /* Tessellation */
        VkPipelineTessellationStateCreateInfo tessellation_state_create_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
                .patchControlPoints = 0
        };

        /* Viewport state */
        VkViewport viewport = {
                .x = 0.0,
                .y = 0.0,
                .width = 1920.0,
                .height = 1080.0,
                .minDepth = 0.0,
                .maxDepth = 1.0
        };

        VkRect2D scissor = {
                .offset = {0, 0},
                .extent = {1920, 1080}
        };

        VkPipelineViewportStateCreateInfo viewport_state_create_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                .viewportCount = HEPH_RENDERER_PIPELINE_VIEWPORT_COUNT,
                .pViewports = &viewport,
                .scissorCount = 1,
                .pScissors = &scissor
        };
       

        /* Rasterization State */
        #warning this is janky come backe and properly do this (EX: cull mode)
        VkPipelineRasterizationStateCreateInfo rasterization_state_create_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                .depthClampEnable = VK_FALSE,
                .rasterizerDiscardEnable = VK_FALSE,
                .polygonMode = VK_POLYGON_MODE_FILL,
                .cullMode = VK_CULL_MODE_FRONT_BIT,
                .frontFace = VK_FRONT_FACE_CLOCKWISE,
                .depthBiasEnable = VK_FALSE,
                .lineWidth = 1.0
        };

        /* Multisample state */
        VkPipelineMultisampleStateCreateInfo multisample_state = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                .flags = 
        };

        /* Final grahpics pipeline */
        VkGraphicsPipelineCreateInfo pipeline_create_info = {
                .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                .pNext = &pipeline_rendering_create_info,
                .stageCount = 2,
                .pStages = shader_stage_create_infos,
                .pVertexInputState = &vertex_input_state_create_info,
                .pInputAssemblyState = &input_assembly_state_create_info,

        };

        VK_TRY(vkCreateGraphicsPipelines(r->ldevice, VK_NULL_HANDLE, 1, &pipeline_create_info, NULL, &r->graphics_pipeline));
}

void renderer_init(renderer_t *const r)
{
        renderer_init_window(r);
        renderer_init_vulkan(r);
        renderer_init_surface(r);

        renderer_init_frame_infos(r);
        renderer_init_graphics_pipeline(r);
}       

void renderer_draw(renderer_t *const r)
{
        static bool first_frame = 1;

        uint32_t resource_index = r->previous_resource_index + 1;
        if (resource_index == r->nswapchain_images)
        {
                resource_index = 0;
        }

        frame_infos_t frame_infos = r->frame_infos[resource_index];

        /* Make sure to not overwrite currently used resources */
        VK_TRY(vkWaitForFences(r->ldevice, 1, &frame_infos.complete_fence, VK_TRUE, 100));
        VK_TRY(vkResetFence(r->ldevice, 1, &frame_infos.complete_fence));

        uint32_t image_index;
        VK_TRY(vkAcquireNextImageKHR(r->ldevice, r->swapchain, 100, frame_infos.image_acquired_semaphore, VK_NULL_HANDLE, &image_index));
        VkImage target_image = r->swapchain_images[image_index];

        VK_TRY(vkResetCommandBuffer(frame_infos.command_buffer, 0));

        VkCommandBufferBeginInfo begin_info = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
        };

        VK_TRY(vkBeginCommandBuffer(frame_infos.command_buffer, &begin_info))

        vkCmdBindGraphicsPipeline(frame_infos.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, r->graphics_pipeline);

        for (uint32_t i = 0; i < nobjects; i++)
        {
                vkCmdBindVertexBuffers(frame_infos.command_buffer, 0, 1, &r->objects[i].vertex_buffer, (VkDeviceSize[]) { 0 });
                vkCmdBindIndexBuffer(frame_infos.command_buffer, &r->objects[i].index_buffer, 0, VK_INDEX_TYPE_UINT32);
                vkCmdDrawIndexed(frame_infos.command_buffer, &r->objects[i].nindices, 1, 0, 0, 0);
        }

        VK_TRY(vkEndCommandBuffer(frame_infos.command_buffer));


        VkSubmitInfo submit_info = {
                .sType = VK_STRUCTURE_TYPE_QUEUE_SUBMIT_INFO, 
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = &r->frame_infos[r->previous_resource_index].complete_semaphore,

        };

        VK_TRY(vkQueueSubmitKHR(r->queue, &submit_info));


        VkPresentInfoKHR present_info = {
                .sType = VK_STRUCTURE_TYPE_PRESENT_INFO, 
                .waitSemaphoreCount = 0,
                .swapchainCount = 1,
                .pSwapchains = &r->swapchain,
                .pImageIndices = (uint32_t[]) {image_index}
        }

        VK_TRY(vkQueuePresent(&r->queue, 1, &present_info));

        first_frame = 0;
}
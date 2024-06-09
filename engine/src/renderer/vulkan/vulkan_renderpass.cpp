#include "vulkan_renderpass.hpp"

#include "core/kmemory.hpp"

void vulkan_renderpass::create(vulkan_context* context, f32 x_, f32 y_, f32 w_, f32 h_, f32 r_, f32 g_, f32 b_, f32 a_, f32 depth_, u32 stencil_){

    x=x_;
    y=y_;
    w=w_;
    h=h_;
    r=r_;
    g=g_;
    b=b_;
    a=a_;
    depth=depth_;
    stencil=stencil_;

    //Main subpass
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    //Attachments TODO: make this configurable.
    constexpr u32 attachment_description_count = 2;
    VkAttachmentDescription attachment_descriptions[attachment_description_count];

    //Color attachment
    VkAttachmentDescription color_attachment;
    color_attachment.format = context->swapchain.image_format.format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;         //undefined at start of renderpass
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;     //transitions to present at end
    color_attachment.flags = 0;

    attachment_descriptions[0] = color_attachment;

    VkAttachmentReference color_attachment_reference;
    color_attachment_reference.attachment = 0;  //attachment description array index
    color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_reference;

    //Depth attachment, if there is one
    VkAttachmentDescription depth_attachment{};
    depth_attachment.format = context->device.depth_format;
    depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    attachment_descriptions[1] = depth_attachment;

    //Depth attachment reference
    VkAttachmentReference depth_attachment_reference;
    depth_attachment_reference.attachment = 1;
    depth_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    //TODO: other attachment types (input, resolve, preserve)

    //Depth stencil data.
    subpass.pDepthStencilAttachment = &depth_attachment_reference;

    //Input from a shader
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = 0;

    //Attachments used for multisampling colour attachments
    subpass.pResolveAttachments = 0;

    //Attachments not used in this subpass, but must be preserved for the next.
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = 0;

    //Render pass dependencies. TODO: make this configurable.
    VkSubpassDependency dependency;
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dependencyFlags = 0;

    //Render pass create.
    VkRenderPassCreateInfo render_pass_create_info{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    render_pass_create_info.attachmentCount = attachment_description_count;
    render_pass_create_info.pAttachments = attachment_descriptions;
    render_pass_create_info.subpassCount = 1;
    render_pass_create_info.pSubpasses = &subpass;
    render_pass_create_info.dependencyCount = 1;
    render_pass_create_info.pDependencies = &dependency;
    render_pass_create_info.pNext = 0;
    render_pass_create_info.flags = 0;

    VK_CHECK(vkCreateRenderPass(context->device.logical_device,&render_pass_create_info,context->allocator,&handle));

}

void vulkan_renderpass::destroy(vulkan_context* context){
    if(handle){
        vkDestroyRenderPass(context->device.logical_device, handle, context->allocator);
        handle=  VK_NULL_HANDLE;
    }
}

void vulkan_renderpass::begin(vulkan_command_buffer*command_buffer, VkFramebuffer frame_buffer){

    VkRenderPassBeginInfo begin_info{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    begin_info.renderPass = handle;
    begin_info.framebuffer = frame_buffer;
    begin_info.renderArea.offset.x = (i32)x;
    begin_info.renderArea.offset.y = (i32)y;
    begin_info.renderArea.extent.width = (i32)w;
    begin_info.renderArea.extent.height = (i32)h;

    VkClearValue clear_values[2];
    kzero_memory(clear_values, sizeof(VkClearValue)*2);
    clear_values[0].color.float32[0] = r;
    clear_values[0].color.float32[1] = g;
    clear_values[0].color.float32[2] = b;
    clear_values[0].color.float32[3] = a;
    clear_values[1].depthStencil.depth = depth;
    clear_values[1].depthStencil.stencil = stencil;

    begin_info.clearValueCount =2;
    begin_info.pClearValues = clear_values;

    vkCmdBeginRenderPass(command_buffer->handle, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
    command_buffer->state = COMMAND_BUFFER_STATE_IN_RENDER_PASS;
}

void vulkan_renderpass::end(vulkan_command_buffer*command_buffer){
    vkCmdEndRenderPass(command_buffer->handle);
    command_buffer->state = COMMAND_BUFFER_STATE_RECORDING;
}
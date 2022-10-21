#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "engine.h"

VkShaderModule createShaderModule(Engine* engine, char* path) {
  VkShaderModule shaderModule;
  FileData fileData = readFile(path);
  VkShaderModuleCreateInfo createInfo;
  memset(&createInfo, 0, sizeof(VkShaderModuleCreateInfo));
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = fileData.size;
  createInfo.pCode = (uint32_t*)fileData.data;
  if (vkCreateShaderModule(engine->device, &createInfo, NULL, &shaderModule) != VK_SUCCESS) {
    printf("Shader creation failed!\n");
    exit(1);
  }
  free(fileData.data);
  return shaderModule;
}

VkPipeline pipelineCreate(Engine* engine) {
  VkPipeline pipeline;

  // Shaders
  VkShaderModule vertShaderModule = createShaderModule(engine, "shaders/triangle.vert.spv");
  VkShaderModule fragShaderModule = createShaderModule(engine, "shaders/triangle.frag.spv");

  VkPipelineShaderStageCreateInfo vertShaderStageInfo;
  memset(&vertShaderStageInfo, 0, sizeof(VkPipelineShaderStageCreateInfo));
  vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageInfo.module = vertShaderModule;
  vertShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo fragShaderStageInfo;
  memset(&fragShaderStageInfo, 0, sizeof(VkPipelineShaderStageCreateInfo));
  fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = fragShaderModule;
  fragShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

  // Vertex input
  VkPipelineVertexInputStateCreateInfo vertexInputInfo;
  memset(&vertexInputInfo, 0, sizeof(VkPipelineVertexInputStateCreateInfo));
  vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexBindingDescriptionCount = 0;
  vertexInputInfo.vertexAttributeDescriptionCount = 0;

  VkPipelineInputAssemblyStateCreateInfo inputAssembly;
  memset(&inputAssembly, 0, sizeof(VkPipelineInputAssemblyStateCreateInfo));
  inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  // Rassterizer
  VkPipelineRasterizationStateCreateInfo rasterizer;
  memset(&rasterizer, 0, sizeof(VkPipelineRasterizationStateCreateInfo));
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rasterizer.depthBiasEnable = VK_FALSE;

  // Multisampling
  VkPipelineMultisampleStateCreateInfo multisampling;
  memset(&multisampling, 0, sizeof(VkPipelineMultisampleStateCreateInfo));
  multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  // Color blending
  VkPipelineColorBlendAttachmentState colorBlendAttachment;
  memset(&colorBlendAttachment, 0, sizeof(VkPipelineColorBlendAttachmentState));
  colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = VK_FALSE;

  VkPipelineColorBlendStateCreateInfo colorBlending;
  memset(&colorBlending, 0, sizeof(VkPipelineColorBlendStateCreateInfo));
  colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = VK_LOGIC_OP_COPY;
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;
  colorBlending.blendConstants[0] = 0.0f;
  colorBlending.blendConstants[1] = 0.0f;
  colorBlending.blendConstants[2] = 0.0f;
  colorBlending.blendConstants[3] = 0.0f;

  // Dynamic viewport
  VkPipelineViewportStateCreateInfo viewportState;
  memset(&viewportState, 0, sizeof(VkPipelineViewportStateCreateInfo));
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.scissorCount = 1;

  VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
  VkPipelineDynamicStateCreateInfo dynamicState;
  memset(&dynamicState, 0, sizeof(VkPipelineDynamicStateCreateInfo));
  dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicState.dynamicStateCount = 2;
  dynamicState.pDynamicStates = dynamicStates;

  // Depth stencil
  VkPipelineDepthStencilStateCreateInfo depthStencil;
  memset(&depthStencil, 0, sizeof(VkPipelineDepthStencilStateCreateInfo));
  depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthStencil.depthTestEnable = VK_TRUE;
  depthStencil.depthWriteEnable = VK_TRUE;
  depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
  depthStencil.depthBoundsTestEnable = VK_FALSE;
  depthStencil.minDepthBounds = 0.0f;  // Optional
  depthStencil.maxDepthBounds = 1.0f;  // Optional
  depthStencil.stencilTestEnable = VK_FALSE;

  // Create the pipeline
  VkGraphicsPipelineCreateInfo pipelineInfo;
  memset(&pipelineInfo, 0, sizeof(VkGraphicsPipelineCreateInfo));
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shaderStages;
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.pDynamicState = &dynamicState;
  pipelineInfo.layout = engine->pipelineLayout;
  pipelineInfo.renderPass = engine->renderPass;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
  pipelineInfo.basePipelineIndex = -1;
  pipelineInfo.pDepthStencilState = &depthStencil;

  if (vkCreateGraphicsPipelines(engine->device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &pipeline) != VK_SUCCESS) {
    printf("Failed to create graphics pipeline!\n");
    exit(EXIT_FAILURE);
  }

  vkDestroyShaderModule(engine->device, fragShaderModule, NULL);
  vkDestroyShaderModule(engine->device, vertShaderModule, NULL);

  return pipeline;
}

// Minimal pipeline helper for vklite: load SPIR-V, create shader modules and a graphics pipeline
#include "vklite.h"
#include <fstream>
#include <vector>
#include <iostream>
#include <cstdio>
#include <memory>
#include <GLFW/glfw3.h>

namespace vklite {

// Helper to run a shell command and capture combined stdout/stderr.
// Run a command that writes binary to stdout and redirects stderr to an
// error file. Capture stdout as binary into a vector<char> and return the
// process exit code. If errPath is provided, stderr is redirected there.
static int runCommandCaptureBinary(const std::string& cmd, const std::string& errPath, std::vector<char>& outBytes) {
  outBytes.clear();
  // Ensure stderr is redirected to errPath
  std::string full = cmd + " 2> " + errPath;
  FILE* pipe = popen(full.c_str(), "r");
  if (!pipe) return -1;
  const size_t bufSize = 4096;
  std::unique_ptr<char[]> buffer(new char[bufSize]);
  size_t n = 0;
  while (true) {
    size_t r = fread(buffer.get(), 1, bufSize, pipe);
    if (r > 0) {
      outBytes.insert(outBytes.end(), buffer.get(), buffer.get() + r);
    }
    if (r < bufSize) break;
  }
  int rc = pclose(pipe);
  if (WIFEXITED(rc)) return WEXITSTATUS(rc);
  return rc;
}

void Context::destroyPipeline(Pipeline* p) {
  if (!p) return;
  if (device != VK_NULL_HANDLE) {
    if (p->pipeline != VK_NULL_HANDLE) vkDestroyPipeline(device, p->pipeline, nullptr);
    if (p->layout != VK_NULL_HANDLE) vkDestroyPipelineLayout(device, p->layout, nullptr);
    if (p->frag != VK_NULL_HANDLE) vkDestroyShaderModule(device, p->frag, nullptr);
    if (p->vert != VK_NULL_HANDLE) vkDestroyShaderModule(device, p->vert, nullptr);
  }
  delete p;
}

void Context::recordPipelineDraw(Pipeline* p, Window* window, VkCommandBuffer cmdBuf) {
  if (!p || !window || cmdBuf == VK_NULL_HANDLE) return;
  // Set viewport and scissor for this window using its framebuffer size
  int fbw = 0, fbh = 0;
  if (window->handle) glfwGetFramebufferSize(static_cast<GLFWwindow*>(window->handle), &fbw, &fbh);
  VkViewport vp{};
  vp.x = 0.0f;
  vp.y = 0.0f;
  vp.width = fbw > 0 ? static_cast<float>(fbw) : static_cast<float>(window->width);
  vp.height = fbh > 0 ? static_cast<float>(fbh) : static_cast<float>(window->height);
  vp.minDepth = 0.0f;
  vp.maxDepth = 1.0f;
  vkCmdSetViewport(cmdBuf, 0, 1, &vp);

  VkRect2D sc{};
  sc.offset = {0, 0};
  sc.extent = { static_cast<uint32_t>(vp.width), static_cast<uint32_t>(vp.height) };
  vkCmdSetScissor(cmdBuf, 0, 1, &sc);

  // Debug logging: print viewport/scissor and draw info
  std::cerr << "recordPipelineDraw: pipeline=" << p << " vertexCount=" << p->vertexCount
            << " viewport=" << vp.width << "x" << vp.height
            << " scissor=" << sc.extent.width << "x" << sc.extent.height << std::endl;

  // Bind pipeline and issue a non-indexed draw using vertexCount
  vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, p->pipeline);
  std::cerr << "recordPipelineDraw: vkCmdDraw called\n";
  vkCmdDraw(cmdBuf, p->vertexCount, 1, 0, 0);
}

Context::Pipeline* Context::createPipelineFromGlsl(const std::string& vertGlsl, const std::string& fragGlsl, uint32_t vertexCount, VkFormat colorFormat) {
  if (device == VK_NULL_HANDLE) return nullptr;
  // Write GLSL to temporary files in /tmp and invoke glslangValidator
  std::string vertPath = "/tmp/vklite_tmp_vert.glsl";
  std::string fragPath = "/tmp/vklite_tmp_frag.glsl";
  {
    std::ofstream ov(vertPath, std::ios::binary);
    if (!ov) return nullptr;
    ov.write(vertGlsl.data(), vertGlsl.size());
  }
  {
    std::ofstream of(fragPath, std::ios::binary);
    if (!of) return nullptr;
    of.write(fragGlsl.data(), fragGlsl.size());
  }

  // Compile to SPIR-V and capture binary output directly to memory (no .spv files)
  std::string vertErr = "/tmp/vklite_tmp_vert.err";
  std::string fragErr = "/tmp/vklite_tmp_frag.err";
  std::string cmd1 = "glslangValidator -V -S vert " + vertPath + " -o -";
  std::string cmd2 = "glslangValidator -V -S frag " + fragPath + " -o -";
  std::vector<char> vbytes;
  std::vector<char> fbytes;
  int r1 = runCommandCaptureBinary(cmd1, vertErr, vbytes);
  int r2 = runCommandCaptureBinary(cmd2, fragErr, fbytes);
  if (r1 != 0 || r2 != 0) {
    std::string out1, out2;
    // Read error files for diagnostics
    {
      std::ifstream e(vertErr);
      out1.assign((std::istreambuf_iterator<char>(e)), std::istreambuf_iterator<char>());
    }
    {
      std::ifstream e(fragErr);
      out2.assign((std::istreambuf_iterator<char>(e)), std::istreambuf_iterator<char>());
    }
    std::cerr << "glslangValidator failed:\nVERT:\n" << out1 << "\nFRAG:\n" << out2 << std::endl;
    // remove error files
    std::remove(vertErr.c_str());
    std::remove(fragErr.c_str());
    return nullptr;
  }

  std::cerr << "createPipelineFromGlsl: colorFormat=" << static_cast<int>(colorFormat) << "\n";

  // SPIR-V code must be a multiple of 4 bytes
  if (vbytes.size() % 4 != 0 || fbytes.size() % 4 != 0) {
    std::cerr << "Compiled SPIR-V size invalid (not multiple of 4)" << std::endl;
    return nullptr;
  }
  VkShaderModuleCreateInfo smci{};
  smci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  smci.codeSize = vbytes.size();
  smci.pCode = reinterpret_cast<const uint32_t*>(vbytes.data());
  VkShaderModule vertModule = VK_NULL_HANDLE;
  if (vkCreateShaderModule(device, &smci, nullptr, &vertModule) != VK_SUCCESS) return nullptr;

  smci.codeSize = fbytes.size();
  smci.pCode = reinterpret_cast<const uint32_t*>(fbytes.data());
  VkShaderModule fragModule = VK_NULL_HANDLE;
  if (vkCreateShaderModule(device, &smci, nullptr, &fragModule) != VK_SUCCESS) {
    vkDestroyShaderModule(device, vertModule, nullptr);
    return nullptr;
  }

  // Pipeline layout (no descriptors for this simple demo)
  VkPipelineLayoutCreateInfo plci{};
  plci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  plci.setLayoutCount = 0;
  plci.pushConstantRangeCount = 0;
  VkPipelineLayout layout = VK_NULL_HANDLE;
  if (vkCreatePipelineLayout(device, &plci, nullptr, &layout) != VK_SUCCESS) {
    vkDestroyShaderModule(device, fragModule, nullptr);
    vkDestroyShaderModule(device, vertModule, nullptr);
    return nullptr;
  }

  // Shader stages
  VkPipelineShaderStageCreateInfo stages[2]{};
  stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
  stages[0].module = vertModule;
  stages[0].pName = "main";
  stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  stages[1].module = fragModule;
  stages[1].pName = "main";

  // Vertex input (none; we'll use gl_VertexIndex in the shader)
  VkPipelineVertexInputStateCreateInfo vi{};
  vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

  VkPipelineInputAssemblyStateCreateInfo ia{};
  ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  ia.primitiveRestartEnable = VK_FALSE;

  VkPipelineRasterizationStateCreateInfo rs{};
  rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rs.polygonMode = VK_POLYGON_MODE_FILL;
  // Disable back-face culling for debug to ensure geometry isn't culled
  rs.cullMode = VK_CULL_MODE_NONE;
  rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rs.lineWidth = 1.0f;

  VkPipelineMultisampleStateCreateInfo ms{};
  ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  VkPipelineColorBlendAttachmentState ca{};
  ca.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  ca.blendEnable = VK_FALSE;

  VkPipelineColorBlendStateCreateInfo cb{};
  cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  cb.attachmentCount = 1;
  cb.pAttachments = &ca;

  // Viewport state (we'll make viewport and scissor dynamic so we can set
  // them per-window at draw time)
  VkPipelineViewportStateCreateInfo vp{};
  vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  vp.viewportCount = 1;
  vp.scissorCount = 1;

  VkDynamicState dynStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
  VkPipelineDynamicStateCreateInfo dync{};
  dync.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dync.dynamicStateCount = 2;
  dync.pDynamicStates = dynStates;

  VkGraphicsPipelineCreateInfo gpi{};
  gpi.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  gpi.stageCount = 2;
  gpi.pStages = stages;
  gpi.pVertexInputState = &vi;
  gpi.pInputAssemblyState = &ia;
  gpi.pViewportState = &vp;
  gpi.pRasterizationState = &rs;
  gpi.pMultisampleState = &ms;
  gpi.pColorBlendState = &cb;
  gpi.pDynamicState = &dync;
  gpi.layout = layout;
  // For dynamic rendering pipelines, applications must provide
  // VkPipelineRenderingCreateInfo in the pNext chain specifying the
  // color attachment formats that will be used during rendering.
  VkPipelineRenderingCreateInfo prci{};
  prci.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
  prci.viewMask = 0;
  prci.colorAttachmentCount = 1;
  prci.pColorAttachmentFormats = &colorFormat;
  gpi.pNext = &prci;
  gpi.renderPass = VK_NULL_HANDLE; // dynamic rendering will be used

  VkPipeline pipeline = VK_NULL_HANDLE;
  VkResult r = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &gpi, nullptr, &pipeline);
  if (r != VK_SUCCESS) {
    vkDestroyPipelineLayout(device, layout, nullptr);
    vkDestroyShaderModule(device, fragModule, nullptr);
    vkDestroyShaderModule(device, vertModule, nullptr);
    return nullptr;
  }

  Pipeline* p = new Pipeline();
  p->pipeline = pipeline;
  p->layout = layout;
  p->vert = vertModule;
  p->frag = fragModule;
  p->vertexCount = vertexCount;
  return p;
}

} // namespace vklite

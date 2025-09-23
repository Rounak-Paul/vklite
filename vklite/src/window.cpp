// window.cpp - window management implementation for vklite
#include "vklite.h"
#include "window.h"
#include <GLFW/glfw3.h>
#include <vector>
#include <iostream>
#include <vulkan/vulkan.h>
#include <algorithm>

namespace vklite {

Window* Context::createWindow(int width, int height, const std::string& title) {
  GLFWwindow* win = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
  if (!win) {
    // Creation failed; surface-level errors can be retrieved by the application
    // via the GLFW error callback if needed. Return nullptr to indicate failure.
    return nullptr;
  }
  auto w = std::make_unique<Window>();
  w->handle = win;
  w->width = width;
  w->height = height;
  w->title = title;
  windows.push_back(std::move(w));
  Window* created = windows.back().get();

  // Create a VkSurfaceKHR for this window and a swapchain
  if (!createSurfaceForWindow(created)) {
    destroyWindow(created);
    return nullptr;
  }
  if (!createSwapchainForWindow(created)) {
    destroyWindow(created);
    return nullptr;
  }
  return windows.back().get();
}

void Context::destroyWindow(Window* window) {
  if (!window || !window->handle) return;
  // Destroy swapchain and surface
  destroySwapchainForWindow(window);
  if (window->surface != VK_NULL_HANDLE && instance != VK_NULL_HANDLE) {
    vkDestroySurfaceKHR(instance, window->surface, nullptr);
    window->surface = VK_NULL_HANDLE;
  }
  GLFWwindow* gw = static_cast<GLFWwindow*>(window->handle);
  glfwDestroyWindow(gw);
  window->handle = nullptr;
  for (auto it = windows.begin(); it != windows.end(); ++it) {
    if (it->get() == window) {
      windows.erase(it);
      break;
    }
  }
}

bool Context::isWindowOpen(const Window* window) const {
  if (!window || !window->handle) return false;
  return !glfwWindowShouldClose(static_cast<GLFWwindow*>(window->handle));
}

void Context::pollEvents() {
  glfwPollEvents();
}

void Context::runMainLoop() {
  while (true) {
    pollEvents();
    // Render each window
    for (auto& up : windows) {
      Window* w = up.get();
      if (w && w->handle) renderWindow(w);
    }

    // Collect windows requested to close
    std::vector<Window*> toDestroy;
    for (auto& up : windows) {
      Window* w = up.get();
      if (w && w->handle && glfwWindowShouldClose(static_cast<GLFWwindow*>(w->handle))) {
        toDestroy.push_back(w);
      }
    }
    for (Window* w : toDestroy) destroyWindow(w);
    if (windows.empty()) break;
  }
}

bool Context::createSurfaceForWindow(Window* window) {
  if (!window || !window->handle || instance == VK_NULL_HANDLE) return false;
  GLFWwindow* gw = static_cast<GLFWwindow*>(window->handle);
  VkSurfaceKHR surface = VK_NULL_HANDLE;
  VkResult r = glfwCreateWindowSurface(instance, gw, nullptr, &surface);
  if (r != VK_SUCCESS) return false;
  window->surface = surface;
  return true;
}

bool Context::createSwapchainForWindow(Window* window) {
  if (!window || window->surface == VK_NULL_HANDLE || physicalDevice == VK_NULL_HANDLE || device == VK_NULL_HANDLE) return false;

  // Query surface capabilities and formats
  VkSurfaceCapabilitiesKHR caps{};
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, window->surface, &caps);

  uint32_t fmtCount = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, window->surface, &fmtCount, nullptr);
  if (fmtCount == 0) return false;
  std::vector<VkSurfaceFormatKHR> formats(fmtCount);
  vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, window->surface, &fmtCount, formats.data());

  // Prefer SRGB formats if available
  VkSurfaceFormatKHR chosenFormat = formats[0];
  for (auto &f : formats) {
    if (f.format == VK_FORMAT_B8G8R8A8_SRGB || f.format == VK_FORMAT_R8G8B8A8_SRGB) {
      chosenFormat = f;
      break;
    }
  }

  // Choose present mode (prefer MAILBOX, fallback to FIFO)
  uint32_t pmCount = 0;
  vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, window->surface, &pmCount, nullptr);
  std::vector<VkPresentModeKHR> presentModes(pmCount);
  vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, window->surface, &pmCount, presentModes.data());
  VkPresentModeKHR chosenPresent = VK_PRESENT_MODE_FIFO_KHR;
  for (auto &p : presentModes) {
    if (p == VK_PRESENT_MODE_MAILBOX_KHR) { chosenPresent = p; break; }
  }

  // Determine swap extent
  // Use framebuffer size (pixel dimensions) to account for high-DPI displays
  int fbw = 0, fbh = 0;
  if (window->handle) {
    glfwGetFramebufferSize(static_cast<GLFWwindow*>(window->handle), &fbw, &fbh);
  }
  VkExtent2D extent;
  if (caps.currentExtent.width != UINT32_MAX) {
    extent = caps.currentExtent;
  } else {
    extent.width = static_cast<uint32_t>(fbw > 0 ? fbw : window->width);
    extent.height = static_cast<uint32_t>(fbh > 0 ? fbh : window->height);
    extent.width = std::max(caps.minImageExtent.width, std::min(caps.maxImageExtent.width, extent.width));
    extent.height = std::max(caps.minImageExtent.height, std::min(caps.maxImageExtent.height, extent.height));
  }

  uint32_t imageCount = caps.minImageCount + 1;
  if (caps.maxImageCount > 0 && imageCount > caps.maxImageCount) imageCount = caps.maxImageCount;

  VkSwapchainCreateInfoKHR scCreate{};
  scCreate.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  scCreate.surface = window->surface;
  scCreate.minImageCount = imageCount;
  scCreate.imageFormat = chosenFormat.format;
  scCreate.imageColorSpace = chosenFormat.colorSpace;
  scCreate.imageExtent = extent;
  scCreate.imageArrayLayers = 1;
  scCreate.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  scCreate.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  scCreate.preTransform = caps.currentTransform;
  scCreate.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  scCreate.presentMode = chosenPresent;
  scCreate.clipped = VK_TRUE;
  scCreate.oldSwapchain = VK_NULL_HANDLE;

  VkResult r = vkCreateSwapchainKHR(device, &scCreate, nullptr, &window->swapchain);
  if (r != VK_SUCCESS) return false;

  // Retrieve images
  uint32_t scImgCount = 0;
  vkGetSwapchainImagesKHR(device, window->swapchain, &scImgCount, nullptr);
  window->swapchainImages.resize(scImgCount);
  vkGetSwapchainImagesKHR(device, window->swapchain, &scImgCount, window->swapchainImages.data());

  // Create image views
  window->swapchainImageViews.clear();
  window->swapchainImageViews.reserve(scImgCount);
  for (auto img : window->swapchainImages) {
    VkImageViewCreateInfo iv{};
    iv.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    iv.image = img;
    iv.viewType = VK_IMAGE_VIEW_TYPE_2D;
    iv.format = chosenFormat.format;
    iv.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    iv.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    iv.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    iv.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    iv.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    iv.subresourceRange.baseMipLevel = 0;
    iv.subresourceRange.levelCount = 1;
    iv.subresourceRange.baseArrayLayer = 0;
    iv.subresourceRange.layerCount = 1;
    VkImageView view;
    if (vkCreateImageView(device, &iv, nullptr, &view) != VK_SUCCESS) return false;
    window->swapchainImageViews.push_back(view);
  }

  // Remember the swapchain image format for pipeline creation
  window->swapchainFormat = chosenFormat.format;
  std::cerr << "createSwapchainForWindow: chosenFormat=" << static_cast<int>(chosenFormat.format) << "\n";

  // Create command pool and buffer
  VkCommandPoolCreateInfo cp{};
  cp.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  cp.queueFamilyIndex = graphicsQueueFamily;
  cp.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  if (vkCreateCommandPool(device, &cp, nullptr, &window->commandPool) != VK_SUCCESS) return false;

  VkCommandBufferAllocateInfo cbi{};
  cbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  cbi.commandPool = window->commandPool;
  cbi.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  cbi.commandBufferCount = 1;
  if (vkAllocateCommandBuffers(device, &cbi, &window->commandBuffer) != VK_SUCCESS) return false;

  // Semaphores and fence
  VkSemaphoreCreateInfo semInfo{};
  semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  vkCreateSemaphore(device, &semInfo, nullptr, &window->imageAvailableSemaphore);
  vkCreateSemaphore(device, &semInfo, nullptr, &window->renderFinishedSemaphore);

  VkFenceCreateInfo fenceInfo{};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
  vkCreateFence(device, &fenceInfo, nullptr, &window->inFlightFence);

  // Dynamic rendering requires device-level function pointers
  if (!vkCmdBeginRenderingKHR || !vkCmdEndRenderingKHR) {
    // We expect dynamic rendering function pointers to be loaded; fail if not present
    return false;
  }

  return true;
}

void Context::destroySwapchainForWindow(Window* window) {
  if (!window) return;
  // If we don't have a valid device, just clear CPU-side structures and return.
  if (device == VK_NULL_HANDLE) {
    window->swapchainImageViews.clear();
    window->swapchainImages.clear();
    window->swapchain = VK_NULL_HANDLE;
    // Nothing else to do because device-specific objects cannot be destroyed.
    return;
  }

  // Wait for device idle before destroying per-window resources
  vkDeviceWaitIdle(device);
  for (auto iv : window->swapchainImageViews) {
    if (iv != VK_NULL_HANDLE) vkDestroyImageView(device, iv, nullptr);
  }
  window->swapchainImageViews.clear();
  if (window->commandPool != VK_NULL_HANDLE) {
    vkDestroyCommandPool(device, window->commandPool, nullptr);
    window->commandPool = VK_NULL_HANDLE;
  }
  if (window->imageAvailableSemaphore != VK_NULL_HANDLE) vkDestroySemaphore(device, window->imageAvailableSemaphore, nullptr);
  if (window->renderFinishedSemaphore != VK_NULL_HANDLE) vkDestroySemaphore(device, window->renderFinishedSemaphore, nullptr);
  if (window->inFlightFence != VK_NULL_HANDLE) vkDestroyFence(device, window->inFlightFence, nullptr);
  if (window->swapchain != VK_NULL_HANDLE) {
    vkDestroySwapchainKHR(device, window->swapchain, nullptr);
    window->swapchain = VK_NULL_HANDLE;
  }
  window->swapchainImages.clear();
}

// Minimal per-window render: acquire, clear via render pass, present
void Context::renderWindow(Window* window) {
  if (!window || window->swapchain == VK_NULL_HANDLE) return;

  if (!vkCmdBeginRenderingKHR || !vkCmdEndRenderingKHR) return; // dynamic rendering required

  // Wait for previous frame
  vkWaitForFences(device, 1, &window->inFlightFence, VK_TRUE, UINT64_MAX);
  vkResetFences(device, 1, &window->inFlightFence);

  uint32_t imageIndex = 0;
  VkResult r = vkAcquireNextImageKHR(device, window->swapchain, UINT64_MAX, window->imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
  std::cerr << "vkAcquireNextImageKHR result=" << r << " imageIndex=" << imageIndex << "\n";
  if (r != VK_SUCCESS && r != VK_SUBOPTIMAL_KHR) return;

  // Record command buffer: transition image layout and begin dynamic rendering
  VkCommandBufferBeginInfo bi{};
  bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  vkResetCommandBuffer(window->commandBuffer, 0);
  vkBeginCommandBuffer(window->commandBuffer, &bi);

  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.srcAccessMask = 0;
  barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = window->swapchainImages[imageIndex];
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;

  vkCmdPipelineBarrier(window->commandBuffer,
                       VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                       VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                       0, 0, nullptr, 0, nullptr, 1, &barrier);

  VkRenderingAttachmentInfoKHR colorAtt{};
  colorAtt.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
  colorAtt.imageView = window->swapchainImageViews[imageIndex];
  colorAtt.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  // Always clear to opaque red for presentation test
  VkClearValue clearColor{};
  clearColor.color.float32[0] = 1.0f;
  clearColor.color.float32[1] = 0.0f;
  clearColor.color.float32[2] = 0.0f;
  clearColor.color.float32[3] = 1.0f;
  colorAtt.clearValue = clearColor;
  // Use clear as the load operation and store results to the image
  colorAtt.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAtt.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

  VkRenderingInfoKHR ri{};
  ri.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
  ri.flags = 0;
  ri.renderArea.offset = {0,0};
  int fbw=0, fbh=0;
  if (window->handle) glfwGetFramebufferSize(static_cast<GLFWwindow*>(window->handle), &fbw, &fbh);
  ri.renderArea.extent = { static_cast<uint32_t>(fbw>0?fbw:window->width), static_cast<uint32_t>(fbh>0?fbh:window->height) };
  ri.layerCount = 1;
  ri.colorAttachmentCount = 1;
  ri.pColorAttachments = &colorAtt;

  // Create a small host-visible staging buffer to copy the swapchain image into
  // so we can inspect pixels for debugging.
  VkBuffer stagingBuffer = VK_NULL_HANDLE;
  VkDeviceMemory stagingMemory = VK_NULL_HANDLE;
  VkDeviceSize imageSize = static_cast<VkDeviceSize>(ri.renderArea.extent.width) * ri.renderArea.extent.height * 4; // RGBA8
  {
    VkBufferCreateInfo bci{};
    bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bci.size = imageSize;
    bci.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateBuffer(device, &bci, nullptr, &stagingBuffer) != VK_SUCCESS) {
      stagingBuffer = VK_NULL_HANDLE;
    }
  }
  if (stagingBuffer != VK_NULL_HANDLE) {
    VkMemoryRequirements req{};
    vkGetBufferMemoryRequirements(device, stagingBuffer, &req);
    VkMemoryAllocateInfo mai{};
    mai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mai.allocationSize = req.size;
    // Find a host visible memory type
    VkPhysicalDeviceMemoryProperties memProps{};
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);
    uint32_t memTypeIndex = UINT32_MAX;
    for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
      if ((req.memoryTypeBits & (1u << i)) && (memProps.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) == (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
        memTypeIndex = i;
        break;
      }
    }
    if (memTypeIndex == UINT32_MAX) {
      // try any host visible
      for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
        if ((req.memoryTypeBits & (1u << i)) && (memProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
          memTypeIndex = i; break;
        }
      }
    }
    if (memTypeIndex != UINT32_MAX) {
      mai.memoryTypeIndex = memTypeIndex;
      if (vkAllocateMemory(device, &mai, nullptr, &stagingMemory) != VK_SUCCESS) {
        vkDestroyBuffer(device, stagingBuffer, nullptr);
        stagingBuffer = VK_NULL_HANDLE;
        stagingMemory = VK_NULL_HANDLE;
      } else {
        vkBindBufferMemory(device, stagingBuffer, stagingMemory, 0);
      }
    } else {
      // no suitable memory type found
      vkDestroyBuffer(device, stagingBuffer, nullptr);
      stagingBuffer = VK_NULL_HANDLE;
    }
  }

  // Begin/End dynamic rendering via loaded function pointers
  this->vkCmdBeginRenderingKHR(window->commandBuffer, &ri);
  // If the application attached a pipeline to this window, record its draw commands.
  if (window->pipeline) {
    Context::Pipeline* p = reinterpret_cast<Context::Pipeline*>(window->pipeline);
    // Use the convenience helper to record bind + draw
    this->recordPipelineDraw(p, window, window->commandBuffer);
  }
  this->vkCmdEndRenderingKHR(window->commandBuffer);

  // For debugging: copy image to staging buffer (if available) before presenting.
  if (stagingBuffer != VK_NULL_HANDLE) {
    // Transition image from COLOR_ATTACHMENT_OPTIMAL -> TRANSFER_SRC_OPTIMAL
    VkImageMemoryBarrier copyBarrier = barrier;
    copyBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    copyBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    copyBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    copyBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    vkCmdPipelineBarrier(window->commandBuffer,
                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         0, 0, nullptr, 0, nullptr, 1, &copyBarrier);

    VkBufferImageCopy bic{};
    bic.bufferOffset = 0;
    bic.bufferRowLength = 0;
    bic.bufferImageHeight = 0;
    bic.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    bic.imageSubresource.mipLevel = 0;
    bic.imageSubresource.baseArrayLayer = 0;
    bic.imageSubresource.layerCount = 1;
    bic.imageOffset = {0,0,0};
    bic.imageExtent = { ri.renderArea.extent.width, ri.renderArea.extent.height, 1 };

    vkCmdCopyImageToBuffer(window->commandBuffer, window->swapchainImages[imageIndex], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stagingBuffer, 1, &bic);

    // Transition image from TRANSFER_SRC_OPTIMAL -> PRESENT_SRC_KHR
    VkImageMemoryBarrier presBarrier = copyBarrier;
    presBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    presBarrier.dstAccessMask = 0;
    presBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    presBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    vkCmdPipelineBarrier(window->commandBuffer,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                         0, 0, nullptr, 0, nullptr, 1, &presBarrier);
  } else {
    // No staging buffer available; transition directly to present
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = 0;
    barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    vkCmdPipelineBarrier(window->commandBuffer,
                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                         0, 0, nullptr, 0, nullptr, 1, &barrier);
  }

  vkEndCommandBuffer(window->commandBuffer);

  VkSemaphore waitSemaphores[] = { window->imageAvailableSemaphore };
  VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
  VkSemaphore signalSemaphores[] = { window->renderFinishedSemaphore };

  VkSubmitInfo submit{};
  submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit.waitSemaphoreCount = 1;
  submit.pWaitSemaphores = waitSemaphores;
  submit.pWaitDstStageMask = waitStages;
  submit.commandBufferCount = 1;
  submit.pCommandBuffers = &window->commandBuffer;
  submit.signalSemaphoreCount = 1;
  submit.pSignalSemaphores = signalSemaphores;

  VkResult submitRes = vkQueueSubmit(graphicsQueue, 1, &submit, window->inFlightFence);
  std::cerr << "vkQueueSubmit result=" << submitRes << "\n";
  if (submitRes != VK_SUCCESS) return;

  VkPresentInfoKHR present{};
  present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present.waitSemaphoreCount = 1;
  present.pWaitSemaphores = signalSemaphores;
  present.swapchainCount = 1;
  present.pSwapchains = &window->swapchain;
  present.pImageIndices = &imageIndex;

  VkResult presRes = vkQueuePresentKHR(graphicsQueue, &present);
  std::cerr << "vkQueuePresentKHR result=" << presRes << "\n";

  // For debugging: wait for device idle and inspect the staging buffer's center pixel
  if (stagingBuffer != VK_NULL_HANDLE && stagingMemory != VK_NULL_HANDLE) {
    vkDeviceWaitIdle(device);
    void* data = nullptr;
    vkMapMemory(device, stagingMemory, 0, VK_WHOLE_SIZE, 0, &data);
    if (data) {
      uint32_t w = ri.renderArea.extent.width;
      uint32_t h = ri.renderArea.extent.height;
      uint32_t cx = w / 2;
      uint32_t cy = h / 2;
      uint8_t* bytes = reinterpret_cast<uint8_t*>(data);
      size_t rowPitch = static_cast<size_t>(w) * 4;
      size_t idx = static_cast<size_t>(cy) * rowPitch + static_cast<size_t>(cx) * 4;
      if (idx + 3 < imageSize) {
        uint8_t r = 0, g = 0, b = 0, a = 0;
        // Interpret bytes according to the swapchain image format
        switch (window->swapchainFormat) {
          case VK_FORMAT_B8G8R8A8_UNORM:
          case VK_FORMAT_B8G8R8A8_SRGB:
            // Stored as B, G, R, A in memory
            b = bytes[idx + 0];
            g = bytes[idx + 1];
            r = bytes[idx + 2];
            a = bytes[idx + 3];
            break;
          case VK_FORMAT_R8G8B8A8_UNORM:
          case VK_FORMAT_R8G8B8A8_SRGB:
          default:
            // Default to R, G, B, A ordering
            r = bytes[idx + 0];
            g = bytes[idx + 1];
            b = bytes[idx + 2];
            a = bytes[idx + 3];
            break;
        }
        std::cerr << "Swapchain center pixel (interpreted RGBA) = (" << (int)r << "," << (int)g << "," << (int)b << "," << (int)a << ")\n";
      } else {
        std::cerr << "Staging buffer too small for center pixel readback\n";
      }
      vkUnmapMemory(device, stagingMemory);
    }
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingMemory, nullptr);
    stagingBuffer = VK_NULL_HANDLE;
    stagingMemory = VK_NULL_HANDLE;
  }
}

} // namespace vklite
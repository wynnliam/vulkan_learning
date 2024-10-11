// Illtyd Wynn, 8/26/2024, Vulkan Learning

#ifndef APPLICATION_H
#define APPLICATION_H

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <cstdlib>
#include <optional>

const uint32_t WINDOW_W = 800;
const uint32_t WINDOW_H = 600;

// When looking for a suitable physical device, we need to look
// for one that supports the types of commands we want to submit.
// We do this by looking at the types of queues (queue families),
// to find one that supports the commands we need.
struct queue_family_indices {
  // When you query the queue families for a device, you put them
  // in vector. The graphics_family is the index into that vector.
  // So the implication is that the order of the list is always the
  // same.
  std::optional<uint32_t> graphics_family;
  // Although Vulkan itself supports window system integration,
  // the individual device may not. Thus, for a device to be viable,
  // we need to know that it supports it.
  std::optional<uint32_t> present_family;
};

struct application {
  application();

  GLFWwindow* window;

  // An instance is essentially like a handle on Vulkan.
  // It basically describes what features of the Vulkan API
  // your application uses.
  VkInstance vulkan_instance;
  // Vulkan is platform agnostic. We thus need to use the Window
  // System Integration (WSI) extension to interface Vulkan with
  // our windowing system. The extension exposes the VkSurfaceKHR
  // which is an abstract surface to present images to. Note that
  // the tutorial shows how you'd create a Windows specific surface.
  // In our case we will rely on GLFW to do it.
  VkSurfaceKHR surface;
  // Handle for debug callbacks, used to pass debug messages
  // to provided debug callbacks.
  VkDebugUtilsMessengerEXT debug_messenger;
  // Stores the graphics card that we use to do work. You can
  // have multiple devices used simultaneously, but we will only
  // deal with one.
  VkPhysicalDevice physical_device;
  // The logical device that interfaces with our actual physical device.
  VkDevice device;
  // The command queue to process any commands we want to send to
  // the GPU.
  VkQueue graphics_queue;
  // A command queue for presenting images to the surface.
  VkQueue present_queue;
};

//
// APPLICATION ROUTINES
//

void run_application(application* app);

void init_window(application* app);

void init_vulkan(application* app);
void create_vulkan_instance(application* app);
void setup_debug_messenger(application* app);
void create_surface(application* app);
void pick_physical_device(application* app);
void create_logical_device(application* app);

void application_main_loop(application* app);

void application_cleanup(application* app);

//
// QUEUE FAMILY INDICES ROUTINES
//

bool is_complete(const queue_family_indices& queue_fam);

#endif


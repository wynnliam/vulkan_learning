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
};

struct application {
  application();

  GLFWwindow* window;

  // An instance is essentially like a handle on Vulkan.
  // It basically describes what features of the Vulkan API
  // your application uses.
  VkInstance vulkan_instance;
  // Handle for debug callbacks, used to pass debug messages
  // to provided debug callbacks.
  VkDebugUtilsMessengerEXT debug_messenger;
  // Stores the graphics card that we use to do work. You can
  // have multiple devices used simultaneously, but we will only
  // deal with one.
  VkPhysicalDevice physical_device;
};

//
// APPLICATION ROUTINES
//

void run_application(application* app);

void init_window(application* app);

void init_vulkan(application* app);
void create_vulkan_instance(application* app);
void setup_debug_messenger(application* app);
void pick_physical_device(application* app);

void application_main_loop(application* app);

void application_cleanup(application* app);

//
// QUEUE FAMILY INDICES ROUTINES
//

bool is_complete(const queue_family_indices& queue_fam);

#endif


#include "application.h"

#include <stdexcept>
#include <vector>
#include <iostream>
#include <cstring>
#include <algorithm>
#include <iterator>

using namespace std;

//
// Specify Validation Layer stuff.
//

//
// The philosophy of the Vulkan API is minimal driver overhead. As such,
// by default there is minimal error checking done. If the API fails for
// whatever reason, it will simply crash without any information most of
// the time. And furthermore, since Vulkan is very explicit about setting
// everything up, it is very easy to make mistakes.
//
// Despite this, Vulkan *does* have a method for handling errors that is
// called "validation layers". Validation layers are optional components
// that hook into Vulkan function calls to apply additional operations.
// These include things like checking values of parameters against the
// specification to detect mistakes; track resource creation and deletion
// to check for memory leaks; logging stuff; checking thread safety; etc.
//
// Briefly, they are implemented by essentially redefining the function
// you normally call. An abstract example might be:
//
// VkResult vkDoThing(const int example) {
//  if (example < 0) {
//    return VK_ERROR_OF_SOME_KIND;
//  }
//
//  return real_vkDoThing(example);
// }
//

// For our application, the VK_LAYER_KHRONOS_validation layer
// is sufficient and catches most bugs.
const vector<const char*> validation_layers = {
  "VK_LAYER_KHRONOS_validation"
};

// We only want validation layers if we are in debug mode.
#ifdef NDEBUG
const bool enable_validation_layers = false;
#else
const bool enable_validation_layers = true;
#endif

//
// If we want to use a debug callback, we need the function
// vkCreateDebugUtilsMessengerEXT. Unfortunately, this function
// is not automatically loaded. This is because the function is
// an extension function. To solve this, we create our own function
// that loads it. If loading is successful, we invoke it.
//
// Note we need a VkInstance since the debug messenger is specific to
// Vulkan instances.
//

VkResult create_debug_utils_messenger(
  VkInstance instance,
  const VkDebugUtilsMessengerCreateInfoEXT* create_info,
  const VkAllocationCallbacks* allocatior,
  VkDebugUtilsMessengerEXT* debug_messenger
) {
  PFN_vkVoidFunction result_func;
  PFN_vkCreateDebugUtilsMessengerEXT func;

  result_func = vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
  func = (PFN_vkCreateDebugUtilsMessengerEXT)result_func;

  if (func != NULL) {
    return func(instance, create_info, allocatior, debug_messenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void destroy_debug_utils_messenger(
  VkInstance instance,
  VkDebugUtilsMessengerEXT debug_messenger,
  const VkAllocationCallbacks* allocator
) {
  PFN_vkVoidFunction result;
  PFN_vkDestroyDebugUtilsMessengerEXT destroy;

  result = vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
  destroy = (PFN_vkDestroyDebugUtilsMessengerEXT)result;

  if (destroy != NULL) {
    destroy(instance, debug_messenger, allocator);
  }
}

//
// This is an example of a debug callback used by validation layers.
// The VKAPI_ATTR and VKAPI_CALL ensure the function has the right
// signature for Vulkan to call it.
//
// Message severity can be:
// - VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: Diagnostic message
// - VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: Informational.
// - VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: Behavior that isn't
//   neccessarily an error but likely a bug.
// - VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: Behavior that is invalid
//   and may cause crashes.
// The enumeration is set so that you can do numerical comparisons (see below)
//
// The Message type can be:
// - VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT: Some event has happened
//   that is unrelated to the specification or performance.
// - VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT: Something has happened
//   that violates the specification or indicates a possible mistake.
// - VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT: Potential
//   non-optimal use of Vulkan.
//
// The callback data contains the details of the message:
// - pMessage: Debug message as null-terminated string.
// - pObjects: Array of Vulkan object handles related to
//   the message.
// - objectCount: Number of objects in the array.
//
// Finally, we return whether the message should be aborted.
// If true, the call is aborted with the VK_ERROR_VALIDATION_FAILED_EXT
// error. Normally you should return VK_FALSE, since this is just
// used to test validation layers themselves.
//

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
  VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
  VkDebugUtilsMessageTypeFlagsEXT message_type,
  const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
  void* user_data
) {
  cerr << "validation layer: " << callback_data->pMessage << endl;

  if (message_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    cerr << "brother it's bad" << endl;
  }

  return VK_FALSE;
}

//
// APPLICATION IMPL.
//

// Returns true if all needed validation layers are supported
// on the platform.
bool check_validation_layer_support();
// Returns a list of all required extensions.
vector<const char*> get_required_extensions();
// A utility function to handle setting up the create info
// for the debug messenger.
void populate_debug_messenger_create_info(
  VkDebugUtilsMessengerCreateInfoEXT& create_info
);
// Returns true if a given physical device is suitable for our
// application.
bool is_device_suitable(VkPhysicalDevice device);
// Returns the queue types a given device supports.
queue_family_indices find_queue_families(VkPhysicalDevice device);

application::application() {
  window = NULL;

  physical_device = VK_NULL_HANDLE;
}

void run_application(application* app) {
  init_window(app);
  init_vulkan(app);
  application_main_loop(app);
  application_cleanup(app);
}

void init_window(application* app) {

  //
  // Initialize glfw and create a window which we can render objects to.
  //

  glfwInit();

  // Historically, GLFW relied on OpenGL to provide a context. But since we
  // are using Vulkan, we want to tell GLFW not to rely on OpenGL.
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  // For now, let us disable window resizing to keep things simple.
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  app->window = glfwCreateWindow(WINDOW_W, WINDOW_H, "Vulkan", NULL, NULL);
}

void init_vulkan(application* app) {
  if (enable_validation_layers && !check_validation_layer_support()) {
    throw runtime_error("Validation layers requested, but not available!");
  }

  create_vulkan_instance(app);
  setup_debug_messenger(app);
  pick_physical_device(app);
}

bool check_validation_layer_support() {
  uint32_t layer_count;
  vector<VkLayerProperties> available_layers;
  bool found_layer;

  //
  // First query the supported layers.
  //

  vkEnumerateInstanceLayerProperties(&layer_count, NULL);

  available_layers.resize(layer_count);
  vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());


  //
  // Next for each validation layer we need (vlayer),
  // check that it is in the available_layers. If it is not,
  // then return false. If we get through every possible
  // validation layer we need, then we can return true.
  //

  for (const char* vlayer : validation_layers) {
    found_layer = false;

    for (const auto& layer_properties : available_layers) {
      if (strcmp(vlayer, layer_properties.layerName) == 0) {
        found_layer = true;
        break;
      }
    }

    if (!found_layer) {
      return false;
    }
  }

  return true;
}

void create_vulkan_instance(application* app) {
  VkApplicationInfo app_info{};
  VkInstanceCreateInfo instance_create_info{};
  vector<const char*> extensions;
  uint32_t num_required_extensions;
  uint32_t num_supported_extensions;
  vector<VkExtensionProperties> supported_extensions;
  VkResult result;
  uint32_t num_validation_layers;
  VkDebugUtilsMessengerCreateInfoEXT debug_create_info;

  //
  // First, we must specify to the driver some basic information about our
  // application, which we specify in the app_info struct.
  //

  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "Hello Triangle";
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "No Engine";
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.apiVersion = VK_API_VERSION_1_0;

  //
  // Next we specify the parameters of our instance.
  // We put these in the VkInstanceCreateInfo.
  //

  // First get the required extensions from GLFW and
  // validation layers (if they are enabled)
  extensions = get_required_extensions();
  num_required_extensions = static_cast<uint32_t>(extensions.size());

  // This is basically a factory for specifying all the
  // parameters we want for our instance. Vulkan relies
  // on structs to specify function arguments, and not
  // just arguments themselves. In my opinion, this is
  // a better way of organizing clean code.
  instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_create_info.pApplicationInfo = &app_info;
  instance_create_info.enabledExtensionCount = num_required_extensions;
  instance_create_info.ppEnabledExtensionNames = extensions.data();

  // If validation layers are enabled, we want to specify them
  // in the create info.
  if (enable_validation_layers) {
    num_validation_layers = static_cast<uint32_t>(validation_layers.size());

    instance_create_info.enabledLayerCount = num_validation_layers;
    instance_create_info.ppEnabledLayerNames = validation_layers.data();

    // Set up the debug create info for creating a messenger for instance
    // creation and destruction. Note that we need to make a seperate
    // messenger from the debug_messenger so that we can get error
    // reporting in the creation and destroying of the instance itself.
    populate_debug_messenger_create_info(debug_create_info);
    instance_create_info.pNext = &debug_create_info;
  } else {
    instance_create_info.enabledLayerCount = 0;
    instance_create_info.pNext = NULL;
  }

  //
  // Finally attempt to create the instance. Note the general pattern
  // for Vulkan object creation is: pointer to creation info,
  // a callback for allocation, and a pointer to the resulting handle.
  //

  result = vkCreateInstance(
    &instance_create_info,
    NULL,
    &(app->vulkan_instance)
  );

  if (result != VK_SUCCESS) {
    throw runtime_error("Failed to create Vulkan instance!");
  }

  //
  // This next step is optional, and is purely for my edification.
  // Here we list all of the extensions supported on this machine.
  //

  // Get the number of supported extensions.
  num_supported_extensions = 0;
  vkEnumerateInstanceExtensionProperties(
    NULL,
    &num_supported_extensions,
    NULL
  );

  // Now actually fill in the enumerated extensions.
  supported_extensions.resize(num_supported_extensions);
  vkEnumerateInstanceExtensionProperties(
    NULL,
    &num_supported_extensions,
    supported_extensions.data()
  );

  cout << "Available extensions:" << endl;
  for (const auto& extension : supported_extensions) {
    cout << '\t' << extension.extensionName << endl;
  }
}

vector<const char*> get_required_extensions() {
  uint32_t glfw_extension_count;
  const char** glfw_extensions;
  vector<const char*> result;

  //
  // First add all the extensions GLFW needs.
  //

  glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

  // Copies the glfw_extensions array into result.
  std::copy(
    glfw_extensions,
    glfw_extensions + glfw_extension_count,
    std::back_inserter(result)
  );

  //
  // Now add the validation layer extension for message callbacks if
  // validation layers are supported.
  //

  if (enable_validation_layers) {
    result.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  return result;
}

void populate_debug_messenger_create_info(
  VkDebugUtilsMessengerCreateInfoEXT& create_info
) {
  create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  create_info.messageSeverity =
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  create_info.messageType =
    VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  create_info.pfnUserCallback = debug_callback;
  create_info.pUserData = NULL;
}

void setup_debug_messenger(application* app) {
  VkDebugUtilsMessengerCreateInfoEXT create_info;
  VkResult result;

  // Nothing to do since this only applies to using
  // validation layers.
  if (!enable_validation_layers) {
    return;
  }

  //
  // Set up the create info object.
  //

  populate_debug_messenger_create_info(create_info);

  //
  // Now attempt to create the debug messenger.
  //

  result = create_debug_utils_messenger(
    app->vulkan_instance,
    &create_info,
    NULL,
    &(app->debug_messenger)
  );

  if (result != VK_SUCCESS) {
    throw runtime_error("failed to set up debug messenger!");
  }
}

void pick_physical_device(application* app) {
  uint32_t device_count;
  vector<VkPhysicalDevice> devices;

  //
  // First, get the number of supported devices that we can
  // choose from.
  //

  device_count = 0;
  vkEnumeratePhysicalDevices(app->vulkan_instance, &device_count, NULL);

  if (device_count == 0) {
    throw runtime_error("failed to find GPUs with Vulkan support!");
  }

  //
  // Now get the actual devices.
  //

  devices.resize(device_count);
  vkEnumeratePhysicalDevices(
    app->vulkan_instance,
    &device_count,
    devices.data()
  );

  //
  // Next, scan each device to find the first one that suits our needs.
  //

  for (const VkPhysicalDevice& device : devices) {
    if (is_device_suitable(device)) {
      app->physical_device = device;
      break;
    }
  }

  // If none of the GPUs worked, throw an error.
  if (app->physical_device == VK_NULL_HANDLE) {
    throw runtime_error("failed to find a suitable GPU!");
  }
}

bool is_device_suitable(VkPhysicalDevice device) {
  queue_family_indices indices;

  indices = find_queue_families(device);

  return is_complete(indices);
}

queue_family_indices find_queue_families(VkPhysicalDevice device) {
  queue_family_indices indices;
  uint32_t num_queue_families;
  vector<VkQueueFamilyProperties> queue_families;
  uint32_t i;

  //
  // First query the queue families supported by the device.
  //

  vkGetPhysicalDeviceQueueFamilyProperties(device, &num_queue_families, NULL);

  queue_families.resize(num_queue_families);
  vkGetPhysicalDeviceQueueFamilyProperties(
    device,
    &num_queue_families,
    queue_families.data()
  );

  //
  // Next, scan the list of queue families to find one that has
  // the VK_QUEUE_GRAPHICS_BIT set. Basically, this says the given
  // device can do graphics operations.
  //

  i = 0;
  for (const VkQueueFamilyProperties next_queue_family : queue_families) {
    if (next_queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphics_family = i;
    }

    if (is_complete(indices)) {
      break;
    }

    i++;
  }

  return indices;
}

void application_main_loop(application* app) {
  while (!glfwWindowShouldClose(app->window)) {
    glfwPollEvents();
  }
}

void application_cleanup(application* app) {
  if (enable_validation_layers) {
    destroy_debug_utils_messenger(
      app->vulkan_instance,
      app->debug_messenger,
      NULL   
    );
  }

  vkDestroyInstance(app->vulkan_instance, NULL);

  glfwDestroyWindow(app->window);
  glfwTerminate();
}

//
// QUEUE FAMILY INDICES IMPL.
//

bool is_complete(const queue_family_indices& queue_fam) {
  return queue_fam.graphics_family.has_value();
}

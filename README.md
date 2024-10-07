# Introduction to Vulkan

Notes taken from [this](https://vulkan-tutorial.com/Introduction) tutorial.

## Overview
* Vulkan is a low-overhead, cross-platform API standard for 3D graphics
  and computing.

* Goal of Vulkan: build API from ground up that specifies what we want
  to do with a modern graphics card

* More verbose than say OpenGL.
* Allows thing like parallel commands from different threads
* Reduces inconsistencies in shader compilation by switching to a standardized byte code
  format with a single compiler
* Graphics and compute functionality into a single API

## How to draw a triangle

### Step 1 - Instance and physical device selection
* Create a `VkInstance`
  * Describing application and any API extensions you'll need
* Then select one or more `VkPhysicalDevice`

### Step 2 - Logical device and queue families
* Create a `VkDevice` (logical device)
  * Describe which `VkPhysicalDeviceFeature`s you will be using
    * ie, 64 bit floats, multi-viewports, etc.
* Specify which queue families you are using
  * Commands are submitted to `VkQueue` and executed asynchronously
  * Each queue can do a certain set of operations
    * For example: one for graphics, one for compute, one for memory transfer

### Step 3 - Window surface swap chain
* Create a window as "per usual" (using something like GLFW or SDL)
* Need two additional objects:
  1. Window surface `VkSurfaceKHR`
  2. Swap chain `VkSwapchainKHR`
* `KHR` is apart of an extension
* Vulkan is platform-agnostic
  * Need standardized Window System Interface extension
    to interact with OS window manager
* Surface is cross platform abstraction over windows
  * Instantiated by providing a reference to native window handle
* Swap chain is collection of render targets
  * Image we are currently rendering to is different
    than one already on screen
  * Only want complete images shown

### Step 4 - Image views and framebuffers
* To render image from swap chain, must wrap it in
  `VkImageView` and `VkFrameBuffer`
* Image view: reference part of image to be used
* Framebuffer: image views that are used for color, depth, stencil targets

### Step 5 - Render passes
* Describe type of images that are used during rendering,
  how they are used, and how contents should be treated

### Step 6 - Graphics Pipeline
* `VkPipeline` object
* Describes configurable state of graphics card
  * Viewport size
  * Depth buffer operation
  * Programmable state
  * `VkShaderModule` objects
* Shader module created from shader byte code
* Pipeline also needs to know which render targets will
  be used, which is specified by referencing render pass

* All pipeline configuration must be done in advance.
  * So you'll need multiple pipelines for all the different
    shaders
  * Allows for a lot of great optimizations

### Step 7 - Command pools and command buffers
* Operations must be recorded into a `VkCommandBuffer`
* Command buffers are allocated from a `VkCommandPool`
  that is associated with a specific queue family.
* For drawing a triangle, need to record a command
  buffer with the following operations:
  1. Begin the render pass
  2. Bind the graphics pipeline
  3. Draw 3 vertices
  4. End the render pass

### Step 8 - Main loop
* First, acquire image from swap chain with
  `vkAcquireNextImageKHR`.
* Then select appropriate command buffer for image and
  execute it with `vkQueueSubmit`.
* Finally, return image to the swap chain for
  presentation to the screen with `vkQueuePresentKHR`,
* Operations submitted to queues executed asynchronously.
  * So need semaphors and other objects for correct
    order of execution.

## Validation Layers
* The API, being low-level, usually gives no errors
  as to why it failed. But it has something called
  validation layers that

* Vulkan designed around minimal API, so almost no errors
* But you can add error checking through these validation layers
* Validation layers:
  * Optional components that hook into Vulkan function calls
    * Apply additional operations
* So they seem to be basically wrapper functions. Little disappointed
  rn
  * Okay I will walk this back. They are, but Vulkan seems to have some
    built-in mechanisms for enabling/disabling/using them.
* Just like extensions, validation layers must be enabled by specifying their names.
* Standard validation bundled into a layer called `VK_LAYER_KHRONOS_validation`

## Physical Devices
* Not a lot to say here that isn't understandable in code
* First, Vulkan can use many devices at once
* Also, computers these days have so many devices on them
  * Not a bad idea to write code to rank the best one
* So the physical device in Vulkan is a view of the physical card
  that Vulkan has

### Queue Families
* All operations (drawing, upload textures, etc) requires commands
  to be submitted via a queue.
* There are different types of queues
  * Graphics queue for graphics pipeline commands
  * Compute queues
  * Sparse binding queues
  * And so on
* A queue family describes a set of queues with identical properties
  * I think these queue families are typically described by physical device
    * Yes

## Logical Devices
* A physical device must be interfaced with a logical device
* A logical device is a view of the physical device.
  * To do rendering, we needn't know *everything* about the actual
    device, so we can model it with a logical device

## Window Surface
* Vulkan cannot interface with windowing system on its own
* We thus need Window System Information extensions
  * We learn `VK_KHR_surface`, which exposes `VkSurfaceKHR`
* `VK_KHR_surface` is instance-level extension
* Must be created after instance is created because it 
  can influence physical device selection
* Unlike OpenGL, allows for off-screen rendering (OpenGL hack: invisible window)
* Creation of `VkSurfaceKHR` depends on window system details

### Querying for Presentation Support
* Vulkan may support window system integration, but devices may not.
* Thus, must check if a device can present images to the surface
  we've created
* Presentation is a queue-specific feature, so must find the right
  queue family that supports presenting surface

## Swap Chain
* A queue of images that are waiting to be displayed
* Program grabs image from queue, draws it, and then puts it back
* Not every machine supports presenting images (ie, server, etc).
* Presence of presentation layer implies swap chain support
* Note that just because swap chain is supported, it may not
  be compatible with window surface
* Things we need:
  1. Basic surface capabilities (min/max # of images on swapchain, min/max height
     for said images)
  2. Surface formats (pixel format, color space
  3. Available presentation modes

### Presentation Modes
* Four modes in Vulkan:
  1. `VK_PRESENT_MODE_IMMEDIATE_KHR`
    * Images submitted by program go right to screen
    * May result in tearing
  2. `VK_PRESENT_MODE_FIFO_KHR`
    * Swap chain as queue
      * Takes image from front when display is refreshed
      * Insert images in back
    * If queue full, program halts until available
    * Similiar to vertical sync in games
    * When display refreshed, we call it "vertical blank"
  3. `VK_PRESENT_MODE_FIFO_RELAXED_KHR`
    * Basically same as previous
    * But if application is late and queue was empty at last blank,
      instead of waiting for next blank, render immediately
    * Can lead to visible tearing too
  4. `VK_PRESENT_MODE_MAILBOX_KHR`
    * Another variation of the second mode
    * Instead of blocking application when queue is full,
      images that are already queued are replaced with newer ones.
    * Can be used to render frames as fast as possible
      while avoiding tearing
    * Fewer latency issues than standard v-sync
    * Sometimes called "triple buffering"
      * Does not neccessarily unlock framerate if third buffer present
* Only `VK_PRESENT_MODE_FIFO_KHR guaranteed to be available

### Swap Extent
* Resolution of swap chain images
* Almost always equal to the resolution of the window we're drawing
  in pixels
* Range of resolutions defined in the `VkSurfaceCapabilitiesKHR` structure
* See Swap Extent notes in tutorial - good stuff there

### Graphics Pipeline
* The collection of steps to take textures + meshes and produce
  pixels on screen
* Include (broadly):

1. Input Assembler
  * Collects raw vertex data from the buffers you specify
2. Vertex Shader
  * Runs for every vertex
  * Applies transformations to turn vertex positions
    from model space to screen space
  * Passes per-vertex data down the pipeline
3. Tessellation
  * Subdivide geometry to make mesh quality better
  * Something like taking a brick wall and making
    it less flat when you're nearby
4. Geometry Shader
  * Run on all primitives (triangles, lines, points)
    * Can discard or output more
  * Not very performant, so not widely used
5. Rasterization
  * Discretizes primitives into fragments.
  * Fragments are pixel elements that go into framebuffer
  * Where we clip to remove things outside screen bounds,
    and disregard pieces via depth-testing
6. Fragment Shader
  * Ran on every fragment that survives
  * Determines which framebuffers the fragments are
    written to, and which colors + depth values
7. Color blending
  * Blend/mix fragments that map to same pixel
8. Framebuffer

* Fixed-function pipeline: Just steps 1, 5, 7
  * Can tweak parameters, but underlying work is hard-coded
* Rest of steps are `programmable`
  * Can upload our own code

## Extrata
* Tiled Rendering: Technique for rendering where you take rendering space
  and split it into a series of tiles
* Check `Config` directory of Vulkan SDK to see how to configure validation layers.
  Look for `vk\_layers\_settings.txt`
* TODO: Solve current warning
* TODO: Rate physical devices

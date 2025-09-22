---
title: vulkan introduction
---

Computer graphics has always lived at the intersection of science and imagination. From the pixelated simplicity of Pong in 1970s to the photorealism of modern video games and cinematic visual effects, our tools for drawing to the screen have grown astonishingly powerful. But behind every realistic explosion, glowing sunset, or procedurally generated planet lies a mountain of computation — and a carefully orchestrated pipeline between software and hardware.

To understand Vulkan — a modern graphics API designed for maximum performance and control — we must first understand the journey that brought us here. The story of Vulkan is not just about a new tool, but about the evolution of the very way we think about rendering images with computers.

---

# In the Beginning: The Age of Fixed-Function Pipelines

In the early days of computer graphics, the process of drawing 3D scenes was both limited and rigid. Graphics hardware offered a fixed-function pipeline — a set of hardcoded stages that transformed 3D geometry into a 2D image. You could translate, rotate, and scale objects. You could define how they were lit using a few built-in lights. You could map textures onto surfaces. But all of this had to be done using predefined operations set by the GPU’s designers.

Developers were passengers in this system. They could steer within narrow lanes but had little freedom to rewrite the rules of the road.

This pipeline made many things easy, which was its strength — but it was also its weakness. As games and simulations grew more complex and ambitious, developers needed to create their own lighting models, their own shading logic, their own effects. And the fixed pipeline simply couldn’t bend that far.

The industry was ready for a transformation.

---

# A Turning Point: Programmable Shaders

The breakthrough came in the early 2000s when GPU manufacturers began to roll out programmable hardware. No longer were developers restricted to built-in lighting and shading rules. They could now write their own small programs — called shaders — to define exactly how each vertex and pixel should behave.

This revolution began in earnest with ATI’s Radeon 9700 (R300) in 2002 — the first fully programmable GPU. With it came support for vertex and pixel shaders, and the dawn of the programmable graphics pipeline.

Suddenly, artists and engineers could implement per-pixel lighting, normal mapping, custom fog effects, and more. Realism and creativity flourished. And as GPUs became more powerful, so did the ambition of those writing code for them. A new generation of rendering engines was born, more cinematic and immersive than ever before.

But there was still one problem: the software interface.

---

# The Reign of OpenGL

To access GPU functionality, developers relied on graphics APIs — software layers that let applications communicate with the GPU. One of the most influential APIs was OpenGL, originally developed by Silicon Graphics in the early 1990s. OpenGL offered a cross-platform interface to 3D rendering and eventually evolved to support programmable shaders and more advanced GPU features.

For many years, OpenGL reigned supreme in academic research, professional graphics software, and game engines alike. It was powerful, widely supported, and reasonably easy to use. But as GPUs became more complex and computers moved to multi-core architectures, OpenGL’s original design began to show cracks.

Much of OpenGL’s behavior was implicitly managed by the driver, the software layer provided by GPU vendors. This abstraction was useful for beginners but became a liability for professionals. Performance could vary dramatically across vendors and platforms. Worse, OpenGL’s global state machine model made it difficult — and often impossible — to use multiple CPU threads efficiently. Developers were left guessing how the driver would behave and were often forced to write code defensively, targeting the lowest common denominator of performance and compatibility.

In a world moving toward low-latency VR, ray tracing, and real-time physics, that just wasn’t good enough anymore.

---

# Enter the Modern Graphics API

Around 2014, a new philosophy began to take root in the graphics community. The idea was simple: what if APIs were explicit, giving developers full control over memory, synchronization, and resource usage? What if we stripped away decades of abstraction and gave professionals the tools to fully exploit modern hardware?

This was the vision behind a new class of low-level graphics APIs — designed not to shield developers from complexity, but to expose it in a structured, predictable way.

These APIs included:
- Direct3D 12, Microsoft’s low-level API for Windows.
- Metal, Apple’s graphics API for iOS and macOS.
- And of course, Vulkan, a bold new cross-platform standard from the Khronos Group.

These APIs spoke the language of modern graphics: multi-threading, GPU parallelism, explicit resource control, and deterministic performance.

They also shifted responsibility from the driver to the developer. That meant more work — but also more power.

---

# What Is Vulkan?

Vulkan is the result of this evolution — a modern, low-overhead, cross-platform graphics and compute API that gives developers unprecedented control over the GPU. Released in 2016 by the Khronos Group (the same consortium that manages OpenGL), Vulkan was designed for performance-critical applications on a wide variety of platforms: Windows, Linux, Android, and even macOS (via a translation layer called MoltenVK).

What makes Vulkan different?
- Explicit design: You manage GPU memory, synchronization, and resource lifetimes yourself. No more driver guesswork.
- Multi-threading support: Vulkan’s architecture makes it possible to generate and submit commands from multiple CPU threads in parallel — ideal for modern processors.
- Cross-platform consistency: Vulkan offers the same core behavior across platforms, reducing fragmentation and vendor-specific hacks.
- Unified compute and graphics: You can perform general-purpose GPU tasks (like physics or AI) and rendering with the same API.

But Vulkan is not for the faint of heart. It is verbose. It is strict. It demands precision. And it shifts the burden of correctness to you, the developer.

Yet for all that, it gives something back that few APIs can: predictability. If your Vulkan application runs slowly, it’s likely your fault — not the driver’s. And that is empowering.

**Next** [Vulkan SDK](../01/Vulkan_SDK.md)
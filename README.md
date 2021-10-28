# opengl-geometry-dump
This repository contains the example implementation for extracting the primitives of graphics pipeline using transform feedback. Since that Vulkan seems to support transform feedback, you can try to transcribe OpenGL part with Vulkan. Note that, currently, this code not librarified enough. 

# Motivation
As far as I know, most transform feedback examples[1, 2] only explain or implement only point primitive extraction. So they cannot recover connectivity of the geometry. Of course that softwares such as [RenderDoc](https://renderdoc.org/), [glintercept](https://github.com/dtrebilco/glintercept) can do similar thing. But we cannot extract geometry programatically with RenderDoc, and glintercept cannot be running other than Windows environment.

# Getting Started
## Dependencies
Easiest way to install the dependencies is vcpkg.

- GLUT, GLEW, GLM, OpenMesh

# To Do
- [ ] Support extracting additionaly generated geometry.
- [ ] Connectivity extraction and original geometry recovering.

# References
[1] [Particle System using Transform Feedback](https://ogldev.org/www/tutorial28/tutorial28.html)  
[2] [Transform feedback - Feedbacktransform and geometry shaders](https://open.gl/feedback)
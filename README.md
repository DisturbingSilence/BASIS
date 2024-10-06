# BASIS
Adapting [Fwog](https://github.com/JuanDiegoMontoya/Fwog) for personal needs in the process of learning opengl

## TODO
 - [ ] SPIRV support
    * lacks GL_ARB_bindless_texture support as of 06.10.2024
 - [ ] Framebuffer abstraction
 - [ ] Audio
 - [ ] Viewports and scissors
 - [ ] Async asset loading
 - [ ] Meshlets
 - [ ] Automatic glad pulling(?)
 - [x] Compute shaders
 - [ ] KTX2  
 - [ ] JSON configuration files for quick scene setup(simdjson)

## Building
```
git clone https://github.com/Bannerjee/BASIS.git
cd BASIS
cmake -B build
```
## Dependencies
* glad  
* GLFW  
* stbi  
* imgui
* fastgltf
* simdjson
* glm

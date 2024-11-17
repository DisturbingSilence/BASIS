# BASIS
Library for quick prototyping based on [Fwog](https://github.com/JuanDiegoMontoya/Fwog)

## TODO
 - [ ] SPIRV support
    * lacks GL_ARB_bindless_texture support as of 06.10.2024
 - [x] Framebuffer abstraction
 - [ ] Audio
 - [ ] Viewports and scissors
 - [ ] Async asset loading
 - [ ] Meshlets
 - [ ] Automatic glad pulling(?)
 - [x] Compute shaders
 - [x] KTX2
    * Needs to be reworked in certain places
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

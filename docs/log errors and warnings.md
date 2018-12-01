# Log calls

When errors occur, it might be hard to locate, but with ocore, fatal errors have error codes. All calls to Log are documented in this document, including fatal errors with error codes and classes and error/warning/print text.

## ogc

### Vulkan

#### Fatal errors

| File                                                  | Message                                                      | Class           | Id   | Description                                                  |
| ---- | ---- | ---- | ---- | ---- |
| graphics<br />gl<br />vulkan<br />vkcommandlist.cpp     | CommandList::bind requires VBOs as first argument            | VkCommandList   | 0x0  | CommandList::bind requires the GBuffer*[] given to be of type GBufferType::VBO |
|                                                         | CommandList::bind requires a valid IBO as second argument    |                 | 0x1  | CommandList::bind requires the GBuffer* given to be of type GBufferType::IBO |
| | Couldn't allocate command list | | 0x2 | The command pool allocated doesn't have enough space for the command buffers |
| graphics<br />gl<br />vulkan<br />vkgbuffer.cpp         | Couldn't find a valid memory type for a VkGBuffer: {name}    | VkGBuffer       | 0x0  | The buffer created can't find space on the GPU to allocate the buffer into |
|                                                         | Couldn't find a valid memory type for a staging buffer for VkGBuffer: {name} |                 | 0x1  | The staging buffer required for updating the buffer's data couldn't be allocated (see 0x0) |
| | Failed to create buffer | | 0x2 | vkCreateBuffer returned an error, more details are printed before this error message |
| | Couldn't allocate memory | | 0x3 | vkAllocateMemory returned an error, more details are printed before this error message |
| | Couldn't bind memory to buffer {name} #{version} | | 0x4 | vkBindBufferMemory returned an error, more details are printed before this error message |
| | Failed to create staging buffer | | 0x5 | see 0x2 |
| | Couldn't allocate memory | | 0x6 | see 0x3 |
| | Couldn't bind memory to buffer | | 0x7 | see 0x4 |
| | Failed to map staging buffer | | 0x8 | vkMapMemory returned an error, more details are printed before this error message |
| | Couldn't map memory | | 0x9 | see 0x8 |
| graphics<br />gl<br />vulkan<br />vkgraphics.cpp        | Couldn't intialize family queue                              | VkGraphics      | 0x0  | The GPU doesn't support both graphics and compute operations on 1 queue |
|                                                         | Surface wasn't supported                                     |                 | 0x1  | The current surface doesn't support Vulkan                   |
|                                                         | Couldn't get surface format                                  |                 | 0x2  | The surface doesn't have a format that can be recognized     |
|                                                         | Size is undefined; this is not supported!                    |                 | 0x3  | Some devices allow the size of the surface to be zero, this doesn't have a valid implementation |
|                                                         | Render size didn't match {size}                              |                 | 0x4  | The surface had a different resolution than the window       |
|                                                         | Immediate presentMode is required for single buffering       |                 | 0x5  | Single buffering is only allowed with immediate present mode |
|                                                         | Couldn't initialize swapchain image view                     |                 | 0x6  | The image view for a swapchain image couldn't be generated   |
|                                                         | Couldn't initialize back buffer (render target)              |                 | 0x7  | The render target representing the back buffer couldn't be generated |
|                                                         | Couldn't create command list                                 |                 | 0x8  | CommandList initialization failed                            |
|  | Couldn't get Vulkan extension |  | 0x9 | The Vulkan function requested does not exist |
|  | Couldn't obtain Vulkan instance |  | 0xA | vkCreateInstance returned an error, the message before shows the internal error |
|  | Couldn't create debug report callback |  | 0xB | Couldn't create debug report callback, the message before shows the internal error |
|  | Couldn't obtain device |  | 0xC | vkCreateDevice failed, internal error has been printed |
|  | Couldn't create command pool |  | 0xD | Couldn't create command pool, internal error has been printed |
|  | Couldn't obtain surface |  | 0xE | Couldn't create surface, internal error has been printed |
|  | Surface wasn't supported |  | 0xF | Surface wasn't supported, internal error has been printed |
|  | Couldn't create swapchain |  | 0x10 | Couldn't create swapchain, internal error has been printed |
|  | Couldn't create the present fence |  | 0x11 | The fences required for keeping track of whether an image is presented couldn't be created, internal error has been printed |
|  | Couldn't create submit semaphore |  | 0x12 | The semaphores required for keeping track of the swapchain images couldn't be created, internal error has been printed |
|  | Couldn't create swapchain semaphore |  | 0x13 | The semaphores required for keeping track of the command queue couldn't be created, internal error has been printed |
|  | Couldn't acquire next image |  | 0x14 | Normally occurs when the surface isn't available anymore, internal error has been printed |
|  | Couldn't wait for fences |  | 0x15 | Only happens when the fences were signaled the wrong way or destroyed, internal error has been printed |
|  | Couldn't reset fences |  | 0x16 | see 0x15 |
|  | Couldn't submit queue |  | 0x17 | Couldn't submit queue, contains an invalid command buffer, internal error has been printed |
|  | Couldn't present image |  | 0x18 | Couldn't present image; surface destroyed or see 0x17 |
|  |  |  | 0x19 | see 0x18 |
| graphics<br />gl<br />vulkan<br />vkpipeline.cpp        | Pipeline requires a shader                                   | VkPipeline      | 0x0  | All pipelines require a shader                               |
|                                                         | Graphics pipeline requires a render target, pipeline state and mesh buffer |                 | 0x1  | A graphics pipeline needs a mesh buffer, pipeline state and render target to be set |
|                                                         | Couldn't create pipeline; Shader vertex input type didn't match up with vertex input type; {shaderName}'s {varName} and {meshBufferName}'s {meshVarName} |                 | 0x2  | The inputs of the vertex shader didn't match up with the MeshBuffer's layout |
|                                                         | Couldn't create pipeline; no match found in shader input from vertex input; {varName} |                 | 0x3  | A vertex input is missing, resulting into the MeshBuffer being incompatible |
|                                                         | Couldn't create pipeline; vertex input {name} is already set. Don't use duplicate vertex inputs |                 | 0x4  | The MeshBuffer contains a layout with a name that already exists; duplicated names aren't allowed with shaders |
|                                                         | Invalid pipeline; Shader referenced a shader output to an unknown output |                 | 0x5  | The RenderTarget doesn't have the correct outputs for the shader |
|                                                         | Invalid pipeline; Shader referenced an incompatible output format |                 | 0x6  | The RenderTarget outputs are incompatible with the shader outputs |
| | Couldn't create graphics pipeline | | 0x7 | The pipeline settings were invalid, internal errors have been logged |
| | Couldn't create compute pipeline | | 0x8 | see 0x7 |
| graphics<br />gl<br />vulkan<br />vkshader.cpp          | Shader mentions an invalid buffer                            | VkShader        | 0x0  | When creating the Vulkan descriptor set, the shader found that there was no ShaderBuffer where a buffer was expected |
|                                                         | A ShaderBuffer has been placed on a register not meant for buffers |                 | 0x1  | When creating the Vulkan descriptor set, the shader found that there was a ShaderBuffer where it wasn't expected |
|                                                         | A Sampler has been placed on a register not meant for samplers |                 | 0x2  | When creating the Vulkan descriptor set, the shader found that there was a Sampler where it wasn't expected |
|                                                         | A Texture has been placed on a register not meant for textures or it has to use a TextureList |                 | 0x3  | When creating the Vulkan descriptor set, the shader found that there was a Texture where it wasn't expected |
|                                                         | A TextureList has been placed on a register not meant for textures or it has to use a Texture |                 | 0x4  | When creating the Vulkan descriptor set, the shader found that there was a TextureList where it wasn't expected. If the texture array has size 1 it is represented with a Texture |
|                                                         | A TextureList had invalid size!                              |                 | 0x5  | The shader's texture array didn't match the TextureList size |
|                                                         | A VersionedTexture has been placed on a register not meant for textures |                 | 0x6  | When creating the Vulkan descriptor set, the shader found that there was a VersionedTexture where it wasn't expected |
|                                                         | Shader mentions an invalid resource                          |                 | 0x7  | The resource mentioned in the register is invalid            |
| | Couldn't create descriptor set layout | | 0x8 | Descriptor set layout couldn't be created, internal errors were logged |
| | Couldn't create shader pipeline layout | | 0x9 | Pipeline layout couldn't be created, internal errors were logged |
| | Couldn't create descriptor pool | | 0xA | Descriptor pool couldn't be created, internal errors were logged |
| | Couldn't create descriptor sets | | 0xB | Descriptor set couldn't be created, internal errors were logged |
| graphics<br />gl<br />vulkan<br />vkshaderstage.cpp | Shader stage creation failed | VkShaderStage | 0x0 | Shader stage couldn't be created, internal errors were logged |
| graphics<br />gl<br />vulkan<br />vktexture.cpp         | Couldn't get depth texture; no optimal format available      | VkTexture       | 0x0  | The GPU doesn't support depth buffers                        |
|                                                         | Couldn't find a valid memory type for a VkTexture: {name}    |                 | 0x1  | The GPU couldn't find how to allocate the texture or didn't have enough space |
|                                                         | The buffer was of incorrect size                             |                 | 0x2  | The initialization data didn't match the texture's resolution and or format |
|                                                         | Couldn't find a valid memory type for a texture staging buffer: {name} |                 | 0x3  | The GPU couldn't find how to allocate the texture staging buffer or didn't have enough space |
| | Couldn't create image | | 0x4 | Couldn't create image object, internal errors have been logged |
| | Couldn't allocate memory | | 0x5 | No space could be allocated for the texture; GPU was out of memory or didn't have correct memory space |
| | Couldn't bind memory to texture {name} | | 0x6 | Memory allocated was a different type than what the texture expected |
| | Couldn't create image view | | 0x7 | The image was invalid or the image view couldn't be created |
| | Couldn't send texture data to GPU | | 0x8 | Couldn't create staging buffer |
| | Couldn't allocate memory | | 0x9 | No space could be allocated for the staging buffer; GPU was out of memory or didn't have correct memory space |
| | Couldn't bind memory to staging buffer {name} | | 0xA | see 0x6 |
| | Couldn't map texture staging buffer | | 0xB | The memory for the staging buffer couldn't be unmapped |
| graphics<br />gl<br />vulkan<br />vkrendertarget.cpp | Couldn't create render pass for render target | VkRenderTarget | 0x0 | Render pass settings were invalid, internal errors have been logged |
| | Couldn't create framebuffers for render target | | 0x1 | Framebuffer settings were invalid, internal errors have been logged |
| graphics<br />gl<br />vulkan<br />vksampler.cpp | Couldn't create sampler object | VkSampler | 0x0 | Sampler settings were invalid |

#### Errors

| File                           | Message                         | Description                                                  |
| ------------------------------ | ------------------------------- | ------------------------------------------------------------ |
| graphics<br />gl<br />vulkan.h | VkResult: Out of host memory    | The object couldn't be allocated because the CPU didn't have any memory left |
|                                | VkResult: Out of device memory  | The object couldn't be allocated because the GPU didn't have any memory left |
|                                | VkResult: Initialization failed | The creation info passed was possibly invalid, initialization failed. |
|                                | VkResult: Device lost           | Access to the GPU was lost                                   |
|                                | VkResult: Memory map failed     | Couldn't map memory; not enough driver memory?               |
|                                | VkResult: Layer not present     |                                                              |
|                                | VkResult: Extension not present |                                                              |
|                                | VkResult: Feature not present   |                                                              |
|                                | VkResult: Incompatible driver   | Driver doesn't support Vulkan or the current version         |
|                                | VkResult: Too many objects      | There were too many objects on the GPU, so none could be created |
|                                | VkResult: Format not supported  | Requested (texture) format can't be created                  |
|                                | VkResult: Fragmented pool       | Nothing can be allocated into the pool; it was fragmented    |
|                                |                                 |                                                              |
|                                |                                 |                                                              |

### Fatal errors

| File                                                    | Message                                                      | Class           | Id   | Description                                                  |
| ------------------------------------------------------- | ------------------------------------------------------------ | --------------- | ---- | ------------------------------------------------------------ |
| graphics<br />format<br />fbx                           | Couldn't cast to FbxProperty                                 | FbxProperty     | 0x0  | An FbxProperty doesn't match the requested type              |
| graphics<br />graphics.h                                | Couldn't init GraphicsObject                                 | Graphics        | 0x0  | The GraphicsObject init failed                               |
| graphics<br />graphics.cpp                              | Couldn't read shader                                         |                 | 0x1  | The shader data in the oiSH was unreadable or the file doesn't exist |
|                                                         | Couldn't load texture from disk                              |                 | 0x2  | The texture path given had invalid data or the file doesn't exist |
|                                                         | Couldn't load texture; Texture load format is invalid        |                 | 0x3  | The texture load format given was undefined                  |
| graphics<br />format<br />oish.h                        | Invalid constructor used; SPV isn't text but binary          | ShaderSource    | 0x0  | Constructor with String is not allowed for SPIRV; only the CopyBuffer constructor |
| graphics<br />objects<br />shader<br />shaderbuffer.h   | Couldn't cast ShaderBufferVar with format {format} ({name})  | ShaderBufferVar | 0x0  | A shader buffer variable doesn't match the format given      |
| graphics<br />objects<br />shader<br />shaderbuffer.cpp | Couldn't find the path "{path}"                              |                 | 0x1  | The path provided to ShaderBuffer::get or ShaderBuffer::set was invalid |
| graphics<br />objects<br />view<br />view.h             | Camera, CameraFrustum and View aren't in the same ViewBuffer | ViewInfo        | 0x0  | The view requires both the camera and camera frustum to be in the same ViewBuffer |
| graphics<br />helper<br />bakemanager.cpp               | Invalid BakeManager file                                     | BakeManager     | 0x0  | The file doesn't have a header                               |
|                                                         | Invalid BakeManager file header                              |                 | 0x1  | The file header is invalid                                   |
|                                                         | Invalid BakeManager file size                                |                 | 0x2  | The expected file size didn't match the actual file size     |
|                                                         | Invalid BakeManager's baked file size                        |                 | 0x3  | The file didn't contain enough baked files (invalid header)  |
|                                                         | Invalid BakeManager strings                                  |                 | 0x4  | The oiSL file included in the file didn't include the requested strings |
| graphics<br />objects<br />render<br />drawlist.cpp     | Grouping the meshes by instance is required!                 | DrawList        | 0x0  | `DrawList::draw` can only be called once per mesh, you have to group them by instance manually |
| graphics<br />objects<br />model<br />material.cpp      | Texture should be in same TextureList used in MaterialList   | Material        | 0x0  | It is only possible to use textures from the same texture list in a material list |
| graphics<br />objects<br />model<br />meshbuffer.cpp    | Please only allocate 0 indices when there is no index buffer, or use indices when there is an index buffer | MeshBuffer      | 0x0  | The mesh allocated into a MeshBuffer uses a different draw method than the MeshBuffer. |
|                                                         | Couldn't allocate indices                                    |                 | 0x1  | The indices for the mesh couldn't be allocated, use a bigger index count |
|                                                         | Couldn't allocate vertices                                   |                 | 0x2  | The vertices for the mesh couldn't be allocated, use a bigger vertex count |
| graphics<br />format<br />oish.cpp                      | ShaderRegister {name} is invalid                             | oiSH            | 0x0  | ShaderRegister type or access is invalid                     |
|                                                         | Invalid shader output                                        |                 | 0x1  | The shader output had an invalid type                        |
|                                                         | oiSH::convert couldn't be executed; not enough stages        |                 | 0x2  | The shader info passed doesn't have any shader stages        |
|                                                         | oiSH::convert couldn't be executed; it only has one stage. Which is only allowed for compute |                 | 0x3  | oiSH expected a compute shader, because single stage shaders aren't possible except for compute shaders |
|                                                         | oiSH::convert couldn't be executed; no bytecode              |                 | 0x4  | The compute stage doesn't have any bytecode attached         |
|                                                         | oiSH::convert couldn't be executed; Graphics shaders can't contain Compute module |                 | 0x5  | Compute shaders are only allowed in single stage shaders     |
|                                                         | oiSH::convert couldn't be executed; no bytecode              |                 | 0x6  | The shader stage doesn't have any bytecode attached          |
|                                                         | Invalid register type                                        |                 | 0x7  | The shader register type could not be detected               |
|                                                         | ShaderRegister of type Buffer (SSBO or UBO) doesn't reference a buffer |                 | 0x8  | A shader register of type SSBO or UBO has to reference to an oiSB file |
| graphics<br />objects<br />shader<br />shader.cpp       | Shader::set({path}) failed; the path couldn't be found       | Shader          | 0x0  |                                                              |
|                                                         | Shader::set({path}) failed; invalid type (type is ShaderBuffer, but type provided isn't) |                 | 0x1  | Shader::set was expecting a ShaderBuffer, but a different type was sent |
|                                                         | Shader::set({path}) failed; invalid type (type is Texture or VersionedTexture, but type provided isn't) |                 | 0x2  | Shader::set was expecting a Texture or VersionedTexture, but a different type was sent |
|                                                         | Shader::set({path}) failed; invalid type (type is Sampler, but type provided isn't) |                 | 0x3  | Shader::set was expecting a Sampler but a different type was sent |
|                                                         | Shader::set({path}) failed; TextureList size incompatible with shader |                 | 0x4  | Shader::set was expecting a TextureList but a different type, or one with a different size was passed |
| graphics<br />objects<br />shader<br />shaderbuffer.cpp | Can't instantiate the buffer when it's already set           | ShaderBuffer    | 0x0  | ShaderBuffer::instantiate is only allowed when the buffer is not set yet |
|                                                         | Can't set the buffer when it already contains data           |                 | 0x1  | ShaderBuffer::setBuffer is only allowed when the buffer is not set yet |
|                                                         | GBuffer size should match ShaderBuffer size                  |                 | 0x2  | The buffer provided to ShaderBuffer::setBuffer didn't match the expected size |
|                                                         | ShaderBuffer::copy requires a buffer to be set               |                 | 0x3  | instantiate or setBuffer should be called if the buffer isn't set yet |
| graphics<br />objects<br />texture<br />texture.cpp     | Texture::set was out of bounds                               | Texture         | 0x0  | Setting a pixel requires a position between 0,0 and info.res - 1 |
|                                                         | Texture::set invalid format                                  |                 | 0x1  | The pixel size should match info.format of the texture       |
|                                                         | Texture::set can only be applied to loaded textures          |                 | 0x2  | Setting data into a render target, depth buffer or output texture is not supported |
|                                                         | Texture::setPixels was out of bounds                         |                 | 0x3  | The pixels given are out of bounds                           |
|                                                         | Texture::setPixels invalid format                            |                 | 0x4  | The data size should match info.format of the texture        |
|                                                         | Texture::setPixels can only be applied to loaded textures    |                 | 0x5  | Setting data into a render target, depth buffer or output texture is not supported |
|                                                         | Texture::flush out of bounds                                 |                 | 0x6  | The pixels given are out of bounds                           |


## owc

### Android

#### Fatal errors

| `File`                                         | Message                               | Class          | Id   | Description                                                 |
| ---------------------------------------------- | ------------------------------------- | -------------- | ---- | ----------------------------------------------------------- |
| platforms<br />android<br />awindowmanager.cpp | Only one window supported for Android | AWindowManager | 0x0  | Android doesn't support multiple windows in one application |

### Windows

#### Fatal errors

| `File`                                  | Message                      | Class   | Id   | Description                                                  |
| --------------------------------------- | ---------------------------- | ------- | ---- | ------------------------------------------------------------ |
| platforms<br />windows<br />wwindow.cpp | Couldn't init Windows class  | WWindow | 0x0  | RegisterClassEx failed, meaning the window couldn't be created for this system |
|                                         | Couldn't init Windows window |         | 0x1  | CreateWindowEx failed, meaning that the window data is invalid |

### Fatal errors

| `File`                        | Message                        | Class         | Id   | Description                                  |
| ----------------------------- | ------------------------------ | ------------- | ---- | -------------------------------------------- |
| window<br />windowmanager.cpp | Window out of bounds exception | WindowManager | 0x0  | WindowManager::operator[i] was out of bounds |

## ostlc

### Fatal errors

| `File`                        | Message                                                      | Class                               | Id   | Description                                                  |
| ----------------------------- | ------------------------------------------------------------ | ----------------------------------- | ---- | ------------------------------------------------------------ |
| utils<br />json.h             | `Invalid JSONNodeObj<T>`                                     | JSONNodeObj                         | 0x0  | JSONNodeObj::set failed, because the json value had an invalid type |
|                               | `Invalid JSONNodeObj<T>`                                     |                                     | 0x1  | JSONNodeObj::get failed, because the json value had an invalid type |
| utils<br />json.cpp           | Couldn't find JSONNode                                       | JSONNode                            | 0x0  | JSON Node didn't have the member requested                   |
| memory<br />objectallocator.h | ObjectAllocator buffer should match size * sizeof(T)         | `ObjectAllocator<T>`                | 0x0  | ObjectAllocator's constructor requires the buffer size to be size * sizeof(T) |
| utils<br />serialization.h    | JSONSerialize read not possible with a char*                 | `JSONSerialize<const char*, false>` | 0x0  | Serializing into a const char* is impossible, since the data's length is not known |
| types<br />bitset.cpp         | Couldn't write values to bitset; bitset didn't have enough space | Bitset                              | 0x0  | The `vector<u32>` is too big for this bitset. Bitset::write was impossible |
|                               | Couldn't read values from bitset; bitset didn't have enough space |                                     | 0x1  | The `vector<u32>` was too big for this bitset. Bitset::read was impossible |
| utils<br />log.cpp            | {x}                                                          | Log                                 | 0x0  | Log::print(s, LogLevel) allows to print with variable log levels. Log is not the source for these error messages. |
| types<br />string.cpp         | Couldn't decode the string; perChar can't be bigger than 32 bits | String                              | 0x0  | String::decode can't handle integers bigger than 32 bits     |
|                               | Couldn't decode the string; keyset requires at least 1 char  |                                     | 0x1  | String::decode requires at least 1 char in the keyset        |
|                               | Couldn't encode the string; perChar can't be bigger than 32 bits |                                     | 0x2  | String::encode can't handle integers bigger than 32 bits     |
|                               | Couldn't encode the string; keyset requires at least 1 char  |                                     | 0x3  | String::encode requires at least 1 char in the keyset        |
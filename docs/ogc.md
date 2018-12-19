# Osomi Graphics Core (ogc)

# Graphics class

The Graphics instance is stored in GraphicsInterface, when it is destroyed, all graphics objects are released. 

Defined in "graphics/graphics.h"

## Graphics.finish

Before you start destroying resources, you need to ask for the Graphics to finish the previous frame(s). This can be done by calling g.finish. So a destructor would look like the following:

```cpp
g.finish();
g.destroy(tex);
```

## Helper functions

There are a few helper functions for TextureFormat in Graphics. TextureFormat is the standard layout of a Texture or shader variable/vbo variable. 

```cpp
bool df = Graphics::isDepthFormat(myTextureFormat);		//Check if depth format
u32 csiz = Graphics::getChannelSize(myTextureFormat);	//Get bytes per channel
u32 cs = Graphics::getChannels(myTextureFormat);		//Get channels for format
u32 siz = Graphics::getFormatSize(myTextureFormat);		//cs * csiz

TextureFormatStorage stor = 							//What the format represents
    Graphics::getFormatStorage(myTextureFormat);		//Low lvl (int, float, etc.)

bool compat = Graphics::								//If two formats are equal
	isCompatible(myTextureFormat, TextureFormat::RGBA8)	//except channel size
    
Vec4d col = Graphics::
	convertColor(myCol, myTextureFormat);				//Sets col[cs til 4] to 0
```

# Extensions

In ogc there's a lot of extended structs. These structs are used to store API-dependent data. For setting up the graphics API, a lot of extra data is needed. This data is stored in GraphicsExt (which is a typedef for an underlying struct). An example is VkGraphics, which stores all info required for a Vulkan instance & swapchain. Be careful when using GraphicsExt and use ifdefs if you need API-dependent data.

```cpp
//Correct
#ifdef __VULKAN__
VkGraphics &gext = g.getExtension();
	//Access Vulkan information
#endif

//Correct
GraphicsExt &gext = g.getExtension();

//Incorrect (extension not always VkGraphics)
VkGraphics &gext = g.getExtension();

//Incorrect (extension not always VkGraphics)
GraphicsExt &gext = g.getExtension();
	//Access Vulkan information

```

When accessing Vulkan data through either GraphicsExt or VkGraphics, you have to wrap it into an `#ifdef __VULKAN__`. However, if you just pass GraphicsExt around and you don't use any of the fields, you don't have to.

# GraphicsObject

A GraphicsObject is created as following:

```cpp
T *t = g.create("My graphics object", TInfo(params));
```

So for a texture, it would be:

```cpp
Texture *tex = g.create("My texture", TextureInfo("res/textures/mytexture.png"));
```

Whenever you create a GraphicsObject, don't forget to call destroy at the end. This *doesn't* release the resource, until it reaches *zero references*. "g.use" increments the refCount to an object, allowing you use the same resource multiple times. Every g.use requires a g.destroy and at least 1 destroy is needed (when allocating a GO). Whenever an object is allocated, the TInfo struct becomes inaccessible and can only be modified through the GraphicsObject directly.

```cpp
//initScene
g.use(tex);			//Start using the texture

//destructor
g.destroy(tex);		//Stop using the texture
```

This way of maintaining references allows you to preserve a GBuffer that was automatically created for example (and then passing it to a different shader). 

```cpp
g.printObjects();	//Print all graphics objects (for debugging)
g.contains(tex);	//If texture is still present
g.get<Texture>();	//Get all allocated Textures (std::vector<Texture*>)
```

Defined in "graphics/objects/graphicsobject.h"

## destroyObject vs destroy

destroyObject and destroy are two different functions; one is virtual (destroyObject) and the other is not. destroy is if the final type is known (for example; Texture) and not when it is the parent (GraphicsResource or GraphicsObject). destroy will automatically set the input pointer (e.g. `Texture*` ) to nullptr when there are no more references, while destroyObject will not. Using destroy on GraphicsObject or GraphicsResource will result into an error.

## Ownership

A GraphicsObject can be owned by the API or driver, which means that there is no allocated data maintained by us. This still means you have to call destroy on them, but the driver will take care of the data. An example is a VersionedTexture, Texture and RenderTarget; the backbuffer is owned by the driver and won't be deleted by us. It isn't implemented for other types yet.

## Extension and info

All GraphicsObjects have an info struct; which can be obtained through getInfo. Most of the time, this is an unmodifiable value. If a GraphicsObject has an API dependent implementation, it will have an extension; which can be obtained through getExtension.

## Shader

A shader is code specialized to run on the GPU; this can be compute shaders (calculating things in parallel on the GPU), graphics shaders (~~mesh,~~ vertex, geometry, fragment) or ~~raytracing shaders (raygen, miss, closest hit, any hit, intesection)~~. The ocore abstraction layer allows automatic shader introspection, allowing for the program to inspect all variables used in the shaders. This uses the oiSH format; generated by the oiSH compiler.

### ShaderInfo

#### Constructor

```cpp
String path = ""		//owc-formatted path to the oiSH file
```

#### Data

```cpp
String path;

std::vector<ShaderStage*> stage;				//Vertex/fragment, etc. shader
std::vector<ShaderStageInfo> stages;			//(See ShaderStage)

std::vector<ShaderInput> inputs;				//Entrypoint stage's input
std::vector<ShaderOutput> outputs;				//Final stage's output

std::vector<ShaderRegister> registers;			//The registers (texs, samps, bufs)

std::unordered_map<String, ShaderBufferInfo> 
	buffer;										//The buffer layouts
	
std::unordered_map<String, GraphicsResource*>
	shaderRegister;								//The shader registers
```

### Shader registers

```glsl
layout(binding = 0) uniform sampler samp;
layout(binding = 1) uniform texture2D tex;
```

The GLSL code above generates a shaderRegister with 2 nullptrs; you have to set these values through the Shader interface. Buffers, samplers, textures and texture lists are all registers; their layout is stored in the shader's reflection data. 

```cpp
shader->set("samp", sampler);
shader->set("tex", texture);
shader->get<Sampler>("samp");
shader->get<Texture>("tex");
```

If you pass incorrect data to the shader, it will not work. Only classes inheriting from 'GraphicsResource' can be passed to the shader.

When you update a value in a shader, you don't have to call update manually; it is already done when binding the pipeline (if anything changed).

#### ShaderRegister struct

The ShaderRegister class is a simple struct that contains info about what a register represents. This is backed by the oiSH file format implementation; which has a compacted version.

```cpp
ShaderRegisterType type;		//What the register represents
ShaderRegisterAccess access;	//In what shader stages the resource is accessed
String name;					//Shader's resource name
u32 size;						//Normally 1; except for TextureLists
u32 id;							//Binding of the resource
```

#### ShaderRegisterType OEnum

```cpp
UBO = 1,		//Uniform buffer object / constant buffer (small and fast)
SSBO = 2,		//Shader storage buffer object (big and slow)
Texture2D = 3,	//2D texture handle
Image = 4,		//Writable texture handle
Sampler = 5		//Info on how to sample an image
```

#### ShaderRegisterAccess OEnum

```cpp
Compute = 1,
Vertex = 2,
Geometry = 4,
Fragment = 8
```

### ShaderInput struct

```cpp
TextureFormat type;		//The layout of the input variable
String name;			//Name of the input variable
```

### ShaderOutput struct

```cpp
u32 id;					//Binding of the output variable
TextureFormat type;		//The layout of the output variable
String name;			//Name of the output variable
```

### Shader compilation

The shader compiler could be expanded to take any shader language as input; as long as it can generate valid SPIRV code that can be reflected. The compiler has features such as having custom includes and generating reflection. The shader compiler checks referenced files and if the shader should be updated. In debug it generates SPIRV with variable names and on release it strips them (for size optimization).

The syntax is almost the same as Vulkan GLSL; except the shader version is determined by the compiler and it already enables two extensions (includes & separated stages).

As for includes; using `#include <x.glsl>` will automatically include `res/shaders/x.glsl` while `#include "x.glsl"` will include a file relative to the current file.

#### Baked

I would **highly** suggest using baked shaders instead of compiling them runtime. oiSH compilation isn't fully optimized and shader compilation itself is very expensive. The baking process is done automatically if you put your resource files in `app/res/shaders` from the CMake root.

```cpp
shader = g.create("Simple", ShaderInfo("res/shaders/simple.oiSH"));
g.use(shader);
```

Whenever the shader itself or any of the dependencies (included files) update, the shader will automatically be rebuilt when you run the baker (or if the app project rebuilds). This also happens if the output files don't exist yet.

#### Runtime

If you want to compile a shader at runtime, you can do it using the following code:

```cpp
oiSH::compile(mySource);	//Compile it straight to ShaderInfo
oiSH::convert(mySource);	//Compile it and bake it to oiSH file
```

ShaderSource can be created like following:

```cpp
//From files
ShaderSource mySource("simple", 
	{ "res/shaders/simple.vert", "res/shaders/simple.frag" },
ShaderSourceType::GLSL);

//From source (doesn't allow relative includes)
ShaderSource mySource("simple", ShaderSourceType::GLSL, { 
    { "vert", myVert }, { "frag", myFrag } 
});

//From SPIRV (bytecode; as u32[])
ShaderSource mySource("simple", { 
    { "vert", myVertCopyBuffer }, { "frag", myFragCopyBuffer } 
});
```

## ShaderBuffer

A ShaderBuffer is the representation of a buffer in a shader; it contains all reflection data along with the physical data for the struct. This means that you can get and set data by using variable names (and you don't have to create a struct if you don't want to). Of course you can still layout the data in a buffer and set it, which is still recommended (it is faster). However, this class makes it a lot more user friendly.

This is automatically created by the oiSH reflection data; however the physical data doesn't have to be allocated (specifying `_noalloc` after a buffer's name disables automatic allocation, or when the object is dynamically sized). 

When the data is not allocated; you can use either `instantiate(u32 objects)` or `setBuffer(u32 objects, GBuffer *g)`. Instantiate is for when there is a dynamically sized array at the end of the buffer. Both with setBuffer and instantiate it keeps ownership over the buffer, and will try to destroy it when the shader buffer is destroyed (can be prevented by using `g.use(object)` so the shader buffer doesn't destroy it). setBuffer just allows you to create your own buffer, while instantiate does it for you. The 'objects' is the length of the dynamically sized array at the end of the buffer (if applicable).

This system means that the user doesn't have to care about GPU-alignment. This system abstracts it away. 

### ShaderBufferInfo

#### Constructor

```cpp
ShaderRegisterType type, 	//Type of buffer (UBO/SSBO)
u32 size, 					//Size of buffer
u32 elements, 				//Total element count
bool allocate = true		//If buffer should be allocated
```

#### Data

```cpp
ShaderRegisterType type;
u32 size;
bool allocate;

ShaderBufferObject self;						//The buffer's representation as var
std::vector<ShaderBufferObject> elements;		//All elements in buffer
```

### Functions

```cpp
ShaderBufferVar get(String path);		//Get a variable at path; Buffer and type
ShaderBufferVar get();					//Get the variable for this buffer

u32 getElements();						//Number of children
u32 getSize();							//Size of buffer

GBuffer *getBuffer();					//GPU buffer

//If info.allocate is false, you have to instantiate the buffer yourself
//This can be a dynamic object buffer, light buffer, etc.
ShaderBuffer *instantiate(u32 objects);	

//If info.allocate is false, you can allocate a buffer yourself
//This can be a dynamic object buffer, light buffer, etc.
//The length of the GBuffer should match the ShaderBuffer's length
void setBuffer(u32 objects, GBuffer *g);

//Same as GBuffer's implementation
void flush();
void copy(Buffer buf);

void set(Buffer buf);

//Casting a variable at path and getting it (must match type)
template<typename T>
T &get(String path);

//Casting a variable at path and setting it (must match type)
template<typename T>
void set(String path, T t);
```

### Example

```cpp
//Update per execution shader buffer
ShaderBuffer *perExecution = shader->get<ShaderBuffer>("PerExecution");

perExecution->set("ambient", Vec3(1));
perExecution->set("time", (f32)getRuntime());
perExecution->set("power", 1.f);
perExecution->flush();
```

If you want to apply your changes to the buffer, you have to call 'flush' to push it to the GPU.

Indexing into a variable can be done by using a path;

```cpp
perExecution->get<Vec4>("test/0/myValue");
```

Indexes into test's 0th element and gets the myValue part of it. When using a (multi-dimensional) array, you can do the following:

```cpp
//Example (please ignore padding):
f32 var[2][4] = {
	{ 1, 2, 3, 4 },
	{ 5, 6, 7, 8 }
};

//var[y][x]
//"var" gets everything (1, 2, 3, 4, 5, 6, 7, 8)
//"var/0" gets the first row (1, 2, 3, 4 )
//"var/0/1" gets (2)
    
//If it was an object, you could use the same.
//When referencing a member, it will pick the first object
//"test/val" is the same as test/0/0/val
//"test/1/val" is the same as test/1/0/val
//"test/1/1/val" is the same as test/1/1/val
```

## MeshBuffer

A MeshBuffer is the index buffer and/or vertex buffers that are required to render a model. The requirements of a Mesh are given by the `oiRM::convert` function, as a MeshBufferInfo. This struct can then be modified to allow for multiple mesh allocations. You can also manually create these, like any other GraphicsObject.

### MeshBufferInfo

#### Constructor

```cpp
u32 maxVertices,		//How many vertices are allowed in this MeshBuffer
u32 maxIndices,			//^ indices
std::vector<
    std::vector<
    	std::pair<
    		String, TextureFormat
    	>
    >
> layout,				//The layout of the vertex buffers and names of attributes
TopologyMode topology,	//The topology that is used for this MeshBuffer
FillMode fillMode		//^ fillMode
```

#### Data

```cpp
u32 maxVertices;
u32 maxIndices;

std::vector<std::vector<std::pair<String, TextureFormat>>> buffers;

TopologyMode topologyMode;
FillMode fillMode;

VirtualBlockAllocator *vertices;		//Responsible for allocating vertices
VirtualBlockAllocator *indices;			//Responsible for allocating indices

std::vector<u32> vboStrides;			//The size per vertex for every buffer

std::vector<GBuffer*> vbos;				//Vertex buffers
GBuffer *ibo;							//Index buffers
```

#### Functions

```cpp
void flush();									//Pushes changes to GPU

MeshAllocation alloc(u32 vertices, 
                     u32 indices = 0);			//Allocates space for mesh

bool dealloc(MeshAllocation allocation);		//Deallocates a mesh (if it exists)

bool canAllocate(const MeshBufferInfo &other);	//If a subbuffer can be allocated
```

### TopologyMode OEnum

```cpp
Points = 0,			//Every vertex is a primitive
Line = 1,			//Two vertices per primitive
Line_strip = 2,		//Use last vertex for a line
Triangle = 3,		//Three vertices
Triangle_strip = 4,
Triangle_fan = 5,

//Adjacency
Line_adj = 6,
Line_strip_adj = 7,
Triangle_adj = 8,
Triangle_strip_adj = 9
```

### FillMode OEnum

```cpp
Fill = 0,			//Fill area under primitive
Line = 1,			//Only draw lines around primitive
Point = 2			//Only draw vertices
```

## Mesh

A mesh is defined as an allocation into a MeshBuffer. This MeshBuffer has a layout, that specifies how the vertex data is structured. This data can be maintained manually, or can be automatically set up by the MeshManager. This data is stored in oiRM format to support all functionality; like sized variables.

### MeshInfo

#### Constructor

```cpp
MeshBuffer *parent,			//Maintains all meshes
u32 vertices,				//Number of vertices
u32 indices,				//Number of indices
std::vector<Buffer> vbo;	//VBOs (have to conform to MeshBuffer's layout)
Buffer ibo = {};			//IBO (u32[])
```

#### Data

```cpp
u32 vertices;
u32 indices;

MeshBuffer *buffer;
std::vector<Buffer> vbo;
Buffer ibo;

MeshAllocation allocation;	//The location where the mesh is allocated
```

### MeshAllocation struct

```cpp
u32 baseVertex;				//The model's start vertex
u32 baseIndex;				//^ index
u32 vertices;				//Vertex count
u32 indices;				//^ index

std::vector<Buffer> vbo;	//All vbo data that corresponds to the vbos
Buffer ibo;					//^ ibo
```

### Getters

```cpp
MeshBuffer *getBuffer();
MeshAllocation getAllocation();
```

### Baking a Mesh

Before a Mesh can be used, oibaker has to be ran. This converts files in "app/res/models" from fbx and obj to oiRM format (as well as converting shader files to oiSH format). This format is created to be less limiting and faster; as well as allowing new modern features. oiRM also stores information about the layout, that can be used to create a MeshBuffer.

The inputs of the meshes can be either Fbx or Obj, loading them runtime is **NOT** recommended; but it is possible (see _Reading external formats_).

## MeshManager

A MeshManager is responsible for maintaining meshes and mesh buffers (in a more user friendly way). This automatically allocates buffers when you need them. It uses the MeshAllocationHint to determine that; as well as existing buffers.

### MeshManagerInfo

#### Constructor

```cpp
u32 maxVertices,				//The default max vertices for MeshBuffers
u32 maxIndices,					//^ indices
TopologyMode toplogyMode,		//What TopologyMode to use when it isn't specified
FillMode fillMode				//^ FillMode
```

#### Data

```cpp
u32 vertices, indices;
TopologyMode defTopologyMode;
FillMode defFillMode;

std::unordered_map<String, MeshAllocationInfo> meshAllocations;

std::vector<MeshBuffer*> meshBuffers;
```

### MeshAllocationInfo

#### Constructor (File auto)

This type of allocation tries to find a meshBuffer to allocate into; by using the info from the file. If it can't find any, it will return a nullptr for a Mesh.

```cpp
String path,						//owc-formatted path to the oiRM file

//How the vertices/mesh buffer is allocated
MeshAllocationHint hintVertices = FORCE_EXISTING,

//How many indices to allocate (ALLOCATE_DEFAULT or the number of indices)
MeshAllocationHint hintIndices = ALLOCATE_DEFAULT
```

#### Constructor (File manual)

Tries to allocate into the MeshBuffer; returns nullptr if it fails.

```cpp
String path,						//owc-formatted path to the oiRM file
MeshBuffer *parent					//If null tries to find or allocate MeshBuffer
```

#### Constructor (Data)

To create a Mesh using the vertex and index data, you can use the data constructor. This requires you to name the object, give a parent and provide valid vertex and index data.

```cpp
String name,						//The mesh name
MeshBuffer *meshBuffer,				//The meshBuffer to allocate into
std::vector<Buffer> vbos,			//Data laid out in  MeshBuffer's format
Buffer ibo = {}						//The index data (optional if no indices)
```

#### Data

```cpp
String name;						//Name of the Mesh (if file; = path)
String path;
MeshBuffer *meshBuffer;
MeshAllocationHint hintMaxVertices;
MeshAllocationHint hintMaxIndices;

std::vector<Buffer> vbos;			//Raw vertex input data of the allocation
Buffer ibo;							//Raw index input data

Mesh *mesh;							//Mesh object
```



### MeshAllocationHint enum

```cpp
FORCE_EXISTING = 0xFFFFFFFE,	//Require a mesh buffer to be present (default)
SIZE_TO_FIT = 0xFFFFFFFF,		//Create a mesh buffer with exact size
ALLOCATE_DEFAULT = 0,			//Allocate with size defined in MeshManagerInfo
x = x							//Anything else is the max size in vertices/indices
```

### Functions

```cpp
Mesh *load(MeshAllocationInfo info);			//Loads a mesh (use loadAll instead!)

std::vector<Mesh*> loadAll(std::vector<
					MeshAllocationInfo> &info)	//Loads all meshes (efficiently)

void unload(Mesh *mesh);						//Unloads a mesh and/or buffer
void unloadAll(MeshBuffer *meshBuffer);			//Unloads all meshes for a buffer

Mesh *get(String path);							//Get a loaded mesh from path
bool contains(String path);						//Check if it contains the mesh
```

### Example

```cpp
//Allocate anvil into new MeshBuffer (with default size)
//Allocate sword into existing MeshBuffer
//Allocate sphere into existing MeshBuffer
//Allocate quad into new MeshBuffer (has different layout)

std::vector<MeshAllocationInfo> info = {
	{ "res/models/anvil.oiRM", MeshAllocationHint::ALLOCATE_DEFAULT },
	{ "res/models/sword.oiRM" },
	{ "res/models/sphere.oiRM" },
	{ "res/models/quad.oiRM", MeshAllocationHint::SIZE_TO_FIT }
};

meshes = meshManager->loadAll(info);

//Get our buffers
meshBuffer = meshes[0]->getBuffer();
meshBuffer0 = meshes[3]->getBuffer();
```

## PipelineState

A PipelineState is the required settings for a graphics pipeline; for other pipelines this is unused.

### PipelineStateInfo

#### Constructor

```cpp
DepthMode depth = All,		//The depth mode that is used (mask and test)
BlendMode blend = Alpha,	//What type of blending to use
CullMode cull = Back,		//Use backside, frontside or no culling
WindMode wind = CCW,		//CCW or CW
f32 lineWidth = 1.f,		//Hint for lineWidth (in pixels)
u32 samples = 1				//How many samples to use (MSAA)
```

#### Data

```cpp
f32 lineWidth;
CullMode cullMode;
WindMode windMode;
u32 samples;
BlendMode blendMode;
DepthMode depthMode;
```

## RenderTarget

The term 'render target' refers to the result of rendering. An example is the default back buffer; which can be obtained through Graphics.getBackBuffer. This class automatically takes care of versioning (double buffering, triple buffering, etc.). You can send this to a shader by getting one of the outputs (depth buffer or color outputs). 

```cpp
VersionedTexture *col0 = g.getBackBuffer()->getTarget(0);
Texture *depth = g.getBackbuffer()->getDepth();
```

If you disable depth, the depth texture will be nullptr. You can get the number of targets (color outputs) by using the getTargets function.

### RenderTargetInfo

#### Constructor

##### Graphics RenderTarget

```cpp
Vec2u res, 							//The resolution of the render target
TextureFormat depth, 				//Typically Depth, Undefined for disabled
std::vector<TextureFormat> formats	//List of texture formats for the output
```

##### Compute RenderTarget

```cpp
Vec2u res,							//The resolution of the render target
std::vector<TextureFormat> formats	//List of texture formats for the output
```

This RenderTarget can only be used for compute shaders and don't affect graphics shaders. Therefore, you can use compute shaders like you would use graphics shaders; but you do have to manage the shader registers yourself. This also means that you could have unlimited compute render targets bound, as long as you make sure to unbind them.

#### Data

```cpp
Vec2u res;
u32 targets;								//formats.size()
TextureFormat depthFormat;
std::vector<TextureFormat> formats;

Texture *depth;								//The depth texture
std::vector<VersionedTexture*> textures;	//The output 

bool isComputeTarget;	//Whether or not this should only be used for compute shaders
```

### Functions

```cpp
u32 getVersions();		//How many versions of the texture are made (buffering)
u32 getTargets();

Texture *getDepth();
VersionedTexture *getTarget(u32 i);

Vec2u getSize();
bool isOwned();

bool isComputeTarget();
```

### Example

```cpp
//initSceneSurface
if(renderTarget != nullptr)
    g.destroy(renderTarget);

renderTarget = g.create("Post processing target", 
	RenderTargetInfo(res, TextureFormat::Depth, { TextureFormat::RGBA16f })
);
g.use(renderTarget);

	//Supply our render target to the shader
	shader0->set("tex", renderTarget->getTarget(0));

//destructor
g.finish();
...
g.destroy(renderTarget);
```

## Pipeline

A Pipeline is the required GraphicsObject for a shader to run and setup all important data in the background. The pipeline info depends on the shader type; graphics shaders require more info than a compute shader.

### PipelineInfo

#### Constructor (compute)

```cpp
Shader *shader					//Compute shader
```

#### Constructor (graphics)

```cpp
Shader *shader,					//Graphics shader
PipelineState *state,			//Rasterizer info, blend info, etc.
RenderTarget *target,			//Where to render to
MeshBuffer *meshes				//Which layout to use
```

#### Data

```cpp
Shader *shader;					//Required
PipelineState *pipelineState;	//nullptr for compute
RenderTarget *renderTarget;		//^
MeshBuffer *meshBuffer;			//^
```

### Example

```cpp
//initSceneSurface
if (pipeline != nullptr){
    g.destroy(pipeline);
    g.destroy(pipeline0);
}

	//Rendering to post process target
	pipeline = g.create("Rendering pipeline", 
		PipelineInfo(shader, pipelineState, renderTarget, meshBuffer)
	);
	g.use(pipeline);
	
	//Rendering to back buffer
	pipeline0 = g.create("Post process pipeline", 
		PipelineInfo(shader0, pipelineState, g.getBackBuffer(), meshBuffer0)
	);
	g.use(pipeline0);

//destructor
g.destroy(pipeline);
g.destroy(pipeline0);
```

## Texture

A Texture is an array of attributes. This can be loaded from a file, written to from render target, written to from compute shader or filled in on the CPU.

### TextureInfo

#### Constructor (from file)

```cpp
TextureList *parent,				//The holder of the Texture; can be nullptr
String path,						//Load from file
TextureLoadFormat format = sRGB8	//What type of format to load; TextureLoadFormat
TextureMipFilter mipFilter = Linear;//How the mips get generated (linear or nearest)
```

##### TextureLoadFormat OEnum

```cpp
R8 = 1,
RG8 = 2,
RGB8 = 3,
RGBA8 = 4,
sR8 = 5,
sRG8 = 6,
sRGB8 = 7,
sRGBA8 = 8
```

##### TextureMipFilter UEnum

```cpp
None = 0,
Nearest = 1,
Linear = 2
```

#### Constructor (empty)

```cpp
Vec2u res,						//Resolution of texture
TextureFormat format,			//Any texture format
TextureUsage usage				//Use of texture (render target/depth or image)
```

#### Data

```cpp
Vec2u res;
TextureFormat format;
TextureUsage usage;

String path;
Buffer dat;							//Loaded data from disk (if applicable)
TextureLoadFormat loadFormat = TextureLoadFormat::Undefined;
TextureMipFilter mipFilter = TextureMipFilter::Linear

u32 mipLevels = 1U;					//Automatically detected miplevels

TextureList *parent;				//Owner responsible for the handle
TextureHandle handle = u32_MAX;		//u32_MAX if parent nullptr; otherwise valid

vec2u changedStart, changedEnd;		//If it's updated, this specifies the changed range
```

### TextureFormat OEnum

```cpp
RGBA8 = 1, RGB8 = 2, RG8 = 3, R8 = 4,
RGBA8s = 5, RGB8s = 6, RG8s = 7, R8s = 8,
RGBA8u = 9, RGB8u = 10, RG8u = 11, R8u = 12,
RGBA8i = 13, RGB8i = 14, RG8i = 15, R8i = 16,

RGBA16 = 17, RGB16 = 18, RG16 = 19, R16 = 20,
RGBA16s = 21, RGB16s = 22, RG16s = 23, R16s = 24,
RGBA16u = 25, RGB16u = 26, RG16u = 27, R16u = 28,
RGBA16i = 29, RGB16i = 30, RG16i = 31, R16i = 32,
RGBA16f = 33, RGB16f = 34, RG16f = 35, R16f = 36,

RGBA32f = 37, RGB32f = 38, RG32f = 39, R32f = 40,
RGBA32u = 41, RGB32u = 42, RG32u = 43, R32u = 44,
RGBA32i = 45, RGB32i = 46, RG32i = 47, R32i = 48,

RGBA64f = 49, RGB64f = 50, RG64f = 51, R64f = 52,
RGBA64u = 53, RGB64u = 54, RG64u = 55, R64u = 56,
RGBA64i = 57, RGB64i = 58, RG64i = 59, R64i = 60,

D16 = 61, D32 = 62, D16S8 = 63, D24S8 = 64, D32S8 = 65, Depth = 66,

sRGBA8 = 67, sRGB8 = 68, sRG8 = 69, sR8 = 70,

BGRA8 = 71, BGR8 = 72,
BGRA8s = 73, BGR8s = 74,
BGRA8u = 75, BGR8u = 76,
BGRA8i = 77, BGR8i = 78,
sBGRA8 = 79, sBGR8 = 80
```

The texture format is expressed like following:

`<prefix>[<channels><bpc>]<suffix>`

Where prefix can be 's' for allowing sRGB color space. Suffix can be nothing (unorm), s (snorm), u (uint), i (int), f (float). When something is suffixed with 's' it can't have a suffix and bits per channel (bpc) has to be 8.

The exception to this rule is 'Depth' which generally evaluates to D32S8 (32-bit depth, 8-bit stencil), unless the device doesn't support it. If D32S8 is not supported, it will check D32, D24S8, D16S8 and D16 (in that order).

### TextureUsage OEnum

```cpp
Render_target = 1,			//For rendering 'color'; a non-depth TextureFormat
Render_depth = 2,			//For rendering depth
Image = 3					//Just data
```

### TextureFormatStorage enum

```cpp
INT, 						//suffix == "i"
UINT, 						//suffix == "u"
FLOAT, 						//suffix == "f" || suffix == "" || suffix == "s"
DOUBLE						//endsWith "64f"
```

### Functions

```cpp
TextureFormat getFormat();
TextureUsage getUsage();
Vec2u getSize();
bool isOwned();

TextureHandle getHandle();
void initParent(TextureList *parent);		//Set parent if it is null

void flush();								//If the texture has been updated

bool setPixels(Vec2u start, Vec2u length, Buffer values);
bool getPixels(Vec2u start, Vec2u length, CopyBuffer &output);

bool write(String path, Vec2u start = Vec2u(), Vec2u length = Vec2u());
bool read(String path, Vec2u start = Vec2u(), Vec2u length = Vec2u());
```

getPixels allows you to read the current texture data (even of render targets and depth buffers), which could be used for screenshots. setPixels allows you to update the current texture data (only of images); this is useful with textures that can't immediately push all user-data, like 3D volume textures and 2D tilemaps. Both functions allow you to specify a start and length of the data; the Buffer passed to setPixels has to be the exact same format as the texture (`length.y * length.x * stride`).

read and write allow you to export/import textures from disk into the current texture, allowing you to 'take a screenshot' and load user data.

## Sampler

A sampler is the information required to get a sample from a texture.

### SamplerInfo

```cpp
SamplerMin minification,		//Minification filter; when result is smaller
SamplerMag magnification,		//Magnification filter; when result is bigger
SamplerWrapping s,				//Wrapping on the s-axis (srt; aka uvw)
SamplerWrapping r,				//^ r-axis
SamplerWrapping t,				//^ t-axis
u32 aniso = 16					//Max anisotropic filter; hardware dependent
```

### SamplerWrapping OEnum

```cpp
Repeat = 0, 				//Repeat [x,y] in range [0,1] UV
Mirror_repeat = 1, 			//^ mirror
Clamp_edge = 2, 			//Extend edges indefinitely 
Clamp_border = 3,			//Stop edges at border
Mirror_clamp_edge = 4		//Mirrored edges
```

### SamplerMin OEnum

```cpp
Linear_mip = 0, 			//Use linear mips and linear filtering
Nearest_mip = 1, 			//^ nearest
Linear = 2, 				//Use linear filter (no mips)
Nearest = 3, 				//^ nearest
Linear_mip_nearest = 4,		//Linear filtering and nearest mips
Nearest_mip_linear = 5		//Nearest filtering and linear mips
```

### SamplerMag OEnum

```cpp
Linear = 0,					//Linear filtering
Nearest = 1					//Nearest filtering
```

## DrawList

A DrawList is like a multi draw indirect call; it automatically batches meshes together as one call to the GPU. This requires the Mesh to be in the same MeshBuffer as specified in the DrawList.

### DrawListInfo

#### Constructor

```cpp
MeshBuffer *meshBuffer,			//The MeshBuffer this DrawList uses
u32 maxBatches,					//How many separate mesh draw commands it can fit
bool clearOnUse = true			//If you want to re-use the DL, set this to false
```

#### Data

```cpp
u32 maxBatches;
bool clearOnUse;

MeshBuffer *meshBuffer;
GBuffer *drawBuffer;			//The GPU object representing the DrawList

std::vector<
    std::pair<Mesh*, u32>
> objects;						//The drawlist
```

### Functions

```cpp
u32 getBatches();
u32 getMaxBatches();
GBuffer *getBuffer();

void clear();					//Clears draw list
void flush();					//Prepares draw list for draw

void draw(Mesh *m, 
          u32 instances);		//How many objects to draw of a Mesh
```

### Example

```cpp
//Create drawlist (initScene)
drawList = g.create("Draw list (main geometry)", 
                    DrawListInfo(meshBuffer, 256, false));
g.use(drawList);

//Prepare drawlist (initScene)
drawList->draw(meshes[2], 1);		//Reserve index 0 for the sphere
drawList->draw(meshes[4], 1);		//Reserve index 1 for the planet
drawList->flush();

//Destructor
g.destroy(drawList);
```

The example above means that `meshes[2]` (a sphere) is located at object id 0, while `meshes[4]` (a planet) is located at object id 1. You manually have to group instances together; using draw when the Mesh entry already exists is not allowed.

Now you have to create a struct for every object; which gets indexed in the shader:

```c++
struct MyTestObject {	//9 lines; 144 bytes
  
    Matrix m, mvp;
    
    Vec3u padding;
    Materialhandle material;
    
} objects[2];
```

This means that `objects[0]` is the sphere's object info and `objects[1]` is the planet's object info.

## ComputeList

Similar to the DrawList counterpart. ComputeList is responsible for requesting compute shader dispatches. This allows you to request a bunch of different dispatches from the same compute shader. This could be used to manage particle emitters or update layers of a texture; as well as allowing multi-pass texture updates (that require waiting for the last result).

### ComputeListInfo

#### Constructor

```cpp
Pipeline *computePipeline;		//Pipeline with compute shader
u32 maxDispatches;				//Maximum number of dispatches
bool clearOnUse = false;		//If the compute list has to be cleared after use
```

#### Data

```cpp
Pipeline *computePipeline;
u32 maxDispatches;
bool clearOnUse;

std::vector<Vec3u> dispatches;			//Stores the dispatched groups
GBuffer *dispatchBuffer;				//Buffer for storing dispatches
```

### Functions

```cpp
//Getting dispatch information

u32 getMaxDispatches();
u32 getDispatches();
GBuffer *getDispatchBuffer();

//Clearing and pushing changes

void clear();
void flush();

//Dispatch AT LEAST xyz threads
//But dispatches more if xyz % getThreadsPerGroup() != 0

u32 dispatchThreads(u32 threads);
Vec2u dispatchThreads(Vec2u threads);
Vec3u dispatchThreads(Vec3u threads);

//Dispatch xyz groups of getThreadsPerGroup()

u32 dispatchGroups(u32 groups);
Vec2u dispatchGroups(Vec2u groups);
Vec3u dispatchGroups(Vec3u groups);

//The local size; threads per group

Vec3u getThreadsPerGroup();
```

### Example

```cpp
cmdList->begin(computeTarget);
cmdList->bind(computePipeline);
cmdList->dispatch(computeList);
cmdList->end(computeTarget);
```

The example above (located in render) prepares the compute target to be used in the dispatch call.

```cpp
Vec2u targetRes = computeList->dispatchThreads(res);
computeList->flush();

computeTarget = g.create("Compute target", 
                         RenderTargetInfo(targetRes, { TextureFormat::RGBA16f }));
g.use(computeTarget);

computeShader->set("outputTexture", computeTarget->getTarget(0));
```

The example above (located in initSceneSurface) tries to dispatch res (resolution) number of threads. If this resolution doesn't match with the group size, it will dispatch more threads. To ensure this can still function optimally, the render target will use this size (so the compute shader can run over all threads the same way). But will have to be sampled with this limitation in mind. We send this RGBA16f target to the compute shader, so it can output to it.

## CommandList

A CommandList contains all commands to send to the GPU.

### CommandListInfo

A command list is mostly API dependent, so it doesn't require any additional information.

### Functions

```cpp
void begin();								//Prepare the command list
void begin(RenderTarget *target, 
           RenderTargetClear clear = {});	//Prepare the RT and/or clear it

void end();									//Submit the command list
void end(RenderTarget *target);				//Finalize the RT

void bind(Pipeline *pipeline);				//Use a pipeline
void draw(DrawList *drawList);				//Execute a draw list
```

### Example

```cpp
//render function

//Start rendering
cmdList->begin();

//Render to renderTarget

	//Write to our post processing target and draw objects
	cmdList->begin(renderTarget);
	cmdList->bind(pipeline);
	cmdList->draw(drawList);
	cmdList->end(renderTarget);

//Render to backbuffer

	//Execute our post processing shader
	cmdList->begin(g.getBackBuffer());
	cmdList->bind(pipeline0);
	cmdList->draw(drawList0);
	cmdList->end(g.getBackBuffer());

//Stop rendering
cmdList->end();
```

Above you see that we bind the post processing target, use the default pipeline (forward phong shader) and draw our two objects. Afterwards, we write to the back buffer with the post processing shader and execute it. We then execute our command list.

## TextureList

Is the object created to store multiple textures; so a TextureHandle (uint) can used to identify a Texture. TextureList doesn't currently support VersionedTextures.

### TextureListInfo

#### Constructor

```cpp
u32 count						//How many textures it can hold
```

#### Data

```cpp
std::vector<Texture*> textures;

CommandList *textureCommands;
```

### Functions

```cpp
Texture *get(TextureHandle i);				//Getting a Texture from TextureHandle
TextureHandle alloc(Texture *tex);			//Allocating a TextureHandle
void dealloc(Texture *tex);					//Deallocating a TextureHandle

u32 size();
```

### Example

```cpp
//Allocate our textures into a TextureList
textureList = g.create("Textures", TextureListInfo(2));
shader->set("tex", textureList);

//Allocate textures
trock = g.create("rock", TextureInfo(textureList, "res/textures/rock_dif.png"));
g.use(trock);

twater = g.create("water", TextureInfo(textureList, "res/textures/water_dif.png"));
g.use(twater);
```

## MaterialList

MaterialList has all materials; this is needed for the way draw calls are structured. This means that a MaterialHandle (uint) can be used to identify a Material. It is used for material allocation and deallocation.

### MaterialListInfo

#### Constructor

```cpp
TextureList *textures,		//The textures that are used for these materials
u32 maxCount				//How many materials can be used
```

#### Data

```cpp
TextureList *textures;

bool notified = false;		//True if any submaterials have been updated
u32 size;					//aka maxCount

GBuffer *buffer = nullptr;	//Buffer that stores material structs
```

### Functions

```cpp
MaterialStruct *alloc(MaterialStruct info);		//Allocate a struct into buffer
bool dealloc(MaterialStruct *ptr);				//Deallocate

MaterialStruct *operator[](MaterialHandle handle);				//Get by handle
const MaterialStruct *operator[](MaterialHandle handle) const;	//^ const

u32 getSize() const;							//Get size in materials
u32 getBufferSize() const;						//Get size in bytes (* 128)
GBuffer *getBuffer() const;						//Get GBuffer
```

## Material

A material is a combination of parameters and textures that can be applied to models. These parameters are predefined, but an internal interpretation of materials could always be used; as long as it evaluates to the physically based rendering / raytracing material system. These values are stored in a MaterialStruct but updated through the Material class. 

### MaterialInfo

#### Constructor

```cpp
MaterialList *parent;
```

After calling the constructor (with a parent), you can access the material info through the 'temp' variable; which stores the information before it is allocated. Alternatively, you can always use the MaterialStruct pointer to get the current MaterialStruct (temp or allocated) and this can also be done through the `->` operator.

```cpp
MaterialInfo info(materialList);
info->diffuse = Vec3(0.5f, 0, 1);
info->ambient = Vec3(0.5f, 0.5f, 0.25f);
//Allocate material
```

Or it can be done through the allocated material to use the more user-friendly interface.

```cpp
Material *m = g.create("My material", MaterialInfo(materialList));
m->setDiffuse(Vec3(0.5f, 0, 1));
m->setAmbient(Vec3(0.5f, 0.5f, 0.25f));

//Setting a texture:
m->setDiffuse(myTexture);
```

#### Data

```cpp
MaterialStruct temp;			//Temporary instance (for initialization)

MaterialList *parent;			//Parent; material buffer
MaterialStruct *ptr;			//Current material struct; temp or allocated
```

### Functions

```cpp
MaterialHandle getHandle() const;
MaterialList *getParent() const;

void setDiffuse(Vec3 dif);
void setAmbient(Vec3 amb);
void setShininess(f32 shn);
void setEmissive(Vec3 emi);
void setShininessExponent(f32 sne);
void setSpecular(Vec3 spc);
void setRoughness(f32 rgh);
void setMetallic(f32 met);
void setTransparency(f32 trn);
void setClearcoat(f32 clc);
void setClearcoatGloss(f32 clg);
void setReflectiveness(f32 rfn);
void setSheen(f32 shn);

void setDiffuse(Texture *difTex);
void setOpacity(Texture *opcTex);
void setEmissive(Texture *emiTex);
void setRoughness(Texture *rghTex);
void setAmbientOcclusion(Texture *aocTex);
void setHeight(Texture *hghTex);
void setMetallic(Texture *metTex);
void setNormal(Texture *nrmTex);
void setSpecular(Texture *spcTex);
```

### MaterialStruct

```cpp
//All material info; 128 bytes (8 lines)
struct MaterialStruct {

	Vec3 diffuse = 1.f;
	MaterialHandle id = u32_MAX;

	Vec3 ambient = 0.1f;
	f32 shininess = 0.f;

	Vec3 emissive = 0.f;
	f32 shininessExponent = 1.f;

	Vec3 specular = 1.f;
	f32 roughness = 0.5f;

	f32 metallic = 0.5f;
	f32 transparency = 0.f;
	f32 clearcoat = 0.5f;
	f32 clearcoatGloss = 0.5f;

	f32 reflectiveness = 0.f;
	f32 sheen = 0.f;
	TextureHandle t_diffuse = 0;			//sRGB8 (3 Bpp)
	TextureHandle t_opacity = 0;			//R8 (1 Bpp)

	TextureHandle t_emissive = 0;			//RGB16 (6 Bpp)
	TextureHandle t_roughness = 0;			//R8 (1 Bpp)
	TextureHandle t_ao = 0;					//R8 (1 Bpp)
	TextureHandle t_height = 0;				//R8 (1 Bpp)

	TextureHandle t_metallic = 0;			//R8 (1 Bpp); Metallic
	TextureHandle t_normal = 0;				//RGB8s (3 Bpp)
	TextureHandle t_specular = 0;			//R8 (1 Bpp)
	u32 p0 = 0;

};
```

A MaterialStruct is the GPU representation of a material; a handle can be used to reference one in a MaterialList.

## VersionedTexture

A versioned texture is required for when there are more than one buffer present. This number can vary, but on modern devices it is 2 or 3 (2 = double buffering, 3 = triple buffering). However, there could also just be 1 version (only rendering to texture). This class is for handling how RenderTargets are implemented behind the scenes. 

### VersionedTextureInfo

#### Constructor

```cpp
std::vector<Texture*> textures;	//Each buffer needs a texture; e.g. triple buffering
```

#### Data

```cpp
u32 versions;					//should be equal to Graphics's buffering variable
std::vector<Texture*> version;
```

### Functions

```cpp
TextureFormat getFormat();
TextureUsage getUsage();
Vec2u getSize();
u32 getVersions();
bool isOwned();

Texture *getVersion(u32 i);
```

## Camera

A camera is defined as an object in space; as simple as having a current eye and a center or direction, as well as a normal (pointing up). Unlike most camera implementations, the clipping planes, aspect ratio, resolution and FOV aren't implemented into the camera. This is because you might have multiple frusta for one camera (aka; different views). For resource reuse, we decided to create a separate object "CameraFrustum", which has the projection matrix, while the camera has the view matrix. These two objects are combined into a View in order to create the VP matrix.

### CameraInfo

#### Constructor

```cpp
ViewBuffer *parent,		//The object that stores the cameras, frusta and views
Vec3 position,			//The location of the camera
Vec4 directionOrCenter,	//When w=0; forward direction, w=1; focus point
Vec3 up					//The camera's up direction (normal)
```

#### Data

```cpp
CameraStruct temp;		//The struct for when the GPU version isn't allocated yet

ViewBuffer *parent;
CameraStruct *ptr;		//The pointer to the temporary or allocated GPU struct
```

### CameraStruct (GPU)

```cpp
struct CameraStruct {	//112 bytes (7 'lines'; 7 * 16)
    
    Matrix v;			//View matrix

    Vec3 position;		//Camera 'eye'
    f32 p0;				//Padding (0)

    Vec3 up;			//Normal / camera up direction
    f32 p1;				//Padding (0)

    Vec4 forward;		//Either forward direction or center
    
};
```

### Functions

```cpp
void move(Vec3 dposition);				//Move with dposition (position += dposition)

void setPosition(Vec3 position);		//Move to position
void setUp(Vec3 up);					//Set the normal/up of the camera
void setDirection(Vec3 direction);		//Set the forward direction
void setCenter(Vec3 center);			//Set the center (focus point)

CameraHandle getHandle();				//Get the index in the parent
ViewBuffer *getParent();
```

## CameraFrustum

A frustum is the projection matrix. This object has to be updated when the aspect ratio and/or resolution changes.

### CameraFrustumInfo

#### Constructor

```cpp
ViewBuffer *parent,		//The object that stores the cameras, frusta and views
Vec2u res,				//The resolution
f32 aspect,				//The aspect ratio (not always res.x / res.y)
f32 fov,				//The field of view
f32 near,				//Near plane (depth = 0)
f32 far					//Far plane (depth = 1)
```

#### Data

```cpp
CameraFrustumStruct temp;	//The struct for when the GPU version isn't allocated yet

ViewBuffer *parent;
CameraFrustumStruct *ptr;	//The pointer to the temporary or allocated GPU struct
```

### CameraFrustumStruct (GPU)

```cpp
struct CameraFrustumStruct {	//96 bytes (6 lines)
    
    Matrix p;					//Projection matrix

    f32 near;					//Near clipping plane (depth = 0)
    f32 far;					//Far clipping plane (depth = 1)
    f32 aspect;					//Aspect ratio (resolution.x / resolution.y)
    f32 fov;					//FOV in degrees

    Vec2u resolution;			//Resolution (w, h) in pixels
    u32 p0;						//Padding (0)
    u32 p1;						//Padding (0)
    
};
```

### Functions

```cpp
void resize(Vec2u size);
void setAspect(f32 aspect);
void setNear(f32 near);
void setFar(f32 far);
void setFov(f32 fov);

void resize(Vec2u size, f32 aspect);

CameraFrustumHandle getHandle();
ViewBuffer *getParent();
```

## View

A view is the view projection matrix; it is a combination of a Camera and a CameraFrustum. This object is automatically updated when one of the referenced objects changes. Keep in mind that if it is updated, you have to change all pre-baked MVPs.

### ViewInfo

#### Constructor

```cpp
ViewBuffer *parent,				//Stores the cameras, frusta and views
Camera *camera,					//The transformation of the View
CameraFrustum *frustum			//The projection of the View
```

#### Data

```cpp
ViewStruct temp;	//The struct for when the GPU version isn't allocated yet

ViewBuffer *parent;
ViewStruct *ptr;	//The pointer to the temporary or allocated GPU struct
```

### ViewStruct (GPU)

```cpp
struct ViewStruct {				//80 bytes (5 lines)

    CameraHandle camera;		//Index of camera in parent
    ViewFrustumHandle frustum;	//Index of frustum in parent
    u32 p0;						//Padding (0)
    u32 p1;						//Padding (0)

    Matrix vp;					//ViewProjection matrix (vp * m = mvp)
    
};
```

### Functions

```cpp
void setCamera(Camera *cam);
void setFrustum(CameraFrustum *frustum);

ViewHandle getHandle();
ViewBuffer *getParent();
```

## ViewBuffer

The view buffer is the object responsible for managing all views, cameras and frusta. When one is updated, it updates the matrix and sends the data to the GPU.

### ViewBufferInfo

#### Constructor

Empty constructor; it automatically stores 128 cameras, frusta and 256 views. These should be allocated after the ViewBuffer has been allocated.

#### Data

```cpp
StaticObjectAllocator<CameraStruct, cameraCount> cameras;			//128 cams
StaticObjectAllocator<CameraFrustumStruct, frustumCount> frusta;	//128 frusta
StaticObjectAllocator<ViewStruct, viewCount> views;					//128 views

StaticBitset<cameraCount + frustumCount + viewCount> updated;		//Which updated

GBuffer *buffer;													//UBO for data
```

### ViewBufferStruct (GPU)

```cpp
//128 * (7 + 6) + 256 * 5 = 128 * 23 lines = 2 944 lines = 46 KiB
struct ViewBufferStruct {
  
    CameraStruct cameras[128];			//All cameras
    CameraFrustumStruct frusta[128];	//All frusta
    View views[256];					//All views
    
};
```

### Functions

```cpp
void update();					//Update all camera, viewport and view matrices

//Allocate cam, frust or view
CameraStruct *alloc(const CameraStruct &cs);
CameraFrustumStruct *alloc(const CameraFrustumStruct &vps);
ViewStruct *alloc(const ViewStruct &vs);

//Dealloc
void dealloc(CameraStruct *cs);
void dealloc(CameraFrustumStruct *vps);
void dealloc(ViewStruct *vs);

//Get handle
CameraHandle get(CameraStruct *cs);
CameraFrustumHandle get(CameraFrustumStruct *vps);
ViewHandle get(ViewStruct *vs);

//Get struct from handle
CameraStruct *getCamera(CameraHandle cam);
CameraFrustumStruct *getFrustum(CameraFrustumHandle vp);
ViewStruct *getView(ViewHandle v);

GBuffer *getBuffer();			//Get the buffer with all data
```

### Example

```cpp
//initScene

	//Setup our view buffer
	views = g.create("Default view buffer", ViewBufferInfo());
	g.use(views);

	//Setup our camera
	camera = g.create("Default camera", 
                      CameraInfo(views, Vec3(3, 3, 3), Vec4(0, 0, 0, 1))
                     );
	g.use(camera);

	//Setup our viewport (no size set)
	cameraFrustum = g.create("Default viewport", 
						CameraFrustumInfo(views, Vec2u(1, 1), 1, 40, 0.1f, 100)
                    );
	g.use(cameraFrustum);

	//Setup our view
	view = g.create("Default view", ViewInfo(views, camera, cameraFrustum));
	g.use(view);

//onAspectChange
cameraFrustum->resize(getParent()->getInfo().getSize(), asp);

//update
... update cameras, frusta, etc.
views->update();
... rendering

//destructor
g.destroy(view);
g.destroy(cameraFrustum);
g.destroy(camera);
g.destroy(views);
```

## ShaderStage

A shader stage is a part of a shader; this could be the vertex, fragment, geometry or compute shader. This is then combined with reflection data to form a shader object.

### ShaderStageInfo

#### Constructor / data

```cpp
CopyBuffer code;						//The bytecode for this stage (uint[]; SPIRV)
ShaderStageType type;					//The type of the stage

std::vector<ShaderInput> input;			//Inputs of the stage
std::vector<ShaderOutput> output;		//Outputs of the stage
```

## GBuffer

### GBufferInfo

#### Constructor (buffer)

```cpp
GBufferType type;		//The type of the buffer
Buffer buffer;			//The data of the buffer
```

#### Constructor (size)

```cpp
GBufferType type;		//The type of the buffer
u32 size;				//Empty buffer with size
```

#### Data

```cpp
GBufferType type;
Buffer buffer;

bool hasData;			//Whether or not there's data initialized (buffer constructor)
```

### GBufferType OEnum

```cpp
UBO = 0, 				//Uniform buffer object (<64 KiB buffer)
SSBO = 1, 				//Shader storage buffer
IBO = 2, 				//Index buffer
VBO = 3, 				//Vertex buffer
CBO = 4					//Command buffer (multi draw indirect)
```

### Functions

```cpp
GBufferType getType();

u32 getSize();
u8 *getAddress();
Buffer getBuffer();

bool set(Buffer buf);				//Copies data to CPU buffer

void flush(Vec2u range);			//Push data to GPU
```

# Reading external formats

## FBX

An FBX is more than just a mesh; it is the entire scene (transformations, names, cameras, lights, etc.). However, with the FBX struct; it is made to just parse the mesh (& material) info from it.

```cpp
//Convert 'myModels's models to oiRMc format (as buffers)
//The key is normally the name of the model; unless they are unnamed, then it uses id
std::unordered_map<String, Buffer> oiRMs = Fbx::convertMeshes(
    "res/models/myModels.fbx", true);

//Output model into oiRM file
//If there are multiple; it will output myModel.name.oiRM, myModel.otherName.oiRM
//If the objects aren't named; they use an id (myModel.0.oiRM, myModel.1.oiRM)
Fbx::convertMeshes("res/models/myModel.fbx", "out/models/myModel.oiRM");
```

The constraint of Fbx is that you can't load it directly (onto GPU); it has to be converted and that file has to be loaded instead. This is because Fbx loading isn't optimized and the format itself doesn't support all things oiRM does. 

### FbxFile

All data that is lost in the Fbx->oiRM conversion can be obtained by using `FbxFile::read("res/models/myModel.fbx")`. This will give you a node tree that has all data of an Fbx file; which is used by the converter to obtain all data.

```cpp
FbxFile *myFile = FbxFile::read("res/models/myModel.fbx");	//Load node tree
u32 version = myFile->getVersion();							//7500 = 7.5, 7400 = 7.4
bool isValid = myFile->isValid();							//Checks if valid header
FbxNode *myFileNode = myFile->get();						//File as node
```

### FbxNode

Each node can have properties and children; you can read them like following:

```cpp
//Get number of children and loop through them
u32 children = myFileNode->getChildren();
for(FbxNode *node : myFileNode->getChildArray())
    ;

//Get number of properties and loop through them
u32 properties = myFileNode->getProperties();
for(FbxProperty *prop : myFileNode->getPropertyArray())
    ;

//Get some random info
FbxNode *child = myFileNode->getChild(0);
FbxProperty *prop = myFileNode->getProperty(0);
String name = myFileNode->getName();
```

Unlike most other file formats; a node is **not unique**, which means that there might be multiple nodes named "Geometry". This is because they are treated like an object; the name and values matter; not just the name (like it does for JSON).

An FBX file can be used to define anything; it could be used like a binary version of JSON (but with duplicated nodes and childs). You just have to define how to interpret that node and property data. 

```cpp
//Find nodes by name
myFileNode->findNodes("Objects/Model");		//Get all model nodes
myFileNode->findNodes("Objects/Geometry"); 	//Get all geometry nodes

//Find specific nodes by property
myFileNode->findNodes("Objects/Model", { 2 }, String("Mesh"))
```

As seen above; the `findNodes(String)` function allows you to traverse the entire node and find all nodes named "Objects"; in those nodes it will find nodes named "Model" or "Geometry".

The second usage of findNodes is more complicated; it is variadic. This means that you can input any property value and compare it **(If it is a String; it must be String type; not `std::string`, not `const char*`)**. If it matches, it will be added. This means that our query gives the index of the property and the value. 

```cpp
//Will find all nodes in Imaginary's nodes 'Doesnt' with name 'Exist'
//property[1] (2nd property) == "Hello" (String)
//property[2] == 32 (i32)
//property[3] == true (bool)
myFileNode->findNodes("Imaginary/Doesnt/Exist", { 1, 2, 3 }, String("Hello"), 32, true);
```

### FbxProperty

An FbxProperty has virtual functions for reading, cloning and getting the type id of what the property represents. Before you can use a property, you have to cast it to the appropriate version (`.cast<T>`).

```cpp
FbxProperty *myProperty = ...;

//Get the element; if array; get 0th element
String &str = myProperty->cast<FbxString>().get();

//Get as array
FbxDoubleArray &val = myOtherProperty->cast<FbxDoubleArray>();

for(u32 i = 0; i < val.size(); ++i)
    f64 &v = val[i];

```

Be sure that the cast you're doing is valid; or it will crash. The following types are supported:

|      Name      | Identifier |  Type  | Size (bytes) | Compression support |
| :------------: | :--------: | :----: | :----------: | :-----------------: |
|    FbxShort    |     Y      |  i16   |      2       |         no          |
|    FbxBool     |     C      |  bool  |      1       |         no          |
|     FbxInt     |     I      |  i32   |      4       |         no          |
|    FbxFloat    |     F      |  f32   |      4       |         no          |
|   FbxDouble    |     D      |  f64   |      8       |         no          |
|    FbxLong     |     L      |  i64   |      8       |         no          |
| FbxFloatArray  |     f      |  f32   |      4n      |         yes         |
| FbxDoubleArray |     d      |  f64   |      8n      |         yes         |
|  FbxLongArray  |     l      |  i64   |      8n      |         yes         |
|  FbxIntArray   |     i      |  i32   |      4n      |         yes         |
|  FbxBoolArray  |     b      |  bool  |      n       |         yes         |
|   FbxBuffer    |     R      | Buffer |      n       |         no          |
|   FbxString    |     S      | String |      n       |         no          |

## Obj

```cpp
//Convert to oiRMc
Obj::convert("res/models/test.obj", "out/models/test.oiRM", true);
```

Since a wavefront obj only has data in plain text, supporting it runtime is a bad idea. It is quite heavy to parse, way bigger than an Fbx and doesn't support any modern features. It is only supported for backwards compatibility. 

# Example code

ogc hooks into owc, it requires information about the current Window for setting up a swapchain and all information about the backbuffer. All of these get passed through a WindowInterface, which is what GraphicsInterface builds on. GraphicsInterface provides you with a few extra functions that allow you to manage events yourself, while BasicGraphicsInterface provides you with pre-initialized GraphicsObjects that you might not want to create.

```cpp
//Called when the scene should be initialized
void initScene() override;

//Called when the frame has been updated (resize)
void initSceneSurface(Vec2u res) override;

//Called to render the scene
void renderScene() override;
```

## BasicGraphicsInterface

### Pre-initialized values

```cpp
Sampler *linearSampler;				//{ Linear, Linear, Repeat }
Sampler *nearestSampler;			//{ Nearest, Nearest, Clamp_border }
ViewBuffer *views;					//{ }
Camera *camera;						//{ views, Vec3(3), Vec4(0, 0, 0, 1) }
CameraFrustum *cameraFrustum;		//{ views, Vec2u(1), 1, 40, 0.1f, 100 }
View *view;							//{ views, camera, cameraFrustum }
CommandList *cmdList;				//{ }
MeshManager *meshManager;			//{ 400'000, 500'000 }
```

## Resizing

When you resize your screen, a lot of data becomes invalid. The back buffer suddenly changes resolution  and all render targets now have the wrong resolution. This means you have to destroy all pipelines and render targets that have the wrong resolution. Remember that if the render target is used in a shader, you also have to update the value in the shader.

```cpp
void MainInterface::initSceneSurface(Vec2u res){

	//Destroy old data

	if (pipeline != nullptr) {
		g.destroy(renderTarget);
		g.destroy(pipeline);
		g.destroy(pipeline0);
	}

	//Recreate render targets and pipelines
    
    //Create result of rendering (with HDR)
	renderTarget = g.create("Post processing target",
			RenderTargetInfo(res, TextureFormat::Depth, { TextureFormat::RGBA16f })
	);
	g.use(renderTarget);

    //Update post processing shader
	shader0->set("tex", renderTarget->getTarget(0));

    //drawList -> Rendering pipeline -> renderTarget
	pipeline = g.create("Rendering pipeline", 
			PipelineInfo(shader, pipelineState, renderTarget, meshBuffer)
	);
	g.use(pipeline);

	//renderTarget -> Post processing pipeline -> back buffer
	pipeline0 = g.create("Post process pipeline", 
			PipelineInfo(shader0, pipelineState, g.getBackBuffer(), meshBuffer0)
	);
	g.use(pipeline0);
    
}
```

Above all the pipelines are recreated, because the render targets have been changed. The post processing target (the target we apply post processing to) gets used as the input for our post processing shader (shader0).

## Destructor

When you finish your program, you have to tell Graphics to finish the current frame(s) and to destroy all objects you created. 

```cpp
MainInterface::~MainInterface(){
	g.finish();
	g.destroy(rock);
	g.destroy(water);
	g.destroy(trock);
	g.destroy(twater);
	g.destroy(textureList);
	g.destroy(materialList);
	g.destroy(drawList);
	g.destroy(drawList0);
	g.destroy(renderTarget);
	g.destroy(pipeline);
	g.destroy(pipeline0);
	g.destroy(shader);
	g.destroy(shader0);
}
```

## Rendering the scene

Rendering the scene only includes putting instructions into the command list. Other updates (like updating shader values, buffer values, etc.) are done in the update loop. 

```cpp
void MainInterface::renderScene(){

	//Start rendering
	cmdList->begin();

	//Render to renderTarget

		//Bind fbo and pipeline
		cmdList->begin(renderTarget);
		cmdList->bind(pipeline);
		cmdList->draw(drawList);
		cmdList->end(renderTarget);

	//Render to backbuffer

		//Execute our post processing shader
		cmdList->begin(g.getBackBuffer());
		cmdList->bind(pipeline0);
		cmdList->draw(drawList0);
		cmdList->end(g.getBackBuffer());

	cmdList->end();

}
```

## Setting up the scene

All shader variables have to be set and all graphics objects created. This is done in the 'initScene' function.

```cpp
void MainInterface::initScene() {

	BasicGraphicsInterface::initScene();

	//Setup our shader (forward phong shading)
	shader = g.create("Simple", ShaderInfo("res/shaders/simple.oiSH"));
	g.use(shader);

	//Setup our post process shader (tone mapping & gamma correction)
	shader0 = g.create("Post process", 
                       ShaderInfo("res/shaders/post_process.oiSH"));
	g.use(shader0);

    //First: Allocate anvil into new MeshBuffer (with default size)
    //Second: Allocate sword into existing MeshBuffer
    //Third: Allocate sphere into existing MeshBuffer
    //Last: Allocate quad into new MeshBuffer (same size as mesh)
	std::vector<MeshAllocationInfo> info = {
		{ "res/models/anvil.oiRM", MeshAllocationHint::ALLOCATE_DEFAULT },
        { "res/models/sword.oiRM" },
        { "res/models/sphere.oiRM" },
        { "res/models/quad.oiRM", MeshAllocationHint::SIZE_TO_FIT }
	};

    //Load our models
	meshes = meshManager->loadAll(info);
	meshes.push_back(nullptr);					//Reserve planet model

    //Get our mesh buffers
	meshBuffer = meshes[0]->getBuffer();
	meshBuffer0 = meshes[3]->getBuffer();

	//Read in planet model
	refreshPlanetMesh(true);

	//Set up our draw list
	drawList = g.create("Draw list (main geometry)", 
                        DrawListInfo(meshBuffer, 256, false));
	g.use(drawList);

	drawList0 = g.create("Draw list (post processing pass)", 
                         DrawListInfo(meshBuffer0, 1, false));
	g.use(drawList0);

    //Setup our post processing pass to draw a quad
	drawList0->draw(meshes[3], 1);
	drawList0->flush();

	//Setup geometry draw calls
	drawList->draw(meshes[2], 1);		//Reserve objects[0] for the sphere/water
	drawList->draw(meshes[4], 1);		//Reserve objects[1] for the planet
	drawList->flush();

	//Allocate our textures into a TextureList and send it to our shader
	textureList = g.create("Textures", TextureListInfo(2));
	shader->set("tex", textureList);

	//Allocate textures
	trock = g.create("rock", 
                     TextureInfo(textureList, "res/textures/rock_dif.png"));
	g.use(trock);

	twater = g.create("water", 
                      TextureInfo(textureList, "res/textures/water_dif.png"));
	g.use(twater);

	//Create the material list
	materialList = g.create("Materials", MaterialListInfo(textureList, 2));
	g.use(materialList);

	//Setup our materials
	rock = g.create("Rock material", MaterialInfo(materialList));
	rock->setDiffuse(trock);

	water = g.create("Water material", MaterialInfo(materialList));
	water->setDiffuse(twater);

	//Set our materials in our objects array
	objects[0].diffuse = water->getHandle();	//Water
	objects[1].diffuse = rock->getHandle();		//Planet

	//Set our shader samplers
	shader->set("samp", sampler);
	shader0->set("samp", sampler);

	//Set our view data to our view buffer
	shader->get<ShaderBuffer>("Views")->setBuffer(0, views->getBuffer());
    
    //Set our material data to material buffer
	shader->get<ShaderBuffer>("Materials")->
        setBuffer(materialList->getSize(), materialList->getBuffer());
    
    //Set our object data to our object buffer
	shader->get<ShaderBuffer>("Objects")->instantiate(totalObjects);

	//Setup lighting; 1 point, directional and spot light
	shader->get<ShaderBuffer>("PointLights")->instantiate(1);
	shader->get<ShaderBuffer>("SpotLights")->instantiate(1);
    ShaderBuffer *directionalLights = 
        shader->get<ShaderBuffer>("DirectionalLights")->instantiate(1);

    //Pass directional data (no point/spot lights)
	directionalLights->set("light/dir", Vec3(-1, 0, -1));
	directionalLights->set("light/intensity", 16.f);
	directionalLights->set("light/col", Vec3(1.f));

}
```

## Updating scene

```cpp
void MainInterface::update(f32 dt) {

	WindowInterface::update(dt); 
    
	//Force view buffer to update matrices of cameras, viewports and views
	views->update();
    
    //Rotate planet (& water)
	planetRotation += Vec3(30, 50) * dt;

	//Update rotation
	objects[0].m = Matrix::makeModel(Vec3(), Vec3(planetRotation, 0.f), Vec3(1.5f));
	objects[0].mvp = view->getStruct().vp * objects[0].m;

	objects[1].m = Matrix::makeModel(Vec3(), Vec3(planetRotation, 0.f), Vec3(3.f));
	objects[1].mvp = view->getStruct().vp * objects[1].m;

    //Update object buffer
	shader->get<ShaderBuffer>("Objects")->
        set(Buffer::construct((u8*)objects, sizeof(objects)));

	//Update per execution shader buffer
	ShaderBuffer *perExecution = shader->get<ShaderBuffer>("PerExecution");

	perExecution->set("ambient", Vec3(1));
	perExecution->set("time", (f32)getRuntime());
	perExecution->set("power", 1.f);
    perExecution->set("view", view->getHandle());

	//Setup post processing settings
	ShaderBuffer *postProcessing = 
        shader0->get<ShaderBuffer>("PostProcessingSettings");

	postProcessing->set("exposure", exposure);
	postProcessing->set("gamma", gamma);
```

## Shader code (GLSL)

### post_process

#### .vert

```cpp
#include <types.glsl>

layout(location = 0) out Vec2 uv;

layout(location = 0) in Vec2 inPos;

out gl_PerVertex {
    Vec4 gl_Position;
};

void main() {
    gl_Position = Vec4(inPos, 0, 1);
	uv = inPos * 0.5f + 0.5f;
}
```

#### .frag

```cpp
#include <types.glsl>

layout(location = 0) in Vec2 uv;

layout(location = 0) out Vec4 outColor;

layout(binding = 0) uniform sampler samp;
layout(binding = 1) uniform texture2D tex;

layout(binding = 2) uniform PostProcessingSettings {

	Vec2 padding;
	f32 exposure;
	f32 gamma;

} settings;

Vec4 sample2D(sampler s, texture2D t, Vec2 uv){
	return texture(sampler2D(t, s), uv);
}

void main() {
	
    //Sample color
	Vec3 col = sample2D(samp, tex, uv).rgb;
	
    //Exposure & gamma correction
    col = pow(Vec3(1) - exp(-col * settings.exposure), Vec3(1.0f / settings.gamma));
	
    //Write to back buffer
    outColor = Vec4(col, 1);
    
}
```

### simple

#### .vert

```cpp
#extension GL_ARB_shader_draw_parameters : require
#include <lighting.glsl>

struct PerObject {

	Matrix m;
	Matrix mvp;

	Vec3u padding;
	MaterialHandle material;

};

layout(std430, binding = 0) buffer Objects {

	PerObject arr[];
	
} obj;

layout(location = 0) out Vec3 pos;
layout(location = 1) out Vec2 uv;
layout(location = 2) out Vec3 normal;
layout(location = 3) flat out MaterialHandle material;

layout(location = 0) in Vec3 inPosition;
layout(location = 1) in Vec2 inUv;
layout(location = 2) in Vec3 inNormal;

out gl_PerVertex {
    Vec4 gl_Position;
};

void main() {

	PerObject obj = obj.arr[gl_InstanceIndex];

    gl_Position = obj.mvp * Vec4(inPosition, 1);

	pos = (obj.m * Vec4(inPosition, 1)).xyz;
	uv = inUv;
	normal = normalize(obj.m * Vec4(normalize(inNormal), 0)).xyz;
	material = obj.material;

}
```

#### .frag

```cpp
#include <lighting.glsl>

//Shader

layout(location = 0) in Vec3 pos;
layout(location = 1) in Vec2 uv;
layout(location = 2) in Vec3 normal;
layout(location = 3) in flat MaterialHandle material;

layout(location = 0) out Vec4 outColor;

layout(binding = 1) uniform PerExecution {

	Vec3 ambient;
	f32 time;

	Vec2u padding;
	f32 power;
	u32 view;
	
} exc;

layout(binding = 2) uniform Views_noalloc { ViewBuffer viewData; };

layout(binding = 3) uniform sampler samp;
layout(binding = 4) uniform texture2D tex[2];

layout(std430, binding = 5) buffer DirectionalLights {

	Light light[];

} dir;

layout(std430, binding = 6) buffer PointLights {

	Light light[];

} point;

layout(std430, binding = 7) buffer SpotLights {

	Light light[];

} spot;

layout(std430, binding = 8) buffer Materials {

	MaterialStruct mat[];

};

Vec4 sample2D(sampler s, TextureHandle handle, Vec2 uv){
	return texture(sampler2D(tex[handle], s), uv);
}

void main() {

	Camera cam = viewData.cameras[viewData.views[exc.view].camera];

    //Get camera position
	Vec3 cpos = normalize(cam.position - pos);

    //Calculate lighting result
	LightResult lr = { Vec3(0, 0, 0), 0, Vec3(0, 0, 0), 0 };
	
	for(int i = 0; i < dir.light.length(); i++){
		LightResult res = calculateDirectional(dir.light[i], pos, normal, cpos, exc.power);
		lr.diffuse += res.diffuse;
		lr.specular += res.specular;
	}
	
	for(int j = 0; j < point.light.length(); j++){
		LightResult res = calculatePoint(point.light[j], pos, normal, cpos, exc.power);
		lr.diffuse += res.diffuse;
		lr.specular += res.specular;
	}
	
	for(int k = 0; k < spot.light.length(); k++){
		LightResult res = calculateSpot(spot.light[k], pos, normal, cpos, exc.power);
		lr.diffuse += res.diffuse;
		lr.specular += res.specular;
	}
	
    //Get material
	MaterialStruct m = mat[material];
    
    //Get diffuse texture
	Vec3 dif = sample2D(samp, m.t_diffuse, uv).rgb;
    
    //Write lighting result to render target
    outColor = Vec4(calculateLighting(lr, dif, exc.ambient, m) * ((sin(exc.time) * 0.5 + 0.5) * 0.5 + 0.5), 1);

}
```

# Extensions and features

"Extensions" are API dependent structs that can contain functions for the API dependent details. This is an extension of the normal graphics object and allows the API implementation to store extra data.

"Features" are hardware dependent features that don't have to be available at the moment. An example of this could be raytracing, if the feature isn't enabled, the objects can't be created. You can check the feature through Graphics by using `Graphics::checkFeature(GraphicsFeatures::Raytracing)` which returns a GraphicsFeatureSupport enum; 'None' when it's not supported (like non-NV devices), 'CPU_emulated' when the GPU can't handle it, 'GPU_emulated' when the GPU can handle it (but not natively) or 'GPU_accelerated' when the GPU can handle it. An example: Raytracing can return 'None' on non NVIDIA devices, CPU_emulated on NV devices (below shader level 6.0), GPU_emulated on NV devices (non RTX) and GPU_accelerated on RTX devices.

The following list contains the types for Vulkan and OpenGL implementation; though the OpenGL implementation is just an example (and doesn't exist). The raytracing and VR features are also just a prototype and don't have an implementation yet. The type can be either class, GO (graphics object) or OE (Osomi Enum). OE doesn't directly map to the types given, but rather through an intermediate enum, just like the classes and GO.

| Base                  | Vulkan                                                       | OpenGL | Type  |
| --------------------- | ------------------------------------------------------------ | ------ | ----- |
| Graphics              | VkInstance<br />VkPhysicalDevice<br />VkDevice<br />VkSurfaceKHR<br />VkQueue<br />VkSwapchainKHR<br />VkCommandPool<br />VkPhysicalDeviceFeatures<br />VkPhysicalDeviceMemoryProperties<br />VkColorSpaceKHR<br />VkFormat<br />VkFence[] present<br />VkSemaphore[] submit<br />VkSemaphore[] swapchain<br />CommandList*<br />`VkGBuffer[][]` staging<br />u32 current<br />u32 frames<br />u32 queueFamily<br />VkDebugReportCallbackEXT (debug)<br />PFN_vkSetDebugUtilsObjectNameEXT (debug Windows) | GLnull | class |
| RenderTarget          | VkRenderPass<br />VkFramebuffer[]                            | GLuint | GO    |
| Texture               | VkImage<br />VkDeviceMemory<br />VkImageView                 | GLuint | GO    |
| GBuffer               | VkBuffer[]<br />VkDeviceMemory<br />u32                      | GLuint | GO    |
| ShaderStage           | VkShaderModule<br />VkPipelineShaderStageCreateInfo          | GLuint | GO    |
| Shader                | VkShaderStage*[]<br />VkPipelineLayout<br />VkDescriptorSetLayout<br />VkDescriptorPool<br />VkDescriptorSet[] | GLuint | GO    |
| CommandList           | VkCommandPool<br />VkCommandBuffer[]                         | GLnull | GO    |
| PipelineState         | VkPipelineInputAssemblyStateCreateInfo<br />VkPipelineRasterizationStateCreateInfo<br />VkPipelineColorBlendAttachmentState<br />VkPipelineColorBlendStateCreateInfo<br />VkPipelineDepthStencilStateCreateInfo<br />VkPipelineMultisampleStateCreateInfo | GLnull | GO    |
| TextureFormat         | VkFormat                                                     | GLenum | OE    |
| TextureUsage          | VkImageLayout                                                | GLnull | OE    |
| ShaderStageType       | VkShaderStageFlagBits                                        | GLenum | OE    |
| TopologyMode          | VkPrimitiveTopology                                          | GLenum | OE    |
| FillMode              | VkPolygonMode                                                | GLenum | OE    |
| CullMode              | VkCullModeFlags                                              | GLenum | OE    |
| WindMode              | VkFrontFace                                                  | GLenum | OE    |
| GBufferType           | VkGBufferType                                                | GLenum | OE    |
| ShaderRegisterType    | VkDescriptorType                                             | GLnull | OE    |
| ShaderRegisterAccess  | VkShaderStageFlags                                           | GLnull | OE    |
| SamplerWrapping       | VkSamplerAddressMode                                         | GLenum | OE    |
| SamplerMin            | VkFilter<br />VkSamplerMipmapMode                            | GLenum | OE    |
| SamplerMag            | VkFilter                                                     | GLenum | OE    |
| RayAccelerationFt     | VkAccelerationStructureNV                                    | -      | GO    |
| RayShaderStageTypeFt  | see ShaderStageType                                          | -      | OE    |
| RayGBufferTypeFt      | see GBufferType                                              | -      | OE    |
| MeshShaderStageTypeFt | see ShaderStageType                                          | -      | OE    |
| XRViewportFt          | ???                                                          | -      | GO    |
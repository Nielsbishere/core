# Osomi Graphics Shading Language

ogsl is a API independent shading language. It can be very easily translated into either HLSL, GLSL or future languages; and it is done by simple pre-processing. Types and some functions are API-dependent, meaning that the code you'll write won't compile when you want to use DirectX or Vulkan by flipping a switch. oiSH already fixes these problems, by letting you compile from HLSL and GLSL to SPV; even allowing you to use your own made up languages for it (as long as it is reflectable SPV). However, that still means that you can't include code written for GLSL in an HLSL file. ogsl fixes this and makes sure that if you include a .ogsl file in .glsl or .hlsl you can still compile. 

ogsl makes shader transpiling a lot easier, you only have to have a compiler that does pre-processing and then you make sure it always includes the language-dependent types file, which includes the defines and macros for the ogsl types.

The reason ogsl is structured like it is, is to ensure the C++ code base and the ogsl code base use the same coding standard and with a few macros a shader file could (in theory) be compiled to C++. 

Since there are differences between OpenGL, Vulkan, HLSL and other graphics APIs, this language can provide a simple helper to handle interfacing between all of them. For example; in OpenGL the concept of samplers don't properly exist and bindings don't either, so you can provide a custom types.glsl for OpenGL to handle that.

## Types

| Type     | GLSL         | HLSL | C++ (native) |
| -------- | ------------ | ---- | ------------ |
| f32/f64  | float/double | <-   | <-           |
| i32      | int          | <-   | int32_t      |
| u32      | uint         | <-   | uint32_t     |
| VecnZ    | zvecn        | Zn   | Z[n]         |
| MatrixnZ | matn         | Znxn | Z[n]         |

Where z = u for uint, i for int, d for double, float by default. Z = uint, float, double or int.

## Functions

| Name                                  | GLSL                                  | HLSL                | C++ (native)              |
| ------------------------------------- | ------------------------------------- | ------------------- | ------------------------- |
| lerp(a, b, perc)                      | mix                                   | lerp                | lerp                      |
| sample2D(Sampler, Texture2D, Vec2 uv) | texture(sampler2D(t, s), uv)          | s.Sample(t, uv)     | s->load(t, uv)            |
| write2D(img, coords, value)           | imageStore(img, Vec2i(coords), value) | img[coords] = value | img->store(coords, value) |

sample2D also has sampler2Du and sampler2Di for uint and int textures.

## Global constants

| Name         | GLSL                  | HLSL | C++ (native) |
| ------------ | --------------------- | ---- | ------------ |
| instanceId   | gl_InstanceIndex      | ???  | instanceId   |
| invocationId | gl_GlobalInvocationID | ???  | invocationId |
| vPosition    | gl_Position           | ???  | vPosition    |



## TODO: Lighting.ogsl

This file contains generic ogc structs, defines and functions; these are so common that they have been integrated into the syntax highlighter, so it is more obvious when you call an expensive function (since most lighting functions are very heavy).

These also contain type definitions:

```cpp
#define TextureHandle u32
#define MaterialHandle u32
#define LightHandle u32
#define CameraHandle u32
#define CameraFrustumHandle u32
#define ViewHandle u32
```

Every handle is just a uint into a buffer of some sorts, but you have to maintain those yourself (ogc does a lot for you though).

## Shader data

Shaders all have data they transfer or read to/write from. In ogc this is implemented as "ShaderData", the shader registers; textures, samplers, arrays and buffers. The inputs can only be read from and the outputs can only be written to, and these are exposed through the shader info as well.

One of the problems with mapping glsl & hlsl languages together is that things like uniform buffers and shader storage buffers don't exist in hlsl. However, there are a few similarities; whenever a ssb is used, you probably want to read or modify a lot of data. Whenever a ub is used, you can't. This means a 'Buffer' would map to a writeable buffer and 'Uniform' would map to a cbuffer in hlsl. glsl and hlsl both have a way of having a huge array of data, in hlsl this is a structured buffer and in glsl it is a 'buffer' with a size of 0; aka dynamic. We resolved this by adding 'Array' which maps to a dynamic buffer or a structured buffer. This means that the glsl way of having the possiblity of having data before your array is not available anymore.

| Syntax                                      | GLSL                                                         | HLSL                                                         | C++                                           |
| ------------------------------------------- | ------------------------------------------------------------ | ------------------------------------------------------------ | --------------------------------------------- |
| In(id, name, type)                          | layout(location=id) in type name                             | ???                                                          | type name                                     |
| ConstIn(id, name, type)                     | layout(location=id) in flat type name                        | ???                                                          | type name                                     |
| Out(id, name, type)                         | layout(location=id) out type name                            | ???                                                          | type name                                     |
| Uniform(id, name, typeName, definition)     | layout(binding=id) uniform typeName definition name          | cbuffer name definition                                      | typedef struct typeName definition const name |
| ConstBuffer(id, name, typeName, definition) | layout(binding=id,std430) buffer readonly typeName definition name | `struct typeName definition; Buffer<typeName> name : register(tid);` | typedef struct typeName definition const name |
| Buffer(id, name, typeName, definition)      | layout(binding=id,std430) buffer typeName definition name    | `struct typeName definition; RWBuffer<typeName> name : register(uid);` | typedef struct typeName definition name       |
| ConstArray(id, name, type)                  | layout(binding=id,std430) buffer readonly { type name[]; }   | `StructuredBuffer<type> name : register(tid)`                | `const std::vector<type> name`                |
| Array(id, name, type)                       | layout(binding=id,std430) buffer { type name[]; }            | `RWStructuredBuffer<type> name : register(tid)`              | `std::vector<type> name`                      |
| Sampler(id, name)                           | layout(binding=id) uniform sampler name                      | SamplerState name : register(sid)                            | Sampler *name                                 |
| Texture2Dx(id, name)                        | layout(binding=id) uniform texture2D name                    | Texture2D name : register(tid)                               | Texture *name                                 |
| Image2Dx(id, name, format)                  | layout(binding=id,format) uniform image2D name               | `RWTexture2D<format> name : register(uid);`                  | Texture *name                                 |

"ConstIn" is used for data from the vertex buffer that doesn't have to be interpolated; like a material id (u32).

The x in Texture2Dx or Image2Dx can be nothing for a floating point texture, i for int, u for uint.

## Shader stage

A shader stage is noted by "ShaderStage() { code; }", this contains internal code that sets up the code for that stage. For example, a compute shader has multiple parameters that tell the group size.

```cpp
Compute(16, 16, 1) {
    //Do compute stuff
}
```

A shader module cannot have stages that have the same type; you can't have 2 compute stages in 1 shader.

| Syntax           | GLSL                                                         | HLSL | C++                                             |
| ---------------- | ------------------------------------------------------------ | ---- | ----------------------------------------------- |
| Vertex()         | out gl_PerVertex { Vec4 gl_Position; }; void main()          | ???  | extern "C" __declspec(dllexport) void vertex()  |
| Fragment()       | layout(early_fragment_tests) in; void main()                 | ???  | ^                                               |
| Compute(x, y, z) | layout(local_size_x = x, local_size_y = y, local_size_z = z) in; void main() | ???  | extern "C" __declspec(dllexport) void compute() |

Tesselation, TesselationEvaluation, AnyHit, ClosestHit, Miss, Call and Raygen haven't been properly used, but are defined as void main().

## API/Language specific code

Sometimes you want to have API specific code. This can be done by checking the `_GLSL`,  `_HLSL` and `_CPP` defines, as well as checking the `_VULKAN`, `_OPENGL`, `_DIRECTX` defines. Be sure to add an implementation that fits all APIs, so your shader works everywhere.

```cpp
void mySpecialFunction(){
    
    #ifdef _GLSL
		//Do GLSL
    #elif _HLSL
    	//Do HLSL
    #else
    	//Do C++ (_CPP)
    #endif
    
}

void mySpecialAPIFunction(){
    
    #ifdef _VULKAN
    	//Do Vulkan
    #elif _OPENGL
    	//Do OpenGL
    #else
    	//Do DirectX (_DIRECTX)
	#endif
    
}
```

However, please prevent this and only use it when you have no other choice. Rewriting shaders multiple times is a pain and is one of the reasons ogsl was created in the first place.

## Example

### Vertex

```cpp
In(0, inPos, Vec2);
Out(0, uv, Vec2);

Vertex() {
    vPosition = Vec4(inPos, 0, 1);
	uv = inPos * 0.5f + 0.5f;
}
```

vPosition is the equivalent of "gl_Position"  and is therefore a reserved keyword.

### Fragment

```cpp
In(0, uv, Vec2);
Out(0, outColor, Vec4);

Sampler(0, samp);
Texture2D(1, tex);
Uniform(2, settings, PostProcessingSettings, 
        { Vec2 padding; f32 exposure; f32 gamma; });

Fragment() {
	
    //Sample color
	Vec3 col = sample2D(samp, tex, uv).rgb;
	
    //Exposure & gamma correction
    col = pow(Vec3(1) - exp(-col * settings.exposure), Vec3(1.0f / settings.gamma));
	
    //Write to back buffer
    outColor = Vec4(col, 1);

}
```

### Compute shader

```cpp
Compute(16, 16, 1) {
    
    //... compute functionality
    
}
```

## Syntax highlighting

In the "tools" directory, there is a notepad++ highlighter for the functions, variables and other syntax that ogsl provides.
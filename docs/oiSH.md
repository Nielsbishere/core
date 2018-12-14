# Osomi SHader (.oiSH)
This file format exists to wrap around shaders, giving the end-user more info than just shader code or bytecode. It includes the ability to reflect on all buffers and resources that are used. This allows you to create all shader resources automatically, as well as adding a visual interface for your shaders.
# File Specification
## Header
```cpp
struct SHHeader {

	char header[4];		//oiSH

	u8 version;			//SHVersion
	u16 type;			//SHStageTypeFlag
	u8 shaders;

	u8 buffers;
	u8 registers;
	u16 codeSize;

	u16 groupX;
	u16 groupY;

	u16 groupZ;
	u16 p1 = 0;

};
```
'version' is currently v0_1, but it could change when newer shaders or shader concepts are released.  
'type' is which shader stages are included; See SHStageTypeFlag for more. 
'shaders' the number of shaders included.  
'buffers' how many buffers exist.  
'registers' how many registers (buffers, textures & samplers) exist.  
'codeSize' the size of the code block (all stages bytecode combined).
'groupX', 'groupY' and 'groupZ' are the local_size constants for a compute shader.

### Version

```cpp
enum class SHVersion : u8 {
	Undefined = 0,
	v0_1 = 1
};
```

### Shader stage types

```cpp
enum class SHStageTypeFlag : u16 {

	//Compute

	COMPUTE = 0x0000,

	//Graphics

	VERTEX = 0x0001,
	FRAGMENT = 0x0002,
	GEOMETRY = 0x0004,
	TESSELATION = 0x0008,
	TESSELATION_EVALUATION = 0x0010,

	//0x0020
	//0x0040
	//0x0080

	//Extended graphics pipeline

	MESH = 0x0100,
	TASK = 0x0200,
	
	//Raytracing

	RAY_GEN = 0x0400,
	ANY_HIT = 0x0800,
	CLOSEST_HIT = 0x1000,
	MISS = 0x2000,
	INTERSECTION = 0x4000,
	CALLABLE = 0x8000

};
```

The types beyond geometry shaders can't be compiled yet and are reserved for future implementation; if shader compilation to those types is created, the ogc implementation will follow.

## Stage

Per shader stage, the following struct will be inserted.
```cpp
struct SHStage {

	u8 flags;
	u8 type;				//ShaderStageType: ceil(log2(SHStageTypeFlag + 1))
	u16 nameIndex;

	u16 codeIndex;
	u16 codeLength;

	u8 inputs;
	u8 outputs;
	u16 padding = 0;
    
};
```
'flags' is reserved for future use.  
'type' is the shader stage type that represents this (See section shader stage types).
'nameIndex' is where the name is located in the oiSL file.  
'codeIndex' is the offset in the code block.  
'codeLength' is the length in the code block.  
'inputs' is the number of shader inputs.  
'outputs' is the number of shader outputs.  
For each stage, there's a `std::vector<SHInput>` and `std::vector<SHOutput>`. These are the in/out variables per stage.

### Input vars
```cpp
struct SHInput {
	u8 type;			//TextureFormat
	u8 padding = 0;
	u16 nameIndex;
}
```
Every input var is 4 bytes; an index to the name and the TextureFormat of the attribute.
### Output vars
```cpp
struct SHOutput {
	u8 type;			//TextureFormat
	u8 id;
	u16 nameIndex;
};
```
Just like the inputs, the output struct is 4 bytes. However, it has an id of which offset the output has.
### Register
A register is like a uniform; it stays the same for the execution. This can be a sampler, texture, buffer, etc.
```cpp
struct SHRegister {

	u8 type;				//ShaderRegisterType
	u8 id;					//Register id
	u16 representation;		//Represents which buffer to use

	u16 nameIndex;
	u16 size;				//If the type is an array (array of textures for example)

	u16 access;				//ShaderStageType
	u16 padding = 0;
};
```
'type' the ShaderRegisterType (Undefined (0), UBO (1), SSBO(2), Texture(3), Image(4), Sampler(5)).   
'id' is the register id.  
'representation' is the index of the embedded resource (+ 1). With buffer, this is the id of the buffer info it should use. However, this might be used with immutable samplers in the future.  
'nameIndex' is where the name is stored in the oiSL file.  
'size' is the size of the array (for example, texture array). But is mostly always 1.

### Layout
The SHFile is laid out like following:
```cpp
SHHeader header;
std::vector<SHStage> stages;
std::vector<std::pair<std::vector<SHInput>, std::vector<SHOutput>>> stageInputsOutputs;
std::vector<SHRegister> registers;
SLFile strings;
std::vector<SBFile> shaderBuffers;
Buffer bytecode;
```
# API usage
## Writing a shader file
Just like in the oiSB format, you can directly convert a ShaderInfo struct into a SHFile:
```cpp
ShaderInfo info = ...;
SHFile file = oiSH::convert(info);
if(!oiSH::write(myOutputFile, file))
  ; //Handle error
```
As seen above, it is very similar to how the other file formats are handled.
## Reading a shader file
```cpp
SHFile file;
if(!oiSH::read(myInputFile, file))
  ; //Handle error
ShaderInfo info = oiSH::convert(file);
```
Now you can create a Shader object with it and use it.
## Compiling a shader file
For runtime compilation, you can use the compile and convert functions. These take in the ShaderSource. ShaderSource can be created like following:
```cpp
ShaderSource simpleShader = ShaderSource("simple", 
    { "res/shaders/simple.frag", "res/shaders/simple.vert" }, ShaderSourceType::GLSL);  //Create from files (SPIRV/GLSL/HLSL)
ShaderSource embeddedShader = ShaderSource("embedded", ShaderSourceType::GLSL, 
    { { "vert", myVertCode }, { "frag", myFragCode } });        //Create from source (no relative includes)
ShaderSource spvShader = ShaderSource("spv", 
    { { "vert", vertSpv }, { "frag", fragSpv } });              //Create from spirv (has to be a CopyBuffer)
```
This can then be sent to 'convert' to get a SHFile or 'compile' to get a ShaderInfo struct.
## Using the standalone tool
The oibaker tool is made for allowing you to convert hlsl and glsl to oiSH automatically. Just put your shaders into a res/shaders directory and run the exe from the app folder root, this will convert all shader files into a .oiSH file.

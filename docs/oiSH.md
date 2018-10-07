# Osomi SHader (.oiSH)
This file format exists to wrap around shaders, giving the end-user more info than just shader code or bytecode. It includes the ability to reflect on all buffers and resources that are used. This allows you to create all shader resources automatically, as well as adding a visual interface for your shaders.
# File Specification
## Header
```cpp
struct SHHeader {

	char header[4];   //oiSH

	u8 version;       //SHHeaderVersion_s
	u8 type;          //SHStageTypeFlag
	u8 shaders;
	u8 padding = 0;

	u8 buffers;
	u8 registers;
	u16 codeSize;

};
```
'version' is currently v0_1 (1), but it could change when newer shaders or shader concepts are released.  
'type' is which shader stages are included; 0 (COMPUTE), 1 (VERTEX), 2 (FRAGMENT), 3 (GEOMETRY).  
'shaders' the number of shaders included.  
'buffers' how many buffers exist.  
'registers' how many registers (buffers, textures & samplers) exist.  
'codeSize' the size of the code block (all stages bytecode combined).
## Stage
Per shader stage, the following struct will be inserted.
```cpp
struct SHStage {

	u8 flags;
	u8 type;			//ShaderStageType
	u16 nameIndex;

	u16 codeIndex;
	u16 codeLength;

	u8 inputs;
	u8 outputs;
	u16 padding = 0;
      
};
```
'flags' is reserved for future use.  
'type' is the shader stage type that represents this (Undefined = 0, Vertex = 1, Fragment = 2, Geometry = 3, Compute = 4).
'nameIndex' is where the name is located in the oiSL file.  
'codeIndex' is the offset in the code block.  
'codeLength' is the length in the code block.  
'inputs' is the number of shader inputs.  
'outputs' is the number of shader outputs.  
For each stage, there's a `std::vector<SHInput>` and `std::vector<SHOutput>`. These are the in/out variables per stage.
### Input vars
```cpp
struct SHInput {
	u16 nameIndex;
	u8 padding = 0;
	u8 type;			//TextureFormat
};
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

	u8 typeAccess;        //ShaderRegisterType << 4 | SHRegisterAccess
	u8 id;                //Register id
	u16 representation;   //If type is buffer; represents which buffer to use.

	u16 nameIndex;
	u16 size;             //If the type is an array (array of textures for example)
      
};
```
'typeAccess' uses the upper 4 bits for the ShaderRegisterType (Undefined (0), UBO (1), SSBO(2), Texture(3), Image(4), Sampler(5)). And the lower 4 bits for the ShaderRegisterAccess Compute (1), Vertex (2), Geometry (4), Fragment (8).  
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
The oish_gen tool is made for allowing you to convert hlsl and glsl to oiSH automatically. Just put your shaders into a res/shaders directory and run the exe from the root, this will convert all shader files into a oiSH file.

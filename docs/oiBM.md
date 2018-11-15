# .oiBM (BakeManager)

Is a file format created for converting external formats to internal formats `(.fbx, .obj, .glsl, .hlsl) -> (.oiRM, .oiSH)`. This format itself only keeps references to inputs, outputs and dependencies for every file. Example:

```
res/models/test.fbx -> res/models/test.0.oiRM, res/models/test.1.oiRM
```

When an fbx contains more than 1 model, it will generate multiple output files. The BakeManager prevents this, by storing the information about each conversion.

## Format

```cpp
struct BMHeader {

	char header[4];		//oiBM

	u16 files;
	u16 flags;
    
    u32 version;

};

struct BMBakedFile {
	u8 inputs;
	u8 outputs;
	u16 dependencies;
};

struct BakerFile {

	BMHeader header;

	std::vector<BakedFile> files;

	SLFile strings;

};
```

The SLFile stores all paths as string (only using the necessary bit-depth) and these are referenced to by the BakedFile's array.

```cpp
//oiBM data
6F 69 42 4D 0B 00 00 00 00 00 00 00 01 01 00 00 01 01 00 00 01 01 00 00 01 02 00 00 01 02 00 00 01 01 00 00 01 01 00 00 01 01 00 00 01 01 00 00 02 01 01 00 02 01 02 00 00 00 01 00 02 00 03 00 04 00 05 00 06 00 07 00 08 00 09 00 0A 00 0B 00 0C 00 0D 00 0E 00 0F 00 10 00 11 00 12 00 13 00 14 00 15 00 16 00 17 00 18 00 19 00 1A 00 1B 00 17 00
//oiSL data
...
```

Consider the data above; the first 8 bytes is the header:

```cpp
6F 69 42 4D, 0B 00, 00 00, 00 00 00 00
{ "oiBM", 11, BMFlag::None, 0 /* version */ }
```

For every file, we have a BMBakedFile:

```cpp
01, 01, 00, 00 		//1 input & output
01, 01, 00, 00 
01, 01, 00, 00 
01, 02, 00, 00 		//1 input & 2 outputs
01, 02, 00, 00 
01, 01, 00, 00 
01, 01, 00, 00 
01, 01, 00, 00 
01, 01, 00, 00
02, 01, 01, 00 		//2 inputs, 1 output, 1 dependency
02, 01, 02, 00		//2 inputs, 1 output, 2 dependencies
```

Afterwards, we have the u16[]; the name list. Every input, output and dependency has a name stored in the SLFile.

```cpp
00 00, 01 00 							//names[0] as input, names[1] as output
02 00, 03 00 							//^ 2, 3
04 00, 05 00 							//^ 4, 5
06 00, 07 00, 08 00 					//input: 6, output: 7,8
09 00, 0A 00, 0B 00 					//9, [ 10, 11 ]
0C 00, 0D 00 							//12, 13
0E 00, 0F 00 							//14, 15
10 00, 11 00 							//16, 17
12 00, 13 00 							//18, 19
14 00, 15 00, 16 00, 17 00 				//[20, 21 ], 22, 23 (as dependency)
18 00, 19 00, 1A 00, 1B 00, 17 00		//[24, 25 ], 26, [ 27, 23 ]
```

As you can see above, most dependencies and inputs/outputs are different. Except the last two (which are shaders), they both use 23 (0x17), which is "res/shaders/types.glsl". Below, you can see the unpacked string list:

```cpp
mod/models/anvil.fbx
mod/models/anvil.oiRM
mod/models/sphere.fbx
mod/models/sphere.oiRM
mod/models/cube.fbx
mod/models/cube.oiRM
mod/models/CargoBox.fbx
mod/models/CargoBox.0.oiRM
mod/models/CargoBox.1.oiRM
mod/models/SM_Charging_Platform.fbx
mod/models/SM_Charging_Platform.0.oiRM
mod/models/SM_Charging_Platform.1.oiRM
mod/models/SM_Rock_1.fbx
mod/models/SM_Rock_1.oiRM
mod/models/SM_Cilinder_Cables.fbx
mod/models/SM_Cilinder_Cables.oiRM
mod/models/sword.fbx
mod/models/sword.oiRM
mod/models/SM_Rock_2.fbx
mod/models/SM_Rock_2.oiRM
mod/shaders/post_process.frag
mod/shaders/post_process.vert
mod/shaders/post_process.oiSH
res/shaders/types.glsl
mod/shaders/simple.frag
mod/shaders/simple.vert
mod/shaders/simple.oiSH
res/shaders/lighting.glsl
```
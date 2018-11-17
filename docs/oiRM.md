# Osomi Raw Model (.oiRM)
The file format was created to support the choice between low load times and small file sizes, as well as being more game-based, allowing the specification of any texture format. Unlike fbx, obj or other formats, you can choose the texture format you need for every variable, as well as allowing to use multiple vbos to support that decision. This format would correspond to a VBO/IBO and VAO in OpenGL, which are very easily creatable from plain oiRM. This type is also a binary format, making it harder to modify and keeping your assets safe(r). Other than that, oiRM stores the topology and fill mode of the model, allowing you to specify that the model should only be rendered the way you tell it to. However; setting these to undefined will allow them to be used as anything. Last but not least, oiRM supports miscs, which is a primitive or a group of primitives with a special meaning, this could also have an offset or just be a point in model space. Miscs could be used for gameplay features, such as anchor points, spawn points and more.  
The compression flag turns oiRM from high file size and very fast / almost immediate load in into a very compact, yet still very fast file format. This version with compression is shortened as oiRMc.
# File specification
## Header
```cpp
  struct RMHeader {

  char header[4];   //oiRM

  u8 version;       //RMHeaderVersion
  u8 flags;         //RMHeaderFlag1
  u8 vertexBuffers;
  u8 vertexAttributes;

  u8 topologyMode;  //TopologyMode
  u8 fillMode;      //FillMode
  u8 miscs;
  u8 p0;

  u32 indexOperations;

  u32 vertices;

  u32 indices;      //Optional (=0 if no indices)

};
```
'version' is the oiRM version; v_0_0_1 is the first version, but other versions will probably pop up, to support other features in the future.  
'flags' include the following; Contains_materials (1) if there's materials embedded with the model. Per_tri_materials (2) if there's multiple materials per triangle, this will be `u8[triangles]` that specifies which triangle has which material. Uses_compression (4) if you prefer small file size over fast load times.  
'vertexBuffers' is the number of 'RMVBO' structs that are included, they represent a physical buffer.  
'vertexAttributes' is the number of 'RMAttribute' structs that are included, they represent a variable in a buffer.  
'topologyMode' is defined in ogc.md; important ones are triangle, points and lines.  
'fillMode' is how the polygons are processed; most commonly fill (see ogc.md).  
'miscs' how many miscs are in this file.  
'indexOperations' is how many index operations are in the file; this is the optimization of indices and is only available in triangle mode. Only available in compression mode.
'vertices' how many vertices are in this file.  
'indices' how many indices are in this file (0 if there's no indices present).

## RMVBO
```cpp
struct RMVBO {
  u16 stride;
  u16 layouts;
};
```
'stride' is how many bytes each vertex takes up. A maximum is 65 535 bytes, which is quite a bit for a vertex.  
'layouts' is how many layouts are used in this vbo.
## RMAttribute
```cpp
struct RMAttribute {
  u8 padding;
  u8 format;			//TextureFormat
  u16 name;
};
```
'name' represents the name that's stored in the oiSL file.  
'format' is the attribute's format.  
'padding' is used to pad the attribute to 4 bytes, can also be used in the future.
## RMMisc
```cpp
struct RMMisc {

  u8 flags;			//RMMiscFlags
  u8 type;			//RMMiscType
  u16 size;			//Data size
			
};
```
'flags' can be 'Uses_offset' (1) if there is an offset to the primitives, 'Is_array' (2) if there are multiple primitives present, Offset_in_relative_space (4) if the offset uses relative space (1 vertex; tangent space, multiple; combined tangent space).  
'type' can be Vertex (0), Primitive (1), Center_primitive (2) the center of a primitive, Point (3) a point in model space.  
'size' the data attached to the misc.

Miscs aren't currently queryable because there currently is no way of adding them into a model. However, this could potentially be done with Fbx's properties of a Mesh; which would mean that Obj and non-property formats wouldn't support this.

## VBOs
After this data, all VBO data is stored in binary (but might have to be decompressed). One vertex's buffer is stored as following:
```
[ 0, Graphics::getFormatSize(format0) ]
[ Graphics::getFormatSize(format0), Graphics::getFormatSize(format1)]
etc.
```
For example;
```
Vec3 position, normal(0, 1, 0), tangent(1, 0, 0);
Vec2 uv;
VBO0 v0(position, uv, normal);
VBO1 v1(tangent);
```
A vertex with buffers (position, uv, normal) and (tangent) would be stored as following:
```
First buffer

0 0 0	(pos = 12 bytes; 3 floats)
0 0	(uv = 8 bytes; 2 floats)
0 1 0	(normal = 12 bytes; 3 floats)

Second buffer

1 0 0	(tangent = 12 bytes; 3 floats)

```
The second buffer would be appended to the first buffer, giving us a float array `[0,0,0, 0,0, 0,1,0, 1,0,0]`, which is represented as a buffer. This means that the (relative) offset of the second buffer can be calculated by using RMVBO::stride * vertices and the index buffer (in this case; if there is no other buffer and if indices is not 0) is located at offset buf0.stride * vertices + buf1.stride * vertices.
### Compression
Vertex compression is a more complex concept than index compression; this is because a TextureFormat has the concept of channels (RGB for example). Every channel uses the same data type; allowing the optimization of sharing a value. For example; you have a texture with just red, blue, green and white; these values (R G or B) are either 0 or 1, allowing you to represent the texture as a bitset. This can be done by utilizing a keyset with the data types and storing the actual data in a bitset that references the keyset. This can also be used by having a list of all colors made up of that keyset; meaning that if we had red, blue, green, white and black, we'd have 4 options for every pixel (2 bits instead of 4 bytes). Then, we'd have a keyset of 2 ubytes (2 bytes + 4 bytes uint), 4 colors (3 bits per color = 12 bits (padded = 2 bytes) + 4 bytes uint). This will result into a 64x64 image with those 4 colors to go from 16384 bytes to 1036 bytes (6% of orignal bytes).  
This way of compression can significantly decrease required storage, but of course has to be decompressed. The data is compressed as follows:
```cpp
u32 keys;								//Different data values (stored in binary)
u8 key[keys * Graphics::getChannelSize(format)];			//All values (stored in binary)
u32 keyBits = std::ceil(std::log2(keys));				//Bits required to point to a key (not stored, but calculated)
u32 values;								//How many values that channels can have (stored in binary)
Bitset value(values * channels * keyBits);				//All values that channels can have (stored in binary)
u32 valueBits = std::ceil(std::log2(value));				//Bits required to point to a value (not stored, but calculated)
Bitset data(vertices * valueBits];					//All references to keys (stored in binary)
```
For decompression; the vertex data's order has to be changed, so all attributes are placed next to each other for every vertex.  
In a short diagram:  
![VBO Attributes compression](vbo%20compression.png)

## IBO
The index buffer is located after the vertex buffers and the size is equal to `indices * 4` unless compressed, aka a `u32[indices]`. `u8[]` and `u16[]` aren't used in non-compressed IBOs, because on the GPU it is stored as a `u32[]`, this is because modern models generally require around 65 536+ vertices and because the concept of MeshBuffers requires the index type to be compatible (e.g. u8 indices buffer can't be stored in a u32 indices buffer). It also has to do with the idea that a fetch operation can be optimized better with a uint than a ubyte by the GPU.
### Compression
For compression, the following formula is followed to determine the size of a pointer (in bits):
```cpp
perIndexb = (u32) std::ceil(std::log2(vertices))
```
This means that if you have 24 vertices (ex. a cube), you can use ceil(log2(24)) = ceil(4.58) = 5 bits per index. Resulting in a total index buffer of 23 bytes (36 indices). This compresses a lot (especially when using 65536+ vertices, because mostly they won't end up using 32 bits, but 17 or 18).

However, the IBO compression only uses that compression per index if there are no 'special operations'; which reduces triangles from 3 indices per tri to 1 index per tri. These special operations are the following:

```cpp
NoOp (3 indices; [i, j, k])
Quad ([n + 2, n + 1, n], [n + 3, n + 2, n])
RevIndInc ([n + 2, n + 1, n])
RevIndInc2 ([n + 3, n + 2, n])
```

These 4 operations are stored in a bitset with length `2 * header.indexOperations`. Every indexOperation has 2 bits that specify what the operation does. Quad generation saves 6 indices and only requires the base index n (1 index + 2 bits). 

For example our cube has 6 sides (quad); meaning we can store the indices in 12 bits (2 bytes) and 30 bits (4 bytes); resulting in 6 bytes instead of 23. Other shapes like spheres (and even anvils) mostly use Quad + RevIndInc, meaning that they will be highly optimized too.

```cpp
for(u32 i = 0; i < indexOperations; ++i)
    if(ops[i * 2] || ops[i * 2 + 1])
        ; 	//Read 1 index into either a quad, reverse index increase or inc2
	else
        ;	//Read 3 indices into a normal triangle
```

More about this compression can be found in the [model compression document](model compression.md). It is only applied to triangle meshes.

## MiscBuffer
The MiscBuffer is the buffer for every misc. Every misc can have up to 65535 bytes of data; right now this data doesn't have a use, or definition yet; however this feature will be further developed in the future.
## Names
Like all other Osomi file formats, the names are stored in a oiSL file that is embedded at the end of the object. 
# API Usage
## Reading
To read and use a oiRM file, you'd use the following code:
```cpp
RMFile file;
if(!oiRM::read("res/models/myModel.oiRM", file))
	; //Handle error
std::pair<MeshBufferInfo, MeshInfo> info = oiRM::convert(file);
```
If the file is valid, it reads the MeshInfo and MeshBufferInfo from this oiRM file. The MeshInfo tells how many vertices and indices the file has and the decoded VBOs/IBO. The VBOs and IBO have to be deconstructed, if they aren't sent to a Mesh which takes care of that.  
The MeshBufferInfo specifies what buffer the mesh wants to be allocated into; meaning the topology mode, fill mode and layout have to match (if they are defined). This should be checked against by the user; when selecting a proper MeshBuffer.
## Writing
To write a oiRM file, you have to convert a MeshInfo:
```cpp
RMFile converted = oiRM::convert(myModel->getInfo());
if(!write(converted, "out/models/myModel.oiRM"))
	; //Handle error
```
This means you have to have a VALID MODEL ALLOCATED which already has a MeshBuffer.
## Generating
If you don't want to load a model first, you can output the data directly into a oiRM file, using the following method:
```cpp
Buffer myoiRM = oiRM::generate(myVbo, myIbo, hasPos, hasUv, hasNrm, vertices, indices, useCompression);
```
Our VBO and IBO are just 1 buffer, since the generate function was created specifically for converting simple models (vec3 pos, vec2 uv, vec3 normal) to our own. the hasPos, hasUv and hasNrm bools specify if those attributes exist (they REQUIRE the formats to be vec3 pos, vec2 uv, vec3 nrm). Then, you specify the vertices and indices (0 if none) and if it uses compression (which takes a lot longer to generate and load, but is smaller in space).

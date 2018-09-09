# Osomi Shader Buffer (.oiSB)
The file format was created to support storage of shader buffer reflection data. This format is mainly meant for shader buffers, but could potentially be used for any C-style struct (that only uses raw types).  
# File specification
## Header
```cpp
struct SBHeader {

	char header[4];     //oiSB

	u8 version;         //SBHeaderVersion_s
	u8 flags;           //SBHeaderFlag; & 0x3 = Buffer type
	u16 arrays;

	u16 structs;
	u16 vars;

	u32 bufferSize;

};
```
'version' is the oiSB version; currently there's only version v0_1 supported, but there could be newer editions that support different types if any new GPU types are introduced.  
'flags' contains the flag info; IS_WRITE (1) defines if the buffer can be written to (from within shader), IS_STORAGE (2) defines if the buffer is handled as large buffer (aka structured shader buffer object (SSBO)), IS_ALLOCATED (4) defines if the buffer should be taken care of by the user or if it's automatically created (0x4 is checked unless the variable name contains `_ext`).  
'arrays' is the number of arrays that are stored in this structured buffer (each one is unique).  
'structs' specifies the number of structs in this buffer. A struct is defined as a block of data types.  
'vars' specifies the number of variables in this buffer. A variable is defined as a raw data type (sized matrix/vector/float/int/etc.).  
'bufferSize' specifies the total size of this buffer. If the buffer contains a dynamically sized array, the first bit (0x80 00 00 00) is checked. In this case, flag 0x4 (allocate buffer) won't be checked, because the size of the array can't be determined. Requiring you manually instantiate it.
## Struct
As said before, a struct is defined as a block of data types. This means that any block of variables requires this type. It contains a name in the referenced oiSL file (see oiSL.md), which it references to. 
```cpp
struct SBStruct {

	u16 nameIndex;
	u16 parent;

	u32 offset;

	u16 arrayIndex;
	u8 flags;
	u8 padding;

	u32 length;
      
};
```
'nameIndex' is the index of the struct's name in the oiSL file (usually included at the end of the oiSH file).  
'parent' is the index of the parent struct (this CANNOT be a variable, it must either be 0 (no parent) or the index of the parent).  
'offset' is the relative offset from the start of the parent's struct.  
'arrayIndex' is the the index to the array sizes that are used for this variable.  
'flags' is the SBVarFlag, it tells what type the array is. IS_MATRIX (1) is used to indicate that there's columns specified in the array and that low-level it is a matrix. IS_ARRAY (2) is used when it is an array, this can be used in combination with IS_MATRIX and IS_DYNAMIC. IS_DYNAMIC (4) is used when the size of the struct can't be determined, this is when the outer array has a size zero. This can only happen for the most bottom variable in a buffer.  
'padding' is there for padding this struct to 16 bytes, aligning it properly and reserving space for future use.  
'length' is the stride of this struct.  
## Var
A variable is anything low level (matrix, vector or regular data types) and can also be an array.
```cpp
struct SBVar {

	u16 nameIndex;
	u16 parent;

	u32 offset;
	
	u8 type;
	u8 flags;
	u16 arrayIndex;

};
```
'nameIndex' is the index of the struct's name in the oiSL file (usually included at the end of the oiSH file).  
'parent' is the index of the parent struct (this CANNOT be a variable, it must either be 0 (no parent) or the index of the parent).  
'offset' is the relative offset from the start of the parent's struct.  
'arraySize' is the number of elements in the array (0 = dynamically sized (only available for last struct), 1 = single element, n = array).  
'type' is the TextureFormat that represents this variable (matrices are a special case, because there is no format for them, they are treated as an array with a matrix flag). See ogc.md.  
'arrayIndex' is the the index to the array sizes that are used for this variable.  
'flags' is the SBVarFlag, it tells what type the array is. IS_MATRIX (1) is used to indicate that there's columns specified in the array and that low-level it is a matrix. IS_ARRAY (2) is used when it is an array, this can be used in combination with IS_MATRIX and IS_DYNAMIC. IS_DYNAMIC (4) is used when the size of the struct can't be determined, this is when the outer array has a size zero. This can only happen for the most bottom variable in a buffer.  
'padding' is there for padding this struct to 16 bytes, aligning it properly and reserving space for future use.  
# API usage
## Writing
Writing an oiSB file is easy, but creating one is difficult. If you have a ShaderBufferInfo struct, you can write it to disk by converting it first:
```cpp
ShaderBufferInfo bufferInfo = ...;
SLFile outputNames(String::getDefaultCharset(), {});
SBFile file = oiSB::convert(bufferInfo, outputNames);
Buffer toBuffer = oiSB::write(file);
```
You can also directly write it to file by inserting the output path as the second argument for the write function. Remember that an oiSB file doesn't store its own names, which is why you pass a SLFile it stores data into. In an oiSH file, it is stored next to all other names, to optimize string storage. (For more info check out ogc.md)
## Reading
Reading can be done using the following code:
```cpp
SLFile myNames = ...;
SBFile file;
if(!oiSB::read("res/shaders/myBuffer.oiSB", file))
	; //Handle error
ShaderBufferInfo myInfo = oiSB::convert(file, myNames);
```
The SLFile has to be loaded some other way, as oiSB doesn't include it. Now you can use g.create(objectName, myInfo) to create a Graphics Object from it. (For more info check out ogc.md).

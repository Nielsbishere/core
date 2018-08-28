# Osomi Shader Buffer (.oiSB)
The file format was created to support storage of shader buffer reflection data. This format is mainly meant for shader buffers, but could potentially be used for any C-style struct (that only uses raw types).  
# File specification
## Header
```cpp
		struct SBHeader {

			char header[4];     //oiSB

			u8 version;         //SBHeaderVersion_s
			u8 flags;           //SBHeaderFlag; & 0x3 = Buffer type
			u16 padding = 0;

			u16 structs;
			u16 vars;

			u32 bufferSize;

		};
```
'version' is the oiSB version; currently there's only version v0_1 supported, but there could be newer editions that support different types if any new GPU types are introduced.  
'flags' contains the flag info; IS_WRITE (1) defines if the buffer can be written to (from within shader), IS_STORAGE (2) defines if the buffer is handled as large buffer (aka structured shader buffer object (SSBO)), IS_ALLOCATED (4) defines if the buffer should be taken care of by the user or if it's automatically created (0x4 is checked unless the variable name contains '_ext').  
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

			u32 arraySize;

			u32 length;
      
};
```
'nameIndex' is the index of the struct's name in the oiSL file (usually included at the end of the oiSH file).  
'parent' is the index of the parent struct (this CANNOT be a variable, it must either be 0 (no parent) or the index of the parent).  
'offset' is the relative offset from the start of the parent's struct.  
'arraySize' is the number of elements in the array (0 = dynamically sized (only available for last struct), 1 = single element, n = array).  
'length' is the stride of this struct.  
## Var
A variable is anything low level (matrix, vector or regular data types) and can also be an array.
```cpp
struct SBVar {

      u16 nameIndex;
      u16 parent;

			u32 offset;

			u32 arraySize;

			u8 type;		//TextureFormat
			u8 padding[3];
      
};
```
'nameIndex' is the index of the struct's name in the oiSL file (usually included at the end of the oiSH file).  
'parent' is the index of the parent struct (this CANNOT be a variable, it must either be 0 (no parent) or the index of the parent).  
'offset' is the relative offset from the start of the parent's struct.  
'arraySize' is the number of elements in the array (0 = dynamically sized (only available for last struct), 1 = single element, n = array).  
'type' is the TextureFormat that represents this variable (matrices are a special case, because there is no format for them, they are treated as an array with a matrix flag). See ogc.md.  
'padding' is there for padding this struct to 16 bytes, aligning it properly and reserving space for future use.  
# (TODO) API usage

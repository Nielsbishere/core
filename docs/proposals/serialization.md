# Serialization

ocore has two types of serialization; JSON (Text) and Buffer (Binary) serialization. Binary serialization is used most often, as it is way more efficient.

## Binary serialization

In binary serialization, there's two subtypes; POD (plain old data) or non-POD (e.g. special classes). POD is treated as basic data, while non-POD has to have a serialize and serializeSize function. serializeSize will return the size that is required to store the struct to disk, while serialize will write the struct to disk.

### Make serializable

```cpp
struct MyStruct {
	u32 t0;
	u16 t1;
	u8 t2;
	u8 t2;
};
```

"MyStruct" is immediately serializable, without any of our intervention. However, if you add default arguments, it will implicitly generate a default constructor, thus making it non-serializable by default.

If your struct contains an Array or anything that has a constructor, you have to define the serialize/serializeSize functions yourself:

```cpp
struct MultipleStructs {
    
    Array<MyStruct> myArray;
    
    void serialize(u8 *ptr){
        BufferSerialize::writeData(ptr, myArray);
    }
    
    size_t serializeSize() const {
        return BufferSerialize::dataSize(myArray);
    }
    
};
```

In the example above, we have a serializable Array of MyStruct. However, if you typedef MultipleStructs as `Array<MyStruct>` it would be serializable already, since Arrays and Strings are serializable by default.

### To disk

To get one or several structs to disk, you can call BufferSerialize::write with the arguments;

```cpp
Array<Array<String>> testing{ { "AB", "CD", "E" }, { "FGHIJK", "LMNOP" } };
String val = "ABCD";

CopyBuffer result;
BufferSerialize::write(result, testing, val, val); 
```

The example above will generate the string "ABCDEFGHIJKLMNOPABCDABCD" into our result buffer. **These structs have to be serializable.**

### Common mistakes

+ using size_t or raw pointers in a serializable struct
+ not defining a serialize and/or serializeSize function when the object has a constructor and/or destructor

## Text serialization

When using JSON serialization on a item that uses Binary serialization only, it will still work, however, it will generate nameless member variables. Take our "Binary serialization: To disk" example:

```cpp
Array<Array<String>> testing{ { "AB", "CD", "E" }, { "FGHIJK", "LMNOP" } };
String val0 = "ABCD", val1 = val0;

JSON result;
JSONSerialize::write(result, testing, val0, val1); 
```

This will turn into the following JSON:

```cpp
[ [ [ "AB", "CD", "E" ], [ "FGHIJK", "LMNOP" ] ], "ABCD", "ABCD" ]
```

If you want this to look like a proper JSON, you'd have to provide the names of the member variables like the following;

```cpp
JSON result;
JSONSerialize::write(result, { "testing", "val0", "val1" }, testing, val0, val1); 
```

Which generates:

```cpp
{ "testing": [ [ "AB", "CD", "E" ], [ "FGHIJK", "LMNOP" ] ], "val0": "ABCD", "val1": "ABCD" }
```

There is also a macro that does this for you;

```cpp
JSON result;
oiserialize(result, testing, val0, val1);
```

Which expands to the same code; except it generates the string list from the arguments given. `String(#__VA_ARGS__).replace(' ', '').replace('\t', '').split(',')`.


# OCore
Osomi Core - A basic template library for (graphic) engines
## OSTLC (Osomi Standard Template Library Core)
### Underscore 'operator'/macro
Underscore has a specific use in OSTLC; it is a 'function' that escapes commas so they can be used in other macros. The reason for this is because C++ macros use a comma as a split character, therefore not allowing you to input any commas into a macro. This does however mean that the syntax will look different, but it is worth it, seeing all of the benefits from it:
```cpp
#define someType(x, y, z) x y = z;
```
This example is impossible to write when you are using C++ STL containers;
```cpp
someType(std::unordered_map<OString, u32>, val, { { "Test", 0 }, { "Test2", 1 } });
```
You would think it would generate the following;
```cpp
std::unordered_map<OString, u32> val = { { "Test", 0 }, { "Test2", 1 } };
```
However, because it uses commas to split, it will see the following arguments
```cpp
std::unordered_map<OString
u32>
val
{ { "Test"
0}
{ "Test2"
1 } }
```
which doesn't make sense if you put it in. This is where the _ operator comes in.
```cpp
someType(_(std::unordered_map<OString, u32>), val, _({ { "Test", 0 }, { "Test2", 1 } }));
```
The above does generate the correct code. Yes, it doesn't look perfect, but at least you can use commas in a macro now.
### Standard data types
OSTLC includes defines for types and handles vectors and colors as well. The following is how you note objects from the OSTLC:
```cpp
   //Unsigned integer (u<bits>)
  u32 i = 0U;
  
  //Signed integer (i<bits>)
  i32 j = 0;
  
  //Floating point (f<bits>)
  f32 f = 0.f;
  
  //String
  OString someValue = "Test";
  
  //Vector (Vec<n><special>)
  Vec3 someFloatVec3(1, 2, 3);
  Vec4d someDoubleVec4(4, 5, 6, 7);
  Vec2u someUIntVec2(128, 128);
  Vec2i someIntVec2(128);
  TVec<5, f32> someVec5;
  
  //Colors
  RGBA lowPrecision(127, 127, 127, 255);          //32 bits; 4 bytes
  RGBAf mediumPrecision(0.5f, 0.5f, 0.5f, 1.f);   //128 bits; 16 bytes
  RGBAd highPrecision(0.5, 0.5, 0.5, 1.0);        //256 bits; 32 bytes
  Redd depth(0.5);
  RGf redGreen(mediumPrecision);
  RGB baseColor(255, 0, 255);
  
  f32 depthf = depth[0];
  
```
### 'Java' enums
Not exclusive to Java, but Java has very nice enums. They are compile time values of any class, you can loop through all enums and check their names and values and reference them like regular C++ enums. This is what OSTLC's Enum define is supposed to do; allow you to have a list of all the values of an enum and ways to access them.
#### Enum restrictions
Structured enums only allow basic constexpr data types. They also don't allow you to use custom functions or constructors; they should be pure structs. This is because of limitations and because it should be a compile time constant.
All values have to be declared; so no assuming it will increment the last int, because enums aren't always ints anymore.
#### Defining an enum
Defining a data enum (integer/float) is easy; just use the following (include Template/Enum.h)
```cpp
  DEnum(Name, u32, Value0 = 0, Value1 = 1, Value2 = 2, Value3 = 3);
```
If you want to define a linear data enum (you don't care about the values, you just want to represent names); you can use LDEnum;
```cpp
 LDEnum(Name, u32, Value0, Value1, Value2, Value3);
```
Creating your own (structured) enum can be done using the following;
```cpp
  SEnum(Name, _(i32 x, y, z;), Value0 = { }, Value1 = _({ 1, 2, 3 }), _(Value2 = { 4, 5 }));
```
Remember that the _ macro (escape macro) has to be used whenever your expression contains a comma, so the following would be valid syntax:
```cpp
  SEnum(Name, i32 x; i32 y; i32 z;, Value0 = {}, Value1 = { 1 }, Value2 = { 5 });
```
But the following wouldn't be;
```cpp
  SEnum(Name, i32 x, i32 y, i32 z, Value0 = { 0, 1, 2 }, Value1 = { 1, 2, 3 }, Value2 = { 5, 5, 6 });
```
#### Looping through an enum
You access an enum like you would with any enum class; so Name::Value0 for example. This can evaluate to two values; either const Name_s&, which is the value of that enum, or Name, which contains the current index, name and value.
However, the regular method doesn't allow you to loop through those values, but Osomi Enums do. You simply get the length of the enum; Name::length and make a for loop; then you can write Name n = i; and it will auto detect the name and value of the enum.
```cpp
	for (u32 i = 0; i < SomeTestEnum::length; ++i) {
		const SomeTestEnum_s &it = SomeTestEnum(i);
		printf("%u %u %u %u\n", it.a, it.b, it.c, it.d);
	}
```
The code above gets the enum at i and gets the value, whereafter printing the abcd values for that enum.
### JSON
Osomi Core (STL) wraps around rapidjson to make JSON parsing more intuitive and less of a pain to think about how you're copying stuff and if something already exists. The Utils/JSON.h provides you with a couple of utils, mainly, getting and setting things in the JSON file.  
It treats the JSON more like a file system; you access it by using /'s as seperators and while it does have lists and objects, you can access anything using paths.
#### JSON Paths
```json
{ "testJson": { "object": [ [ 3, 3, 3 ], [ 4, 4, 4 ] ] } }
```
The JSON above can be parsed by using file paths. JavaScript would use the following way of accessing the JSON array;
```js
  var testJson_object_0_1 = json["testJson"]["object"][0][1];
```
However, using the JSON class, we use the following:
```cpp
  i32 testJson_object_0_1 = json.getInt("testJson/object/0/1");
```
This system provides more clarity and allows you to reference a JSON in a better way than by just passing the JSON Value around every time. It doesn't require you to know anything about the JSON you're parsing, as you can also retrieve all 'members' in one function call. This will list all paths that are available, but of course you can also just use the paths that are in the folder you're in (non-recursively). It also allows you to throw in the default value it returns on failure; so getInt can have the 2nd param that is returned when it can't find it, it is the incorrect type or anything else bad happened. By default, this is 0 for numbers, "" for strings and empty vectors for arrays. Setting is pretty much the same, only it takes a value instead of returning it.
#### Checking JSON structure
To validate whether or not something is there; you can use 'exists' and 'mkdir'. Exists checks if the directories leading to an object (including the object itself) actually exist within the JSON. Mkdir tries to ensure the path you gave will be available, but it can fail (if you're referencing to a non-object/non-array), it returns true on success. Mkdir does do a few things by default; when you're in an array and reference an index it doesn't have, it will add elements until it does. If the object doesn't exist, it will set it to an empty object ("{ }"), however, this doesn't matter for how you access it or how you get/set it; an empty object is just seen as filler and can be overriden by anything. mkdir("testJson/object/2") would create an empty object behind the Vec3(4), you can set it to anything and getting things from it isn't possible. mkdir("testJson/object/2/3") after the first mkdir would result into the object turning into an array; `[{}, {}, {}, {}]` instead of '{ }'.
#### Checking 'folders' aka members
You can check the number of members by using 'getMembers'; this returns the fields / members that the object has. Any members inside those won't be counted. getMemberIds returns the relative paths to those members and getMemberNames returns the absolute paths to those members.
Getting all members recursively can be done using getAllMembers and this returns absolute paths.
#### Example (Writing)
The following example is from InputManager; it writes the input bindings & axes to a file:
```cpp
	JSON json;

	for (u32 i = 0; i < (u32)bindings.size(); ++i) {

		OString base = OString("bindings/") + bindings[i].first;
		u32 j = json.getMembers(base);

		json.setString(base + "/" + j, bindings[i].second.toString());
	}

	for (u32 i = 0; i < (u32) axes.size(); ++i) {

		auto &ax = axes[i].second;

		OString base = OString("axes/") + axes[i].first;
		u32 j = json.getMembers(base);

		json.setString(base + "/" + j + "/binding", ax.binding.toString());
		json.setString(base + "/" + j + "/effect", ax.effect == InputAxis1D::X ? "x" : (ax.effect == InputAxis1D::Y ? "y" : "z"));
		json.setFloat(base + "/" + j + "/axisScale", ax.axisScale);
	}

	return json.operator oi::OString().writeToFile(path);
```
### Example (reading)
The following is from InputManager; it reads the input bindings & axes from a file:
```cpp
  JSON json = OString::readFromFile(path);

	if (json.exists("bindings")) {

		for (OString handle : json.getMemberIds("bindings")) {

			for (OString id : json.getMemberIds(OString("bindings/") + handle)) {

				OString bstr = json.getString(OString("bindings/") + handle + "/" + id);
				Binding b(bstr);

				if (b.getType() != BindingType::UNDEFINED && b.getCode() != 0) {
					bind(handle, b);
					Log::println(OString("Binding event ") + b.toString());
				}
				else
					Log::error("Couldn't read binding; invalid identifier");

			}

		}

	}

	if (json.exists("axes")) {
		for (OString handle : json.getMemberIds("axes")) {
			for (OString id : json.getMemberIds(OString("axes/") + handle)) {

				OString base = OString("axes/") + handle + "/" + id;

				OString bstr = json.getString(base + "/binding");
				Binding b(bstr);

				if (b.getType() == BindingType::UNDEFINED || b.getCode() == 0) {
					Log::error("Couldn't read axis; invalid identifier");
					continue;
				}

				OString effect = json.getString(base + "/effect");
				InputAxis1D axis;

				if (effect.equalsIgnoreCase("x")) axis = InputAxis1D::X;
				else if (effect.equalsIgnoreCase("y")) axis = InputAxis1D::Y;
				else if (effect.equalsIgnoreCase("z")) axis = InputAxis1D::Z;
				else {
					Log::error("Couldn't read axis; invalid axis effect");
					continue;
				}

				f32 axisScale = json.getFloat(base + "/axisScale", 1.f);

				bindAxis(handle, InputAxis(b, axis, axisScale));
				Log::println(OString("Binding axis ") + b.toString() + " with axis " + effect + " and scale " + axisScale);
			}
		}
	}

	return true;
```

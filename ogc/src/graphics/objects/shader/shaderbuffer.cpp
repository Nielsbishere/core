#include "graphics/graphics.h"
#include "graphics/objects/shader/shaderbuffer.h"
using namespace oi::gc;
using namespace oi;

///Shader buffer object

ShaderBufferObject::ShaderBufferObject(ShaderBufferObject *parent, u32 offset, u32 length, std::vector<u32> arr, String name, TextureFormat format, SBOFlag flags) : parent(parent), offset(offset), length(length), arr(arr), name(name), format(format), flags(flags) {}
ShaderBufferObject::ShaderBufferObject() : ShaderBufferObject(nullptr, 0, 0, {}, "", 0, (SBOFlag)0) {}

void ShaderBufferObject::addChild(ShaderBufferObject *obj) { childs.push_back(obj); }

ShaderBufferObject *ShaderBufferObject::find(String oname) {
	for (ShaderBufferObject *child : childs)
		if (child->name == oname)
			return child;
	return nullptr;
}

///Shader buffer info

ShaderBufferInfo::ShaderBufferInfo(ShaderRegisterType type, u32 size, u32 elements, bool allocate) : type(type), size(size), elements(elements), allocate(allocate), self(nullptr, 0, size, {}, "", TextureFormat::Undefined, SBOFlag::Value) {}
ShaderBufferInfo::ShaderBufferInfo() : ShaderBufferInfo(ShaderRegisterType::Undefined, 0, 0, false) {}

void ShaderBufferInfo::addRoot(ShaderBufferObject *obj) { self.childs.push_back(obj); }
u32 ShaderBufferInfo::getRoots() { return (u32) self.childs.size(); }

ShaderBufferObject &ShaderBufferInfo::operator[](u32 i) { return elements[i]; }
ShaderBufferObject &ShaderBufferInfo::getRoot(u32 i) { return *self.childs[i]; }

u32 ShaderBufferInfo::lookup(ShaderBufferObject *elem) {

	u32 i = 1U;

	for (ShaderBufferObject &e : elements)
		if (&e == elem) return i;
		else if(e.format == TextureFormat::Undefined) ++i;

	return 0U;
}

ShaderBufferInfo &ShaderBufferInfo::operator=(const ShaderBufferInfo &info) {
	copy(info);
	return *this;
}

ShaderBufferInfo::ShaderBufferInfo(const ShaderBufferInfo &info) {
	copy(info);
}

void ShaderBufferInfo::copy(const ShaderBufferInfo &info) {

	type = info.type;
	size = info.size;
	allocate = info.allocate;
	self = info.self;
	elements = info.elements;

	for (auto &elem : elements) {

		for (auto *&ptr : elem.childs)
			if (ptr == &info.self)
				ptr = &self;
			else
				ptr = elements.data() + (ptr - info.elements.data());

		if (elem.parent == &info.self)
			elem.parent = &self;
		else
			elem.parent = elements.data() + (elem.parent - info.elements.data());
	}

	for (auto *&ptr : self.childs)
		if (ptr == &info.self)
			ptr = &self;
		else
			ptr = elements.data() + (ptr - info.elements.data());

}

void ShaderBufferInfo::push(ShaderBufferObject obj, ShaderBufferObject &parent) {

	u32 targetSize = (u32) elements.size() + 1U;

	u32 parentId = &parent == &self ? 0U : (u32)(&parent - elements.data()) + 1U;

	ShaderBufferInfo copy;
	copy.copy(*this);

	elements.resize(targetSize);
	elements.assign(copy.elements.begin(), copy.elements.end());

	for (u32 i = 0; i < elements.size(); ++i) {

		ShaderBufferObject &sbo = elements[i];

		if (sbo.parent == &copy.self)
			sbo.parent = &self;
		else
			sbo.parent = elements.data() + (sbo.parent - copy.elements.data());

		for (auto *&elem : sbo.childs) {
			if (elem == &copy.self)
				elem = &self;
			else
				elem = elements.data() + (elem - copy.elements.data());
		}

	}

	self.childs.assign(copy.self.childs.begin(), copy.self.childs.end());

	for (auto *&ptr : self.childs)
		if (ptr == &copy.self)
			ptr = &self;
		else
			ptr = elements.data() + (ptr - copy.elements.data());

	elements.push_back(obj);

	ShaderBufferObject *objptr = &elements[elements.size() - 1U];
	(objptr->parent = (parentId == 0 ? &self : elements.data() + (parentId - 1U)))->childs.push_back(objptr);
}

///ShaderBufferVar

ShaderBufferVar::ShaderBufferVar(ShaderBufferObject &obj, Buffer buf) : obj(obj), buf(buf) {}

///ShaderBuffer

const ShaderBufferInfo ShaderBuffer::getInfo() { return info; }

ShaderBufferVar ShaderBuffer::get() {
	return { info.self, buffer->getBuffer() }; 
}

u32 ShaderBuffer::getElements(){ return (u32) info.self.childs.size(); }
u32 ShaderBuffer::getSize() { return (u32) info.size; }

ShaderBuffer *ShaderBuffer::instantiate(u32 objects) {

	if (buffer == nullptr) {
		setObjectCount(objects);
		buffer = g->create(getName() + " buffer", GBufferInfo(info.type.getValue() - 1, info.size));
		g->use(buffer);
	} else
		Log::throwError<ShaderBuffer, 0x0>("Can't set the buffer when it's already set");

	return this;

}

void ShaderBuffer::setBuffer(u32 objects, GBuffer *gb) {

	if (buffer != nullptr)
		Log::throwError<ShaderBuffer, 0x1>("Can't set the buffer when it's already set");

	setObjectCount(objects);

	if(gb == nullptr || gb->getSize() != info.size)
		Log::throwError<ShaderBuffer, 0x2>("GBuffer size should match ShaderBuffer size");

	buffer = gb;

}

void ShaderBuffer::setObjectCount(u32 count) {

	for (ShaderBufferObject &obj : info.elements) {
		auto &arr = obj.arr;

		if (arr.size() != 0 && arr[arr.size() - 1] == 0) {

			arr[arr.size() - 1] = count;
			u32 siz = 1;

			for (u32 i : arr)
				siz *= i;

			info.size += siz * obj.length;
			break;
		}
	}

}

GBuffer *ShaderBuffer::getBuffer() {
	return buffer;
}

void ShaderBuffer::set(Buffer buf) {

	if (buffer == nullptr || buf.size() == 0U)
		Log::throwError<ShaderBuffer, 0x4>("ShaderBuffer::copy requires a buffer to be set and ShaderBuffer has to be open");

	buffer->set(buf);

}

ShaderBuffer::ShaderBuffer(ShaderBufferInfo info): info(info) {}
ShaderBuffer::~ShaderBuffer() {

	if (buffer != nullptr)
		g->destroy(buffer);

}

bool ShaderBuffer::init() {

	if (info.allocate) {
		buffer = g->create(getName() + " buffer", GBufferInfo(info.type.getValue() - 1, info.size));
		g->use(buffer);
	}

	return true;
}

void ShaderBuffer::calculateArrayInfo(std::vector<u32> &off, std::vector<u32> &len, u32 stride, u32 &offset, u32 &count) {

	u32 totalOffset = 0;
	count = 1;

	for (u32 x : len)
		count *= x;

	for (u32 i = 0, j = (u32)off.size(), k = (u32)len.size(); i < j && i < k; ++i) {
		u32 siz = len[k - 1 - i];
		count /= siz;
		totalOffset += (off[i] % siz) * count;
	}

	offset += totalOffset * stride;

}

ShaderBufferVar ShaderBuffer::get(String path) {

	if (path == "") return get();

	ShaderBufferObject *sbo = &info.self;
	u32 offset = 0, count = 0;

	std::vector<u32> arr;
	std::vector<String> paths = path.split("/");

	for (String str : paths) {

		if (!str.isUint()) {

			if (arr.size() != 0) {
				calculateArrayInfo(arr, sbo->arr, sbo->length, offset, count);
				arr.clear();
			}

			if ((sbo = sbo->find(str)) == nullptr)
				Log::throwError<ShaderBufferVar, 0x7>(String("Couldn't find the path \"") + path + "\"");
			else
				offset += sbo->offset;

		} else 
			arr.push_back((u32)str.toLong());
		
	}

	calculateArrayInfo(arr, sbo->arr, sbo->length, offset, count);

	return { *sbo, Buffer::construct(buffer->getAddress() + offset, sbo->length * count) };
}
#include "graphics/shaderbuffer.h"
#include "graphics/graphics.h"
using namespace oi::gc;
using namespace oi;

///Shader buffer object

ShaderBufferObject::ShaderBufferObject(ShaderBufferObject *parent, u32 offset, u32 length, u32 arraySize, String name, TextureFormat format) : parent(parent), offset(offset), length(length), arraySize(arraySize), name(name), format(format) {}
ShaderBufferObject::ShaderBufferObject() : ShaderBufferObject(nullptr, 0, 0, 0, "", 0) {}

void ShaderBufferObject::addChild(ShaderBufferObject *obj) { childs.push_back(obj); }

ShaderBufferObject *ShaderBufferObject::find(String name) {
	for (ShaderBufferObject *child : childs)
		if (child->name == name)
			return child;
	return nullptr;
}

///Shader buffer info

ShaderBufferInfo::ShaderBufferInfo(ShaderRegisterType type, u32 size, u32 elements, bool allocate) : type(type), size(size), elements(elements), allocate(allocate), self(nullptr, 0, size, 1, "", TextureFormat::Undefined) {}
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

ShaderBufferVar::ShaderBufferVar(ShaderBufferObject &obj, Buffer buf, bool available) : obj(obj), buf(buf), available(available) {}

///ShaderBuffer

const ShaderBufferInfo ShaderBuffer::getInfo() { return info; }

ShaderBufferVar ShaderBuffer::get() {
	return { info.self, current, isOpen }; 
}

u32 ShaderBuffer::getElements(){ return (u32) info.self.childs.size(); }
u32 ShaderBuffer::getSize() { return (u32) info.size; }

void ShaderBuffer::setBuffer(GBuffer *buf) {
	if (buffer == nullptr)
		buffer = buf;
	else
		Log::throwError<ShaderBuffer, 0x0>("Can't set the buffer when it's already set");
}

GBuffer *ShaderBuffer::getBuffer() {
	return buffer;
}

void ShaderBuffer::open() {

	isOpen = true;

	if (buffer != nullptr) {
		buffer->open();
		this->current = Buffer::construct(buffer->getAddress(), buffer->getSize());
	}
	else
		Log::throwError<ShaderBuffer, 0x1>("Can't update a non-existent buffer");

}

void ShaderBuffer::copy(Buffer buf) {

	if (buffer == nullptr || buf.size() == 0U || !isOpen)
		Log::throwError<ShaderBuffer, 0x4>("ShaderBuffer::copy requires a buffer to be set and ShaderBuffer has to be open");

	buffer->copy(buf);

}

void ShaderBuffer::set(Buffer buf) {

	if (buffer == nullptr || buf.size() == 0U)
		Log::throwError<ShaderBuffer, 0x5>("ShaderBuffer::set requires a buffer to be set");

	buffer->set(buf);
}

void ShaderBuffer::close() {

	isOpen = false;
	this->current = {};

	if (buffer != nullptr)
		buffer->close();
	else
		Log::throwError<ShaderBuffer, 0x2>("Can't update a non-existent buffer");

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

ShaderBufferVar ShaderBuffer::get(String path) {

	if (path == "") return get();

	ShaderBufferObject *sbo = &info.self;
	u32 offset = 0;

	std::vector<u32> arr;

	for (String str : path.split("/")) {

		if (!str.isUint()) {

			if (arr.size() != 0) {
				//TODO: Calculate offset
				arr.clear();
			}

			if ((sbo = sbo->find(str)) == nullptr)
				Log::throwError<ShaderBufferVar, 0x3>(String("Couldn't find the path \"") + path + "\"");

			offset += sbo->offset;

		} else 
			arr.push_back((u32)str.toLong());
		
	}

	if (arr.size() != 0) {
		//TODO: Calculate offset
	}

	return { *sbo, Buffer::construct(this->current.addr() + offset, sbo->length * sbo->arraySize), isOpen };
}
#include "graphics/graphics.h"
#include "graphics/objects/graphicsobject.h"
using namespace oi::gc;
using namespace oi;

std::unordered_map<size_t, String> GraphicsObject::names = std::unordered_map<size_t, String>();

GraphicsObject::~GraphicsObject() { g->remove(this); }
size_t GraphicsObject::getHash() const { return hash; }
String GraphicsObject::getTypeName() const { return names[hash]; }
String GraphicsObject::getName() const { return name; }
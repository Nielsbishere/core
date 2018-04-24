#include "graphics/graphicsobject.h"
#include "graphics/graphics.h"
using namespace oi::gc;
using namespace oi;

GraphicsObject::~GraphicsObject() { g->remove(this); }
size_t GraphicsObject::getHash() const { return hash; }
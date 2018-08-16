#pragma once
#include "oirm.h"

namespace oi {

	namespace gc {

		//We only accept converting obj files
		//This is because Obj files only support position, uv and normal.
		//oiRM supports custom inputs, as well as sized inputs, miscs (additional mesh information), etc.
		//Obj doesn't support skeletal meshes / animations either.
		//oiRM is made specifically for Tiled-Material Indirect Rendering; which isn't what obj is made for.
		//All convert functions convert to oiRM format as a Buffer.
		struct Obj {

			static Buffer convert(Buffer objBuffer, bool compression);
			static bool convert(Buffer objBuffer, String outPath, bool compression);

			static Buffer convert(String objPath, bool compression);
			static bool convert(String objPath, String outPath, bool compression);

		};

	}

}
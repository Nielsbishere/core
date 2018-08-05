#pragma once

#include "types/generic.h"
#include "types/vector.h"

namespace oi {

	class Grad {

	public:

		f32 x, y, z, w;
		Grad(f32 x, f32 y, f32 z);
		Grad(f32 x, f32 y, f32 z, f32 w);
		Grad();
	};

	/*
	* !SimplexNoise.h
	*
	*	Created on: Apr 10, 2016
	*	Last edit:	Oct 2, 2017
	*    	Authors:
	*    		Stefan Gustavson (stegu@itn.liu.se) (Example code)
	*    		Peter Eastman (peastman@drizzle.stanford.edu) (Optimizations)
	*    		Sebastian Lague (sebastlague@gmail.com) (Octaves, lacunarity, persistance upgrade)
	*    		Niels Brunekreef (n@osomi.net)	(Conversion to c++ and 1D noise)
	*											(Port to Lunarc)
	*											(Port to Loki/oiC)
	*    	Credits: http://webstaff.itn.liu.se/~stegu/simplexnoise/SimplexNoise.java
	*/
	class SimplexNoise {

	public:

		static f32 noise(Vec2 p);
		static f32 noise(Vec3 p);
		static f32 noise(Vec4 p);
		static f32 noise(f32 p);

		static f32 noise(f32 p, i32 octaves, f32 persistance, f32 lacunarity);
		static f32 noise(Vec2 p, i32 octaves, f32 persistance, f32 lacunarity);
		static f32 noise(Vec3 p, i32 octaves, f32 persistance, f32 lacunarity);
		static f32 noise(Vec4 p, i32 octaves, f32 persistance, f32 lacunarity);


	private:

		/// To remove the need for index wrapping, f32 the permutation table length
		static Grad grad3[12];
		static Grad grad4[32];
		static u8 p[512], perm[512], permMod12[512];

		static inline i32 fastfloor(f32 x);
		static inline f32 dot(Grad g, f32 x, f32 y);
		static inline f32 dot(Grad g, f32 x, f32 y, f32 z);
		static inline f32 dot(Grad g, f32 x, f32 y, f32 z, f32 w);

		/// Skewing and unskewing factors for 2, 3, and 4 dimensions
		///These have to be initialized, they are not constant because they use a function
		static f32 F2, G2, F3, G3, F4, G4;
	};
}
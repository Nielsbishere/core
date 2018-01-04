#pragma once

#include "Types/Generic.h"
#include "Types/Vector.h"

namespace oi {

	class Grad {

	public:

		f64 x, y, z, w;
		Grad(f64 x, f64 y, f64 z);
		Grad(f64 x, f64 y, f64 z, f64 w);
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

		static f64 noise(Vec2 p);
		static f64 noise(Vec3 p);
		static f64 noise(Vec4 p);
		static f64 noise(f64 p);

		static f64 noise(f64 p, i32 octaves, f64 persistance, f64 lacunarity);
		static f64 noise(Vec2 p, i32 octaves, f64 persistance, f64 lacunarity);
		static f64 noise(Vec3 p, i32 octaves, f64 persistance, f64 lacunarity);
		static f64 noise(Vec4 p, i32 octaves, f64 persistance, f64 lacunarity);


	private:

		/// To remove the need for index wrapping, f64 the permutation table length
		static Grad grad3[12];
		static Grad grad4[32];
		static u8 p[512], perm[512], permMod12[512];

		static i32 fastfloor(f64 x);
		static f64 dot(Grad g, f64 x, f64 y);
		static f64 dot(Grad g, f64 x, f64 y, f64 z);
		static f64 dot(Grad g, f64 x, f64 y, f64 z, f64 w);

		/// Skewing and unskewing factors for 2, 3, and 4 dimensions
		///These have to be initialized, they are not constant because they use a function
		static f64 F2, G2, F3, G3, F4, G4;
	};
}
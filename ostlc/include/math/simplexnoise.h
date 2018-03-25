#pragma once

#include "types/generic.h"
#include "types/vector.h"

namespace oi {

	class Grad {

	public:

		flp x, y, z, w;
		Grad(flp x, flp y, flp z);
		Grad(flp x, flp y, flp z, flp w);
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

		static flp noise(Vec2 p);
		static flp noise(Vec3 p);
		static flp noise(Vec4 p);
		static flp noise(flp p);

		static flp noise(flp p, i32 octaves, flp persistance, flp lacunarity);
		static flp noise(Vec2 p, i32 octaves, flp persistance, flp lacunarity);
		static flp noise(Vec3 p, i32 octaves, flp persistance, flp lacunarity);
		static flp noise(Vec4 p, i32 octaves, flp persistance, flp lacunarity);


	private:

		/// To remove the need for index wrapping, flp the permutation table length
		static Grad grad3[12];
		static Grad grad4[32];
		static u8 p[512], perm[512], permMod12[512];

		static inline i32 fastfloor(flp x);
		static inline flp dot(Grad g, flp x, flp y);
		static inline flp dot(Grad g, flp x, flp y, flp z);
		static inline flp dot(Grad g, flp x, flp y, flp z, flp w);

		/// Skewing and unskewing factors for 2, 3, and 4 dimensions
		///These have to be initialized, they are not constant because they use a function
		static flp F2, G2, F3, G3, F4, G4;
	};
}
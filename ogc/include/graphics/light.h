#pragma once

namespace oi {

	namespace gc {

		struct Light {

			Vec3 pos = {};
			f32 intensity = 0.f;

			Vec3 col = {};
			f32 radius = 0.f;

			Vec3 dir = {};
			f32 angle = 0.f;

			//Point light
			Light(Vec3 position, f32 intensity, Vec3 color, f32 radius) : pos(position), intensity(intensity), col(color), radius(radius) {}

			//Directional light
			Light(Vec3 direction, f32 intensity, Vec3 color) : intensity(intensity), col(color), dir(direction) {}

			//Spot light
			Light(Vec3 position, f32 intensity, Vec3 color, f32 radius, Vec3 direction, f32 angle) : pos(position), intensity(intensity), col(color), radius(radius), dir(direction), angle(angle) {}

		};

	}

}
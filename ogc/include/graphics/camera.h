#pragma once

#include <types/vector.h>
#include <types/matrix.h>
#include "graphicsobject.h"

#undef near
#undef far

namespace oi {

	namespace gc {

		class Graphics;
		class ShaderBuffer;
		class Pipeline;

		struct CameraStruct {

			Matrix p;

			Matrix v;

			Vec3 position;
			f32 fov;

			Vec3 up;
			f32 aspect;

			Vec3 forward;
			f32 padding;

			f32 near;
			f32 far;
			Vec2u resolution;

			CameraStruct() {}
			CameraStruct(Matrix p, Matrix v, Vec3 position, f32 fov, Vec3 up, f32 aspect, Vec3 forward, f32 near, f32 far, Vec2u resolution) : p(p), v(v), position(position), fov(fov), up(up), aspect(aspect), forward(forward), near(near), far(far), resolution(resolution) {}

		};

		struct CameraInfo {

			Vec3 position;
			f32 fov;

			Vec3 up;
			f32 near;

			Vec4 directionOrCenter;

			f32 far;

			CameraInfo(f32 fov = 45.f, Vec3 position = { }, Vec4 directionOrCenter = { 0, 0, -1, 0 }, Vec3 up = { 0, 1, 0 }, f32 near = 0.1f, f32 far = 100.f) : fov(fov), position(position), directionOrCenter(directionOrCenter), up(up), near(near), far(far) {}

		};

		class Camera : public GraphicsObject {

			friend class Graphics;

		public:

			const CameraInfo getInfo();

			void bind(Vec2u res, f32 ratio);
			CameraStruct getBound() const;

			Matrix getBoundProjection() const;
			Matrix getBoundView() const;

			void move(Vec3 dposition);

			void setPosition(Vec3 position);
			void setFov(f32 fov);
			void setNear(f32 near);
			void setFar(f32 far);
			void setUp(Vec3 up);
			void setDirection(Vec3 direction);
			void setCenter(Vec3 center);

		protected:

			~Camera();
			Camera(CameraInfo info);
			bool init();

		private:

			CameraInfo info;
			CameraStruct bound;

		};

	}

}
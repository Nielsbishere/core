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

			Matrixf p;

			Matrixf v;

			Vec3f position;
			f32 fov;

			Vec3f up;
			f32 aspect;

			Vec3f forward;
			f32 padding;

			f32 near;
			f32 far;
			Vec2u resolution;

			CameraStruct() {}
			CameraStruct(Matrixf p, Matrixf v, Vec3f position, f32 fov, Vec3f up, f32 aspect, Vec3f forward, f32 near, f32 far, Vec2u resolution) : p(p), v(v), position(position), fov(fov), up(up), aspect(aspect), forward(forward), near(near), far(far), resolution(resolution) {}

		};

		struct CameraInfo {

			Vec3f position;
			f32 fov;

			Vec3f up;
			f32 near;

			Vec4f directionOrCenter;

			f32 far;

			CameraInfo(f32 fov = 45.f, Vec3f position = { }, Vec4f directionOrCenter = { 0, 0, -1, 0 }, Vec3f up = { 0, 1, 0 }, f32 near = 0.1f, f32 far = 100.f) : fov(fov), position(position), directionOrCenter(directionOrCenter), up(up), near(near), far(far) {}

		};

		class Camera : public GraphicsObject {

			friend class Graphics;

		public:

			const CameraInfo getInfo();

			CameraStruct getStruct(Pipeline *pipeline);
			CameraStruct getBound() const;

			Matrixf getBoundProjection() const;
			Matrixf getBoundView() const;

			void move(Vec3f dposition);

			void setPosition(Vec3f position);
			void setFov(f32 fov);
			void setNear(f32 near);
			void setFar(f32 far);
			void setUp(Vec3f up);
			void setDirection(Vec3f direction);
			void setCenter(Vec3f center);

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
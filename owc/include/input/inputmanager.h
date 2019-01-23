#pragma once

#include "inputhandler.h"
#include "types/vector.h"

namespace oi {

	namespace wc {

		enum class InputAxes2D {
			XY, XZ, YZ, YX, ZX, ZY
		};

		enum class InputAxis1D {
			X, Y, Z
		};

		struct InputAxis {

			Binding binding;
			InputAxis1D effect;
			f32 axisScale, threshold;
			bool delta;

			InputAxis(Binding binding, InputAxis1D effect, f32 axisScale = 1, bool delta = false, f32 threshold = 0);
		};

		struct MappedStateBinding {

			std::vector<Binding> bindings;
			InputState value;

		};

		struct MappedAxisBinding {

			std::vector<InputAxis> bindings;
			Vec3 value;

		};

		class InputManager {

		public:

			InputManager(InputHandler *handler);

			void update();
			bool contains(String handle) const;
			void clear(String handle);
			void clear();

			///InputStates

			bool containsState(String handle) const;

			InputState getState(String handle) const;
			bool isDown(String handle) const;
			bool isUp(String handle) const;
			bool isPressed(String handle) const;
			bool isReleased(String handle) const;

			void bindState(String handle, Binding binding);
			void clearState(String handle);

			///Axes

			bool containsAxis(String handle) const;

			void bindAxis(String handle, InputAxis axis);
			void clearAxis(String handle);

			Vec3 getAxis(String handle) const;
			Vec2 getAxis2D(String handle, InputAxes2D ia = InputAxes2D::XY) const;
			f32 getAxis1D(String handle, InputAxis1D ia = InputAxis1D::X) const;

			///Loading/writing

			bool load(String loadPath);
			bool write(String writepath) const;
			String write() const;

		private:
			
			std::unordered_map<String, MappedStateBinding> states;
			std::unordered_map<String, MappedAxisBinding> axes;

			InputHandler *handler;

		};

	}

}
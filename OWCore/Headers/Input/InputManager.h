#pragma once

#include <Types/OString.h>
#include "Binding.h"
#include "InputHandler.h"

namespace oi {

	namespace wc {

		enum class InputAxes2D {
			XY, YZ, XZ
		};

		enum class InputAxis1D {
			X, Y, Z
		};

		struct InputAxis {

			Binding binding;
			InputAxis1D effect;
			f32 axisScale;

			InputAxis(Binding _binding, InputAxis1D _effect, f32 _axisScale);

			OString toString();
		};

		template<typename T>
		struct TKeyPair {

			OString first;
			T second;

			bool operator<(const TKeyPair &other) {
				return first < other.first;
			}

		};

		class InputManager {

		public:

			InputManager(InputHandler &handler);

			bool isDown(OString handle);
			bool isUp(OString handle);
			bool isPressed(OString handle);
			bool isReleased(OString handle);

			void bind(OString handle, Binding binding);
			void unbind(OString handle, Binding binding);

			void unbind(OString handle);

			std::vector<Binding> getBound(OString handle);

			void bindAxis(OString handle, InputAxis axis);

			Vec3 getAxis(OString handle);
			Vec2 getAxis2D(OString handle, InputAxes2D ia = InputAxes2D::XY);
			f32 getAxis1D(OString handle, InputAxis1D ia = InputAxis1D::X);

			//Loads in the presets from a JSON
			bool load(OString json);

			//Writes the presets to a JSON
			OString write();

		private:

			std::vector<TKeyPair<Binding>> bindings;
			std::vector<TKeyPair<InputAxis>> axes;
			InputHandler &handler;

		};

	}

}
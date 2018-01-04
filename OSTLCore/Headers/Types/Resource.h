#pragma once

#include "OString.h"

namespace oi {

	class Resource {

	public:

		virtual bool load(const OString &path) = 0;
		virtual bool write(const OString &path) = 0;

	};
}
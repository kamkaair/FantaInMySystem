#pragma once        // include only once
#include <typeinfo>

namespace kgfw {

	// Base class to be used as an base class of each class. Provides detection of memoty leaks.
	class Object {
	public:
		// Default constructor
		Object(const char* const __function__);

		// Destructor
		virtual ~Object();

		// Returns number of allocated objects.
		static int getRefCount();

	private:
		// Hidden copy constructor and assignment operator for avoiding unnecessary class copying, for avoiding problems.
		// These will cause compiling error, if tried to copy or assign an object inherited from Object-class.
		Object();
		Object(const Object&);
		Object& operator=(const Object&);
	};

}

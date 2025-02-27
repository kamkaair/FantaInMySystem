#include <kgfw/Object.h>    // Include class header filebuf
#include <assert.h>         // assert
#include <stdio.h>          // printf

namespace kgfw {

	// Anynomous namespace for file internal classes
	namespace {
		// RefCounter-class
		class RefCounter {
		public:
			RefCounter()
				: m_numOfObjectRefs(0) {
			}

			~RefCounter() {
				if (m_numOfObjectRefs == 0) {
					printf("\nNo memory leaks detected.\n");
				}
				else {
					printf("\n%d Memory leaks detected! Delete all objects using \"delete\" which you have created using \"new\".\n", getRefCount());
				}
			}

			void addRef() {
				++m_numOfObjectRefs;
			}

			void removeRef() {
				assert(getRefCount() >= 1); // Negative ref count not allowed!
				--m_numOfObjectRefs;
			}

			int getRefCount() const {
				return m_numOfObjectRefs;
			}

		private:
			int m_numOfObjectRefs;
		};

		// Global reference counter for Objects.
		RefCounter g_refCounter;
	}


	Object::Object(const char* const __function__) {
		g_refCounter.addRef();
	}


	Object::~Object() {
		g_refCounter.removeRef();
	}
	

	int Object::getRefCount() {
		return g_refCounter.getRefCount();
	}
}

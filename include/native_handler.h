#ifndef _NATIVE_HANDLER_H_
#define _NATIVE_HANDLER_H_

#include <am-vector.h>

template <typename T>
class NativeHandler {
private:
	ke::Vector<T*> m_handles;

public:
	NativeHandler() {
		//...
	}

	~NativeHandler() {
		this->clear();
	}

public:
	void clear() {
		for (size_t i = 0; i < m_handles.length(); i++) {
			if (m_handles[i] != nullptr) {
				delete m_handles[i];
			}
		}
	}

	template <typename... Targs>
	int create(Targs... Fargs) {
		for (size_t i = 0; i < m_handles.length(); i++) {
			if (m_handles[i] != nullptr) {
				m_handles[i] = new T(Fargs...);
				return static_cast<int>(i) + 1;
			}
		}

		m_handles.append(new T(Fargs...));
		return m_handles.length();
	}

	T* lookup(int handle) {
		--handle;
		if (handle < 0 || handle > static_cast<int>(m_handles.length())) {
			return nullptr;
		}

		return m_handles[handle];
	}

	bool destroy(int handle) {
		handle--;
		if (handle < 0 || handle > static_cast<int>(m_handles.length())) {
			return false;
		}

		if (m_handles[handle] == nullptr) {
			return false;
		}

		delete m_handles[handle];
		m_handles[handle] = nullptr;
		return true;
	}
};

#endif // _NATIVE_HANDLER_H_
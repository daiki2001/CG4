#pragma once

template <class T>
inline void safe_delete(T*& p) {
	if (p == nullptr)
	{
		return;
	}

	delete p;
	p = nullptr;
}

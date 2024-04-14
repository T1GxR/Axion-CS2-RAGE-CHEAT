#pragma once

struct Resource_t
{
	void* pData;
};

template <typename T>
class stronghandle
{
public:
	operator T* () const
	{
		if (pBinding == nullptr)
			return nullptr;

		return static_cast<T*>(pBinding->pData);
	}

	T* operator->() const
	{
		if (pBinding == nullptr)
			return nullptr;

		return static_cast<T*>(pBinding->pData);
	}

	const Resource_t* pBinding;
};




#include <cstddef>
#include <cstdint>

#include "utlstring.h"
#include "cinterlockedint.hpp"

enum EResourceBindingFlags
{
	RESOURCE_BINDING_CACHED = 0x1,
	RESOURCE_BINDING_ERROR = 0x2,
	RESOURCE_BINDING_PERMANENT = 0x4,
	RESOURCE_BINDING_ANONYMOUS = 0x8,
};

struct ResourceBindingBase_t
{
	void* pData = nullptr;
	CUtlString* pResourceName = nullptr;
	std::uint32_t uFlags = 0;
	std::uint8_t uResourceType = -1;
	CInterlockedIntT< std::uint32_t > uRefCount = 0;
};

template< typename tResource >
struct ResourceBinding_t
	: public ResourceBindingBase_t
{
	template< class tValue >
	friend class CStrongHandle;

	template< class tValue >
	friend int ResourceAddRef( const ResourceBinding_t< tValue >* pResource ) noexcept;

	template< class tValue >
	friend int ResourceRelease( const ResourceBinding_t< tValue >* pResource ) noexcept;
};

template< class tValue >
int ResourceAddRef( const ResourceBinding_t< tValue >* pResource ) noexcept
{
	return ++pResource->uRefCount;
}

template< class tValue >
int ResourceRelease( const ResourceBinding_t< tValue >* pResource ) noexcept
{
	return --pResource->uRefCount;
}

using ResourceHandle_t = ResourceBindingBase_t*;

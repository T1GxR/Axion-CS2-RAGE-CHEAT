#ifndef CS2_CHEAT_CINTERLOCKEDINT_HPP
#define CS2_CHEAT_CINTERLOCKEDINT_HPP

#include <concepts>
#include <cstdint>

#include <intrin.h>

template< std::integral tElement >
requires( sizeof( tElement ) == sizeof( std::int32_t ) || sizeof( tElement ) == sizeof( std::int64_t ) )
class CInterlockedIntT
{
public:
	constexpr CInterlockedIntT( ) noexcept
		: m_iValue( 0 )
	{
	}

	constexpr CInterlockedIntT( tElement iValue ) noexcept
		: m_iValue( iValue )
	{
	}

	constexpr tElement operator( )( ) const noexcept
	{
		return m_iValue;
	}

	constexpr operator tElement( ) const noexcept
	{
		return m_iValue;
	}

	constexpr bool operator!( ) const noexcept
	{
		return !m_iValue;
	}

	tElement operator++( ) noexcept
	{
		if constexpr ( sizeof( tElement ) == sizeof( std::int32_t ) )
			return static_cast< tElement >( _InterlockedIncrement( reinterpret_cast< volatile long* >( &m_iValue ) ) );
		else
			return static_cast< tElement >( _InterlockedIncrement64( reinterpret_cast< volatile long long* >( &m_iValue ) ) );
	}

	tElement operator--( ) noexcept
	{
		if constexpr ( sizeof( tElement ) == sizeof( std::int32_t ) )
			return static_cast< tElement >( _InterlockedDecrement( reinterpret_cast< volatile long* >( &m_iValue ) ) );
		else
			return static_cast< tElement >( _InterlockedDecrement64( reinterpret_cast< volatile long long* >( &m_iValue ) ) );
	}

	tElement operator++( int ) noexcept
	{
		return operator++( ) - 1;
	}

	tElement operator--( int ) noexcept
	{
		return operator--( ) + 1;
	}

	[[nodiscard]] tElement operator+( tElement iOtherValue ) const noexcept
	{
		return m_iValue + iOtherValue;
	}

	[[nodiscard]] tElement operator-( tElement iOtherValue ) const noexcept
	{
		return m_iValue - iOtherValue;
	}

	void operator+=( tElement iAdd ) noexcept
	{
		if constexpr ( sizeof( tElement ) == sizeof( std::int32_t ) )
			return static_cast< tElement >( _InterlockedExchangeAdd( reinterpret_cast< volatile long* >( &m_iValue ), iAdd ) );
		else
			return static_cast< tElement >( _InterlockedExchangeAdd64( reinterpret_cast< volatile long* >( &m_iValue ), iAdd ) );
	}

	void operator-=( tElement iSub ) noexcept
	{
		if constexpr ( sizeof( tElement ) == sizeof( std::int32_t ) )
			return static_cast< tElement >( _InterlockedExchangeSub( reinterpret_cast< volatile long* >( &m_iValue ), iSub ) );
		else
			return static_cast< tElement >( _InterlockedExchangeSub64( reinterpret_cast< volatile long* >( &m_iValue ), iSub ) );
	}

	tElement operator=( tElement iNewValue ) noexcept
	{
		if constexpr ( sizeof( tElement ) == sizeof( std::int32_t ) )
			_InterlockedExchange( reinterpret_cast< volatile long* >( &m_iValue ), iNewValue );
		else
			_InterlockedExchange64( reinterpret_cast< volatile long long* >( &m_iValue ), iNewValue );

		return m_iValue;
	}

public:
	volatile tElement m_iValue;
};

using CInterlockedInt = CInterlockedIntT< std::int32_t >;
using CInterlockedUInt = CInterlockedIntT< std::uint32_t >;

#endif // CS2_CHEAT_CINTERLOCKEDINT_HPP

#ifndef CS2_CHEAT_MEMORY_HPP
#define CS2_CHEAT_MEMORY_HPP

#include <vector>
#include <cstdint>
#include <cstddef>
#include <optional>
#include <string_view>


namespace Core::Memory
{
	struct Address_t
	{
		constexpr Address_t() noexcept = default;
		constexpr ~Address_t() noexcept = default;

		explicit constexpr Address_t(std::uintptr_t uAddress) noexcept
			: m_uAddress(uAddress)
		{
		}

		explicit Address_t(const void* pAddress) noexcept
			: m_uAddress(reinterpret_cast<std::uintptr_t>(pAddress))
		{
		}

		constexpr operator std::uintptr_t() const noexcept
		{
			return m_uAddress;
		}

		constexpr operator bool() const noexcept
		{
			return m_uAddress;
		}

		operator void* () const noexcept
		{
			return reinterpret_cast<void*>(m_uAddress);
		}

		template< typename tReturn = Address_t >
		[[nodiscard]] constexpr tReturn Offset(std::ptrdiff_t iOffset) const noexcept
		{
			return tReturn(m_uAddress + iOffset);
		}

		template< typename tReturn = Address_t >
		[[nodiscard]] inline tReturn Get(std::uint32_t iDereferenceCount = 1) const noexcept
		{
			std::uintptr_t uOutAddress = m_uAddress;
			while (iDereferenceCount-- && uOutAddress)
				uOutAddress = *reinterpret_cast<std::uintptr_t*>(uOutAddress);

			return tReturn(uOutAddress);
		}

		template< typename tReturn = Address_t >
		[[nodiscard]] inline tReturn Jump(std::ptrdiff_t iOffset = 0x1) const noexcept
		{
			std::uintptr_t uOutAddress = m_uAddress + iOffset;

			uOutAddress += *reinterpret_cast<std::int32_t*>(uOutAddress); // @note / xnxkzeu: those could be negative.
			uOutAddress += 4;

			return tReturn(uOutAddress);
		}

		template< typename tReturn = std::uintptr_t >
		[[nodiscard]] constexpr tReturn Cast() const noexcept
		{
			return tReturn(m_uAddress);
		}
	private:
		std::uintptr_t m_uAddress = { };
	};

	[[nodiscard]] Address_t GetModuleBaseAddress(std::string_view szModuleName) noexcept;
	[[nodiscard]] Address_t GetModuleBaseAddress(std::wstring_view szModuleName) noexcept;

	[[nodiscard]] Address_t GetExportAddress(Address_t uModuleBaseAddress, std::string_view szProcedureName) noexcept;

	[[nodiscard]] bool GetSectionInfo(Address_t uModuleAddress, std::string_view szSectionName, Address_t* pOutSectionAddress, std::size_t* pOutSectionSize) noexcept;

	[[nodiscard]] Address_t FindPattern(std::string_view szModuleName, std::string_view szPattern) noexcept;
	[[nodiscard]] Address_t FindPattern(Address_t uRegionAddress, std::size_t uRegionSize, std::string_view szPattern) noexcept;

	[[nodiscard]] std::vector< std::optional< std::byte > > PatternToBytes(std::string_view szPattern) noexcept;

	template< typename tReturn, std::size_t uIndex, typename... tArgs >
	constexpr inline tReturn CallVFunc(void* pInstance, tArgs... argList) noexcept
	{
		using fnVirtual_t = tReturn(__thiscall*)(void*, tArgs...) noexcept;
		return (*static_cast<fnVirtual_t**>(pInstance))[uIndex](pInstance, argList...);
	}
} // namespace Core::Memory

#define VFUNC( uIndex, fnVirtual, tArgs, tReturn, ... )                                     \
	auto fnVirtual noexcept                                                                 \
	{                                                                                       \
		return Core::Memory::CallVFunc< tReturn __VA_OPT__(, ) __VA_ARGS__, uIndex > tArgs; \
	}

#define OFFSET( iOffset, fnVariable, tReturn, ... )                                                                                                                       \
	[[nodiscard]] std::add_lvalue_reference_t< tReturn __VA_OPT__(, ) __VA_ARGS__ > fnVariable( ) noexcept                                                                \
	{                                                                                                                                                                     \
		return *reinterpret_cast< std::add_pointer_t< tReturn __VA_OPT__(, ) __VA_ARGS__ > >( reinterpret_cast< std::uintptr_t >( this ) + iOffset );                     \
	}                                                                                                                                                                     \
	[[nodiscard]] std::add_lvalue_reference_t< std::add_const_t< tReturn __VA_OPT__(, ) __VA_ARGS__ > > fnVariable( ) const noexcept                                      \
	{                                                                                                                                                                     \
		return *reinterpret_cast< std::add_pointer_t< std::add_const_t< tReturn __VA_OPT__(, ) __VA_ARGS__ > > >( reinterpret_cast< std::uintptr_t >( this ) + iOffset ); \
	}

#endif // CS2_CHEAT_MEMORY_HPP
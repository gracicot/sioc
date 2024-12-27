#ifndef KANGARU5_DETAIL_CTTI_HPP
#define KANGARU5_DETAIL_CTTI_HPP

#include "concepts.hpp"
#include "murmur.hpp"

#include <string_view>
#include <cstddef>

#include "define.hpp"

namespace kangaru::detail::ctti {
	template<typename T>
	inline KANGARU5_CONSTEVAL auto raw_typed_signature() -> std::string_view {
		return KANGARU5_FUNCTION_SIGNATURE;
	}
	
	inline constexpr auto signature_prefix_length = std::size_t{raw_typed_signature<int>().find("int")};
	inline constexpr auto signature_postfix_length = std::size_t{raw_typed_signature<int>().size() - signature_prefix_length - std::string_view{"int"}.size()};
	
	static_assert(raw_typed_signature<int>().substr(signature_prefix_length, 3) == "int");
	
	static_assert(signature_prefix_length != std::string_view::npos, "Cannot find the type name in the function signature");
	
	// TODO: Get stable type name on all compilers
	template<typename T>
	inline KANGARU5_CONSTEVAL auto type_name_prefix_length() -> std::size_t {
		using namespace std::literals;
		auto const sig_prefix_trimmed = raw_typed_signature<T>().substr(signature_prefix_length);
		
		if (sig_prefix_trimmed.starts_with("class ")) {
			return signature_prefix_length + "class "sv.size();
		}
		
		if (sig_prefix_trimmed.starts_with("struct")) {
			return signature_prefix_length + "struct "sv.size();
		}
		
		return signature_prefix_length;
	}
	
	template<typename T>
	inline KANGARU5_CONSTEVAL auto type_name() -> std::string_view {
		auto const sig_prefix_trimmed = raw_typed_signature<T>().substr(type_name_prefix_length<T>());
		return sig_prefix_trimmed.substr(
			0,
			sig_prefix_trimmed.size() - signature_postfix_length
		);
	}
	
	// TODO: This cannot be private since it's needs to be in the public interface
	template<typename T>
	struct type_id_for_result : std::integral_constant<std::size_t, detail::murmur::murmur64a(type_name<T>())> {};
	
	template<typename T>
	inline KANGARU5_CONSTEVAL auto type_id_for() -> type_id_for_result<T> {
		return type_id_for_result<T>{};
	}
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_CTTI_HPP
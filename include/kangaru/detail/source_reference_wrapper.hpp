#ifndef KANGARU5_DETAIL_SOURCE_REFERENCE_WRAPPER_HPP
#define KANGARU5_DETAIL_SOURCE_REFERENCE_WRAPPER_HPP

#include "source.hpp"
#include "concepts.hpp"

#include <type_traits>

#include "define.hpp"

namespace kangaru {
	template<object Source> requires source<std::remove_const_t<Source>>
	struct source_reference_wrapper {
		explicit constexpr source_reference_wrapper(Source& source) noexcept : source{std::addressof(source)} {}
		
		template<injectable T>
		constexpr auto provide() const& -> T
		requires source_of<Source&, T> {
			return kangaru::provide<T>(*source);
		}
		
		[[nodiscard]]
		constexpr auto unwrap() const noexcept -> Source& {
			return *source;
		}
		
	private:
		Source* source;
	};
	
	template<source_ref Source>
	struct source_forwarding_reference_wrapper {
		explicit constexpr source_forwarding_reference_wrapper(Source source) noexcept : source{std::addressof(source)} {}
		
		template<injectable T>
		constexpr auto provide() const& -> T
		requires source_of<Source, T> {
			return kangaru::provide<T>(detail::utility::forward_like<Source>(*source));
		}
		
		[[nodiscard]]
		constexpr auto unwrap() const& noexcept -> Source {
			return detail::utility::forward_like<Source>(*source);
		}
		
	private:
		std::remove_reference_t<Source>* source;
	};
	
	template<forwarded_source Source>
	source_forwarding_reference_wrapper(Source&&) -> source_forwarding_reference_wrapper<Source&&>;
	
	template<typename T>
	concept reference_wrapper = source<T> and requires(T ref) {
		{ ref.unwrap() } -> reference;
	};
	
	template<typename T>
	concept forwarded_reference_wrapper = reference_wrapper<std::remove_reference_t<T>>;
	
	inline constexpr auto maybe_unwrap(forwarded_reference_wrapper auto&& ref) noexcept -> decltype(auto) {
		return KANGARU5_FWD(ref).unwrap();
	}
	
	inline constexpr auto maybe_unwrap(auto&& any) -> decltype(any) {
		return KANGARU5_FWD(any);
	}
	
	template<reference_wrapper Wrapper>
	using source_reference_wrapped_type = std::remove_reference_t<decltype(std::declval<Wrapper>().unwrap())>;
	
	template<typename MaybeWrapper>
	using maybe_wrapped_t = std::remove_reference_t<decltype(KANGARU5_NO_ADL(maybe_unwrap)(std::declval<MaybeWrapper>()))>;
	
	template<source Source> requires (not reference_wrapper<std::remove_cvref_t<Source>>)
	inline constexpr auto ref(Source& source) -> source_reference_wrapper<Source> {
		return source_reference_wrapper<Source>{source};
	}
	
	template<source Source> requires (not reference_wrapper<Source>)
	inline constexpr auto ref(source_reference_wrapper<Source> source) -> source_reference_wrapper<Source> {
		return source;
	}
	
	template<forwarded_source Source> requires (not reference_wrapper<std::remove_cvref_t<Source>>)
	inline constexpr auto fwd_ref(Source&& source) -> source_forwarding_reference_wrapper<Source&&> {
		return source_forwarding_reference_wrapper<Source&&>{KANGARU5_FWD(source)};
	}
	
	template<forwarded_source Source> requires (not reference_wrapper<std::remove_cvref_t<Source>>)
	inline constexpr auto fwd_ref(source_forwarding_reference_wrapper<Source> source) -> source_forwarding_reference_wrapper<Source&&> {
		return source;
	}
} // namespace kangaru

#include "undef.hpp"

#endif // KANGARU5_DETAIL_SOURCE_REFERENCE_WRAPPER_HPP
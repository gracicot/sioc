#pragma once

#include "../container.hpp"
#include "invoke.hpp"
#include "single.hpp"
#include "injected.hpp"

#include <type_traits>

namespace kgr {

template<typename... Args>
struct Dependency {};

namespace detail {

template<typename...>
struct Injector;

template<typename CRTP, typename... Deps>
struct Injector<CRTP, Dependency<Deps...>> {
	static decltype(auto) construct(Inject<Deps>... deps) {
		return CRTP::makeService(std::forward<Inject<Deps>>(deps).forward()...);
	}
};

} // detail

template<typename CRTP, typename Type, typename Deps>
struct GenericService : detail::Injector<CRTP, Deps> {
	friend struct Container;
	using Self = CRTP;
	
	GenericService() = default;
	
	GenericService(GenericService&& other) {
		if (other._initialized) {
			emplace(std::move(other.getInstance()));
		}
	}
	
	GenericService& operator=(GenericService&& other) {
		if (other._initialized) {
			emplace(std::move(other.getInstance()));
		}
		return *this;
	}
	
	GenericService(const GenericService& other) {
		if (other._initialized) {
			emplace(other.getInstance());
		}
	}
	
	GenericService& operator=(const GenericService& other) {
		if (other._initialized) {
			emplace(other.getInstance());
		}
		return *this;
	}
	
	template<typename... Args>
	GenericService(in_place_t, Args&&... args) {
		emplace(std::forward<Args>(args)...);
	}
	
	~GenericService() {
		if (_initialized) {
			getInstance().~Type();
		}
	}
	
protected:
	template<typename F, typename... T>
	void autocall(Inject<T>... others) {
		CRTP::call(getInstance(), F::value, std::forward<Inject<T>>(others).forward()...);
	}
	
	template<typename F, template<typename> class Map>
	void autocall(Inject<ContainerService> cs) {
		autocall<Map, F>(detail::tuple_seq<detail::function_arguments_t<typename F::value_type>>{}, std::move(cs));
	}
	
	Type& getInstance() {
		return *reinterpret_cast<Type*>(&_instance);
	}
	
	const Type& getInstance() const {
		return *reinterpret_cast<const Type*>(&_instance);
	}
	
private:
	template<typename... Args>
	void emplace(Args&&... args) {
		if (_initialized) {
			getInstance().~Type();
		}
		
		_initialized = true;
		new (&_instance) Type(std::forward<Args>(args)...);
	}
	
	template<template<typename> class Map, typename F, int... S>
	void autocall(detail::seq<S...>, Inject<ContainerService> cs) {
		cs.forward().invoke<Map>([this](detail::function_argument_t<S, typename F::value_type>... args){
			CRTP::call(getInstance(), F::value, std::forward<detail::function_argument_t<S, typename F::value_type>>(args)...);
		});
	}
	
	bool _initialized = false;
	typename std::aligned_storage<sizeof(Type), alignof(Type)>::type _instance;
};

} // namespace kgr

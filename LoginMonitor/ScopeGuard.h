#pragma once

#include <utility>

template<typename T, typename F>
struct ScopeGuard
{
    F func;
    T data{};
    ScopeGuard(F&& f) : func(std::forward<F>(f)) {}
    ~ScopeGuard() { func(data); }
};

template<typename T, typename F> ScopeGuard(F&& frv)->ScopeGuard<T, F>; //CTAD(C++17) 
//auto func = [] {};
//ScopeGuard x(func);       // No move, F& refers to func
//ScopeGuard y([] {});       // Lambda is moved to F.
//
//  Author
//      github.com/luncliff
//  Note
//      DLL loading + RAII
//
#pragma once
#ifndef PLUGIN_LOADER_H
#define PLUGIN_LOADER_H

#include <utility>

#if _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#elif __APPLE__ || __linux__ || __unix__
#include <dlfcn.h>
using HMODULE = void *;
constexpr auto GetProcAddress = ::dlsym;
constexpr auto FreeLibrary = ::dlclose;

#endif // Platform


// - Reference
//      Plugin in the Go Language
class plugin
{
  HMODULE handle = nullptr;

private:
  plugin(const plugin &) = delete;
  plugin &operator=(const plugin &) = delete;

  plugin(HMODULE _mod) noexcept : handle{_mod} {}

public:
  // Move semantic for RVO
  plugin(plugin &&rhs) noexcept { std::swap(*this, rhs); }
  plugin &operator=(plugin &&rhs) noexcept
  {
    std::swap(*this, rhs);
    return *this;
  }
  // Automatic close
  ~plugin() noexcept
  {
    if (*this)
      ::FreeLibrary(handle);
  }

public:
  operator bool() const noexcept { return handle != nullptr; }

  template <typename FuncType>
  auto lookup(const char *symbol) const noexcept -> FuncType *
  {
    // Casting with decay_t
    using ProcType = typename std::decay<FuncType>::type;
    return reinterpret_cast<ProcType>(lookup(symbol));
  }
  auto lookup(const char *symbol) const noexcept -> void *
  {
    return ::GetProcAddress(handle, symbol);
  }

public:
  static plugin load(const char *file) noexcept(false)
  {
#if _WIN32
    return ::LoadLibraryA(file);
#elif __APPLE__ || __linux__ || __unix__
    return ::dlopen(file, RTLD_NOW);
#endif
  }
};

#endif // PLUGIN_LOADER_H
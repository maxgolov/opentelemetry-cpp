#pragma once

#include <memory>
#include <type_traits>

#include "opentelemetry/version.h"

OPENTELEMETRY_BEGIN_NAMESPACE
namespace nostd
{
template <class Sig>
class function_ref;

/**
 * Non-owning function reference that can be used as a more performant
 * replacement for std::function when ownership sematics aren't needed.
 *
 * See http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0792r0.html
 *
 * Based off of https://stackoverflow.com/a/39087660/4447365
 */
template <class R, class... Args>
class function_ref<R(Args...)>
{
  void *callable_                = nullptr;
  R (*invoker_)(void *, Args...) = nullptr;

  template <class F>
  using FunctionPointer = decltype(std::addressof(std::declval<F &>()));

  template <class F>
  void BindTo(F &f) noexcept
  {
    callable_ = static_cast<void *>(std::addressof(f));
    invoker_  = [](void *callable_, Args... args) -> R {
      return (*static_cast<FunctionPointer<F>>(callable_))(std::forward<Args>(args)...);
    };
  }

  template <class R_in, class... Args_in>
  void BindTo(R_in (*f)(Args_in...)) noexcept
  {
    using F = decltype(f);
    if (f == nullptr)
    {
      return BindTo(nullptr);
    }
    callable_ = reinterpret_cast<void *>(f);
    invoker_  = [](void *callable_, Args... args) -> R {
      return (F(callable_))(std::forward<Args>(args)...);
    };
  }

  void BindTo(std::nullptr_t) noexcept
  {
    callable_ = nullptr;
    invoker_  = nullptr;
  }

public:
  template <
      class F,
      typename std::enable_if<!std::is_same<function_ref, typename std::decay<F>::type>::value,
                              int>::type = 0,
      typename std::enable_if<
          std::is_convertible<typename std::result_of<F &(Args...)>::type, R>::value,
          int>::type = 0>
  function_ref(F &&f)
  {
    BindTo(f);  // not forward
  }

  function_ref(std::nullptr_t) {}

  function_ref(const function_ref &) noexcept = default;
  function_ref(function_ref &&) noexcept      = default;

  R operator()(Args... args) const { return invoker_(callable_, std::forward<Args>(args)...); }

  explicit operator bool() const { return invoker_; }
};
}  // namespace nostd
OPENTELEMETRY_END_NAMESPACE

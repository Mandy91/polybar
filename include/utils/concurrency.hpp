#include <atomic>
#include <mutex>
#include <thread>

// Based on https://github.com/ademakov/Evenk

namespace concurrency
{
  struct NoBackoff
  {
    bool operator()()
    {
      return true;
    }
  };

  struct YieldBackoff
  {
    bool operator()()
    {
      std::this_thread::yield();
      return false;
    }
  };

  class SpinLock
  {
    std::atomic_flag lock_flag = ATOMIC_FLAG_INIT;

    public:
      SpinLock() = default;
      SpinLock(const SpinLock &) = delete;
      SpinLock &operator=(const SpinLock &) = delete;

      void lock() noexcept
      {
        lock(NoBackoff {});
      }

      template <typename Backoff>
      void lock(Backoff backoff) noexcept
      {
        while (lock_flag.test_and_set(std::memory_order_acquire)) {
          backoff();
        }
      }

      void unlock() noexcept
      {
        lock_flag.clear(std::memory_order_release);
      }
  };

  template<typename T>
  class Atomic
  {
    concurrency::SpinLock lock;
    std::atomic<T> value;

    public:
      Atomic() = default;
      Atomic(T init) {
        this->operator=(init);
      }

      void operator=(T value)
      {
        std::lock_guard<concurrency::SpinLock> lck(this->lock);
        this->value = value;
      }

      T operator()()
      {
        std::lock_guard<concurrency::SpinLock> lck(this->lock);
        return this->value;
      }

      operator bool()
      {
        std::lock_guard<concurrency::SpinLock> lck(this->lock);
        return this->value;
      }
  };

  template<typename T>
  class Value
  {
    concurrency::SpinLock lock;
    T value;

    public:
      Value() = default;
      Value(T init) {
        this->operator=(init);
      }

      void operator=(T value)
      {
        std::lock_guard<concurrency::SpinLock> lck(this->lock);
        this->value = value;
      }

      T operator()()
      {
        std::lock_guard<concurrency::SpinLock> lck(this->lock);
        return this->value;
      }

      operator bool()
      {
        std::lock_guard<concurrency::SpinLock> lck(this->lock);
        return this->value;
      }
  };
}


#pragma once

#include <mutex>

namespace voya
{
#define MUTEX_LOCK_SCOPE(_Mutex) std::lock_guard<std::mutex> _Mutex##Lock__(_Mutex)
}


#pragma once

#include "base/CCConsole.h"
#include "base/ccMacros.h"

namespace voya
{

#define voya_assert(_Bool, _Info) CCASSERT(_Bool, _Info)
#define voya_log(format, ...) cocos2d::log(format,  ##__VA_ARGS__)
#define voya_log_warning(format, ...) cocos2d::log(format,  ##__VA_ARGS__)

// #define voya_assert(_Bool, _Info) 
// #define voya_log(format, ...) 
// #define voya_log_warning(format, ...) 

}
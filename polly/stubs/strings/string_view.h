#pragma once

#include "stubs/base/config.h"

#if !defined(POLLY_HAVE_STD_STRING_VIEW)
#include "stubs/strings/internal/string_view.h"
#else
namespace polly {
using std::string_view;
} // namespace polly
#endif
#pragma once

#include <sncore/api_common.h>

#if defined(SN_FILE_STATIC)
    #define SN_FILE_API
#elif defined(SN_EXPORT)
    #define SN_FILE_API SN_API_HELPER_EXPORT
#else
    #define SN_FILE_API SN_API_HELPER_IMPORT
#endif

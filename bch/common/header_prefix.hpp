/**
Copyright: Jesper Storm Bache (bache.name)
*/

// Alignment prefix for bch headers: Set data alignment to be natural alignment

#include "bch/common/compiler_settings.hpp"

#if BCH_PRAGMA_PACK_SUPPORT

#if BCH_64_BIT
#pragma pack(push, bch_header, 16)
#else
#pragma pack(push, bch_header, 8)
#endif

#endif

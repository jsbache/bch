/**
Copyright: Jesper Storm Bache (bache.name)
*/

#ifndef BCH_COMPILER_SETTINGS_HPP

// -----------------------------------------------------------------------------
// Compiler identification

#if defined __clang__
#define BCH_PRAGMA_PACK_SUPPORT     1
#define BCH_64_BIT                  1
#endif


// -----------------------------------------------------------------------------

// Default values

#ifndef BCH_PRAGMA_PACK_SUPPORT
#define BCH_PRAGMA_PACK_SUPPORT     0
#endif

#ifndef BCH_64_BIT
#define BCH_64_BIT                  1
#endif

#endif  // BCH_COMPILER_SETTINGS_HPP

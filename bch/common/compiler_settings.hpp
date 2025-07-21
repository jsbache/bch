/**
Copyright: Jesper Storm Bache (bache.name)
*/

#ifndef BCH_COMPILER_SETTINGS_HPP
#define BCH_COMPILER_SETTINGS_HPP

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

// -----------------------------------------------------------------------------

/* Preprocessor defines that are used by the smart-pointer implementation.
General pattern is that the project that use the functionality use the _ENABLE
variation, and we then calculate necessary preprocessor symbols based n this.
We rely on DEBUG being defined in debug builds.
BCH_SMART_PTR_DEBUG_ENABLE can be defined to 1 to add debug tests
BCH_SMART_PTR_UNITTEST_ENABLE can be defined to 1 to add code that is needed by
    unit tests
*/
#ifdef BCH_SMART_PTR_DEBUG
#error "BCH_SMART_PTR_DEBUG_ENABLE should be used rather than BCH_SMART_PTR_DEBUG"
#endif
#ifdef BCH_SMART_PTR_UNITTEST
#error "BCH_SMART_PTR_UNITTEST_ENABLE should be used rather than BCH_SMART_PTR_UNITTEST"
#endif

#ifndef BCH_SMART_PTR_DEBUG_ENABLE
#define BCH_SMART_PTR_DEBUG_ENABLE 0
#endif

#if BCH_SMART_PTR_DEBUG_ENABLE
    #define BCH_SMART_PTR_DEBUG 1

    #ifndef BCH_SMART_PTR_UNITTEST_ENABLE
    #define BCH_SMART_PTR_UNITTEST_ENABLE 0
    #endif

    #define BCH_SMART_PTR_UNITTEST BCH_SMART_PTR_UNITTEST_ENABLE

#else
    #define BCH_SMART_PTR_DEBUG 0
    #define BCH_SMART_PTR_UNITTEST 0
#endif

#endif  // BCH_COMPILER_SETTINGS_HPP

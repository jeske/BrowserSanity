/**
 * @file nuklear_impl.cpp
 * @brief Nuklear implementation file - defines NK_IMPLEMENTATION once for the entire project
 */

// Define implementation macros before including nuklear
#define NK_IMPLEMENTATION
#define NK_GDI_IMPLEMENTATION

// Include the same defines as NKWindow.h
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT

#include <nuklear.h>
//
// Created by vitor on 6/29/26.
//

#ifndef MELONDS_XXHASH_LEGACY_H
#define MELONDS_XXHASH_LEGACY_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint64_t KHLEG_XXH3_64bits(const void* data, size_t len);

#ifdef __cplusplus
}
#endif

#endif //MELONDS_XXHASH_LEGACY_H

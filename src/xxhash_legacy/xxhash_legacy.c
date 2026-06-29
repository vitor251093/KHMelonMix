//
// Created by vitor on 6/29/26.
//

#include "xxhash_legacy.h"
#include "xxhash.h"

uint64_t KHLEG_XXH3_64bits(const void* data, size_t len)
{
    return XXH3_64bits(data, len);
}

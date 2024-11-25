#ifndef __ANDROID_FOPEN_H__
#define __ANDROID_FOPEN_H__

#include <stdio.h>
#include <android/asset_manager.h>

#ifdef __cplusplus
extern "C" {
#endif

    FILE *android_fopen(AAssetManager* android_asset_manager, const char *fname, const char *mode);

#ifdef __cplusplus
}
#endif

#endif // __ANDROID_FOPEN_H__
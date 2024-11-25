#ifndef ANDROIDCAMERAHANDLER_H
#define ANDROIDCAMERAHANDLER_H

#include "../types.h"

namespace MelonDSAndroid
{
    class AndroidCameraHandler {
    public:
        virtual void startCamera(int camera) = 0;
        virtual void stopCamera(int camera) = 0;
        virtual void captureFrame(int camera, u32* frameBuffer, int width, int height, bool isYuv) = 0;
        virtual ~AndroidCameraHandler() {};
    };
}

#endif //ANDROIDCAMERAHANDLER_H

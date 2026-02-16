#pragma once 

#include <iostream>
#include "batteries/opengl.h"


namespace bob 
{
    struct Framebuffer
    {
        // frame buffer object
        unsigned int fbo; 
        unsigned int colorBuffer[8];
        unsigned int depthBuffer;
        unsigned int width;
        unsigned int height;
    };

    Framebuffer createFramebuffer(unsigned int width, unsigned int height, int colorFormat, unsigned int depthBuffer);

    void deleteFramebuffer(Framebuffer bufferToDelete); 
       
}
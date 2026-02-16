#include "framebuffer.h"


namespace bob
{

    Framebuffer createFramebuffer(unsigned int width, unsigned int height, int colorFormat, unsigned int depthBuffer)
    {

        Framebuffer frameBuffer; 

        unsigned int fboTexture;
        unsigned int tempFbo = frameBuffer.fbo;
        frameBuffer.depthBuffer = depthBuffer;
   
        // create our buffer obejct
        glGenFramebuffers(1, &frameBuffer.fbo);
        //glCreateFramebuffers(1, &tempFbo);
        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer.fbo);


        // create a texture for it 
        glGenTextures(1, &fboTexture);
        glBindTexture(GL_TEXTURE_2D, fboTexture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


        // make a texture for the depth buffer
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 800, 600, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthBuffer, 0);  

        // create our color buffer
        glGenTextures(1, &frameBuffer.colorBuffer[0]);
        glBindTexture(GL_TEXTURE_2D, frameBuffer.colorBuffer[0]);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);

        // attach our color buffer to our frame buffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frameBuffer.colorBuffer[0], 0);

        // make our depth buffer
        glGenRenderbuffers(1, &frameBuffer.depthBuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, frameBuffer.depthBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, frameBuffer.depthBuffer);
        //glFrameBufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT0, GL_TEXTURE_2D, depthBuffer, 0);


        // make it our defualt buffer that will be rendered to
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // done after main attachments are added to the frame buffer
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
        {
            std::cout << "Sucessfully loaded the framebuffer!";
            return frameBuffer;
        }
        else
        {
            std::cout << "Frame buffer is not complete "; 
        }

        return frameBuffer;
    }

    void deleteFrameBuffer(Framebuffer bufferToDelete)
    {
         // delete when were all done
        glDeleteFramebuffers(1, &bufferToDelete.fbo);
    }
};
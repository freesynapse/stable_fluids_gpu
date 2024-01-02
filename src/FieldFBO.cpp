
#include "FieldFBO.h"

#include <synapse/Renderer>


FieldFBO::FieldFBO(const ColorFormat& _format, 
                   const glm::ivec2& _size, 
                   const std::string& _name)
{
    m_format = _format;
    m_pxFmt = getOpenGLPixelFormat(m_format);

    m_colorAttachmentCount = 1;
    m_colorAttachmentID = new GLuint[m_colorAttachmentCount];
    m_hasDepthAttachment = false;

    m_name = _name;

    // create the framebuffer
    resize(_size);

}

//---------------------------------------------------------------------------------------
void FieldFBO::resize(const glm::ivec2& _size)
{
    if (_size.x == m_size.x && _size.y == m_size.y)
        return;
    else
        m_size = _size;

    SYN_RENDER_S0({
        if (self->m_framebufferID)
        {
            glDeleteFramebuffers(1, &self->m_framebufferID);
            glDeleteTextures(self->m_colorAttachmentCount, self->m_colorAttachmentID);
            glDeleteTextures(1, &self->m_depthAttachmentID);
        }

        glGenFramebuffers(1, &self->m_framebufferID);

        glBindFramebuffer(GL_FRAMEBUFFER, self->m_framebufferID);

        // generate texture(s) as rendering target
        glGenTextures(self->m_colorAttachmentCount, self->m_colorAttachmentID);
        GLenum drawBufferIDs[self->m_colorAttachmentCount];

        for (size_t i = 0; i < self->m_colorAttachmentCount; i++)
        {
            glBindTexture(GL_TEXTURE_2D, self->m_colorAttachmentID[i]);

            // set texture with correct formats and type based on the FramebufferFormat specification.
            glTexImage2D(GL_TEXTURE_2D, 
                0, 
                self->m_pxFmt.internalFormat, 
                self->m_size.x, 
                self->m_size.y, 
                0, 
                self->m_pxFmt.storageFormat, 
                self->m_pxFmt.storageType, 
                NULL);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            // attach the texture to the framebuffer
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, self->m_colorAttachmentID[i], 0);
            drawBufferIDs[i] = GL_COLOR_ATTACHMENT0 + i;
        }

        glDrawBuffers(self->m_colorAttachmentCount, drawBufferIDs);

        // check for completeness
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            SYN_CORE_FATAL_ERROR("Framebuffer not complete.");
        }

        #ifdef DEBUG_FRAMEBUFFER
            SYN_CORE_TRACE("Framebuffer '", self->m_name.c_str(), "' (renderTargets=",
                            self->m_colorAttachmentCount, " [", self->m_size.x, " x ", 
                            self->m_size.y, " x ", getPixelFmtChannels(m_format) ,
                            "]) created.");
        #endif

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    });
}

//---------------------------------------------------------------------------------------
std::pair<glm::vec4, glm::vec4> FieldFBO::range(int _channels)
{

    glm::vec4 buffer[fieldSize()];
    readFieldData((void*)&buffer[0]);
    
    // int dim = 2; // how many channels? for this max is 2 (2d vector field)
    int dim = (_channels >= 1 && _channels <= 2 ? _channels : 1);
    
    glm::vec4 min(std::numeric_limits<float>::max());
    glm::vec4 max(std::numeric_limits<float>::min());

    for (int y = 0; y < m_size.y; y++)
    {
        for (int x = 0; x < m_size.x; x++)
        {
            const glm::vec4 &px = buffer[y * m_size.x + x];
            for (int i = 0; i < dim; i++)
            {
                min[i] = std::min(min[i], px[i]);
                max[i] = std::max(max[i], px[i]);
            }

        }

    }

    return { min, max };

}

//---------------------------------------------------------------------------------------
void FieldFBO::readFieldData(void *_buffer)
{
    // inhibit clamping of values to [0..1]
    glClampColor(GL_CLAMP_READ_COLOR, GL_FALSE);
    
    // bind before reading
    this->bind(false, true);

    // read into buffer
    glReadPixels(0, 0, m_size.x, m_size.y, m_pxFmt.storageFormat, GL_FLOAT, _buffer);

    // unbind
    this->unbind();

}





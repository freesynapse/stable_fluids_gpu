#pragma once

#include <glad/glad.h>
#include <stdio.h>

//
class Quad
{
public:

    //
    ~Quad() 
    {
        if (s_initialized)
        {
            glDeleteVertexArrays(1, &s_vao);
            glDeleteBuffers(1, &s_vbo);
        }

    }

    //
    static void bind() 
    { 
        if (!s_initialized) 
            initialize(); 
        glBindVertexArray(s_vao); 
    
    }
    
    //
    static void render() { glDrawArrays(GL_TRIANGLES, 0, 6); }

private:

    //
    static void initialize()
    {
        constexpr float vertices[] =
        {
                // xy         // uv
            -1.0f, -1.0f,  0.0f, 0.0f,
             1.0f, -1.0f,  1.0f, 0.0f,
             1.0f,  1.0f,  1.0f, 1.0f,

            -1.0f, -1.0f,  0.0f, 0.0f,
             1.0f,  1.0f,  1.0f, 1.0f,
            -1.0f,  1.0f,  0.0f, 1.0f
        };

        glGenVertexArrays(1, &s_vao);
        glGenBuffers(1, &s_vbo);
        glBindVertexArray(s_vao);
        glBindBuffer(GL_ARRAY_BUFFER, s_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        
        s_initialized = true;

    }

    //
    inline static GLuint s_vbo, s_vao;
    inline static bool s_initialized = false;

};



// This file is part of snark, a generic and flexible library for robotics research
// Copyright (c) 2016 The University of Sydney
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. Neither the name of the University of Sydney nor the
//    names of its contributors may be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
// NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
// GRANTED BY THIS LICENSE.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
// HOLDERS AND CONTRIBUTORS \"AS IS\" AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
// BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
// IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once

#include "types.h"
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>

namespace snark { namespace graphics { namespace qt3d { namespace gl {

// class shape;    
// class shape_container
// {
// public:
//     std::vector<std::shared_ptr<shape>> shapes;
//     //set GL context and bind voa, ?optionaly synchronize with paint
//     virtual void begin_update()=0;
//     virtual void end_update()=0;
// protected:
//     void init();
//     void paint();
//     void destroy();
// };
class shape : protected QOpenGLFunctions
{
protected:
    QOpenGLVertexArrayObject vao;
    QOpenGLBuffer vbo;
    friend class widget;
public:
    shape(GLenum mode=GL_POINTS);
    virtual ~shape();
    //this can only be called between begin_update and end_update
    //size: size of array e.g. update(v.data(),v.size())
    virtual void update(const vertex_t* data, std::size_t size);    //write data to internal buffer
protected:
    //GL context should be set and voa bound for these functions by caller (i.e. gl_widget)
    virtual void init();    //create buffer
    virtual void paint();  //invoke glDraw*
    virtual void destroy();   //destroy buffer
    GLenum mode;
    std::size_t size_;
private:
};

namespace shapes {
    
class point : public shape
{
public:
    float point_size;
    point(float point_size=1);
protected:
    void paint();
};

class lines : public shape
{
public:
    lines();
protected:
    void paint();
};

class line_strip : public shape
{
public:
    line_strip();
protected:
    void paint();
};

class line_loop : public shape
{
public:
    line_loop();
protected:
    void paint();
};

} // namespace shapes {

} } } } // namespace snark { namespace graphics { namespace qt3d { namespace gl {
    

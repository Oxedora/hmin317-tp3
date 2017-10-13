/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mainwidget.h"

#include <QMouseEvent>
#include <QKeyEvent>
#include <Qt>

#include <math.h>
#include <ctime>
#include <iostream>

qreal MainWidget::angularSpeed = 0.1f;
int MainWidget::maxFps = 0;
QQuaternion MainWidget::rotation;

MainWidget::MainWidget(QWidget *parent, int fps) :
    QOpenGLWidget(parent),
    fps(fps),
    geometries(0),
    texture(0),
    position(0.0, 0.0, -50.0)
{
    if (maxFps < fps)
        maxFps = fps;
}

MainWidget::~MainWidget()
{
    // Make sure the context is current when deleting the texture
    // and the buffers.
    makeCurrent();
    delete texture;
    delete geometries;
    doneCurrent();
}

void MainWidget::updateGeometry()
{
    geometries->updateSeason();
}

//! [0]
void MainWidget::mousePressEvent(QMouseEvent *e)
{
}

void MainWidget::mouseReleaseEvent(QMouseEvent *e)
{
}

// Adding arrows pressed to the list of directions the camera will have to go
void MainWidget::keyPressEvent(QKeyEvent *e)
{
    cameraDirections.push_back(e->key());
}

// Deleting the relevant key when released
void MainWidget::keyReleaseEvent(QKeyEvent *e) {
    for (unsigned int i = 0; i < cameraDirections.size(); i++) {
        if (cameraDirections[i] == e->key()) {
            cameraDirections.erase(cameraDirections.begin() + i);
            break;
        }
    }
}


void MainWidget::wheelEvent(QWheelEvent *e)
{
    if(e->phase() == Qt::ScrollUpdate)
    {
        wheelDirection = (e->delta() > 0 ? 1 : -1);
    }
}


//! [0]

//! [1]
void MainWidget::timerEvent(QTimerEvent *)
{
    // Update rotation
    if(fps == maxFps)
        rotation = QQuaternion::fromAxisAndAngle(QVector3D(0.0, 0.0, 1.0), angularSpeed * elapsedTimer.elapsed()) * rotation;

    const float speed = 0.03;
    const float scrollSpeed = 0.0005;
    for (unsigned int i = 0; i < cameraDirections.size(); i++) {
        switch (cameraDirections[i]) {
            case Qt::Key_Left:
                angularSpeed += speed;
                break;
            case Qt::Key_Right:
                angularSpeed -= speed;
                break;
        }
    }

    emit askCalendrier();

    elapsedTimer.restart();
    // Request an update
    update();

}
//! [1]

void MainWidget::initializeGL()
{
    initializeOpenGLFunctions();

    glClearColor(0, 0, 0, 1);

    initShaders();
    initTextures();

//! [2]
    // Enable depth buffer
    glEnable(GL_DEPTH_TEST);

    // Enable back face culling
    glEnable(GL_CULL_FACE);
//! [2]

    geometries = new GeometryEngine(std::rand()%3);

    // Use QBasicTimer because its faster than QTimer
    timer.start(1000/fps, this);

    elapsedTimer.start();
}

//! [3]
void MainWidget::initShaders()
{
    // Compile vertex shader
    if (!program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vshader.glsl"))
        close();

    // Compile fragment shader
    if (!program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/fshader.glsl"))
        close();

    // Link shader pipeline
    if (!program.link())
        close();

    // Bind shader pipeline for use
    if (!program.bind())
        close();
}
//! [3]

//! [4]
void MainWidget::initTextures()
{
    // Load cube.png image
    texture = new QOpenGLTexture(QImage(":/heightmap-1.png"));

    // Set nearest filtering mode for texture minification
    texture->setMinificationFilter(QOpenGLTexture::Nearest);

    // Set bilinear filtering mode for texture magnification
    texture->setMagnificationFilter(QOpenGLTexture::Linear);

    // Wrap texture coordinates by repeating
    // f.ex. texture coordinate (1.1, 1.2) is same as (0.1, 0.2)
    texture->setWrapMode(QOpenGLTexture::Repeat);

}
//! [4]

//! [5]
void MainWidget::resizeGL(int w, int h)
{
    // Calculate aspect ratio
    qreal aspect = qreal(w) / qreal(h ? h : 1);

    // Set near plane to 3.0, far plane to 7.0, field of view 45 degrees
    const qreal zNear = 1.0, zFar = 1000.0, fov = 45.0;

    // Reset projection
    projection.setToIdentity();

    // Set perspective projection
    projection.perspective(fov, aspect, zNear, zFar);
}
//! [5]

void MainWidget::paintGL()
{
    // Clear color and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    texture->bind();

//! [6]
    // Calculate model view transformation
    QMatrix4x4 matrix;

    matrix.translate(position);

    matrix.rotate(QQuaternion::fromAxisAndAngle(QVector3D(1,0,0), -45.0));

    matrix.rotate(rotation);

    // Set modelview-projection matrix
    program.setUniformValue("mvp_matrix", projection * matrix);
//! [6]

    // Use texture unit 0 which contains cube.png
    program.setUniformValue("texture", 0);

    // Draw cube geometry
//    geometries->drawCubeGeometry(&program);
    geometries->drawPlaneGeometry(&program);
}

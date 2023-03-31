#include "mygl.h"
#include <glm_includes.h>

#include <iostream>
#include <QApplication>
#include <QKeyEvent>
#include <thread>

#include "algo/perlin.h"
#include "scene/biome.h"
#include "scene/structure.h"
#include <QDateTime>

MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
      m_worldAxes(this),
      m_progLambert(this), m_progFlat(this), m_progInstanced(this),
      m_terrain(this), m_player(glm::vec3(48.f, 129.f, 48.f), m_terrain),
      ip("localhost"), time(0),
      m_currentMSecsSinceEpoch(QDateTime::currentMSecsSinceEpoch()),
      m_mousePosPrev(0)
{
    // Connect the timer to a function so that when the timer ticks the function is executed
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(tick()));
}

//move stuff from constructor to here so that it can be called upon entering game scene
void MyGL::start() {
    setFocusPolicy(Qt::ClickFocus);

    setMouseTracking(true); // MyGL will track the mouse's movements even if a mouse button is not pressed
    setCursor(Qt::BlankCursor); // Make the cursor invisible

    //distribution tests
    //distTest();
    //biomeDist();
    //TO DO: uncomment the bottom out to initialize spawn chunks before player loads in
    //m_terrain.createInitScene();

    // Tell the timer to redraw 60 times per second
    m_timer.start(16);
}

MyGL::~MyGL() {
    makeCurrent();
    glDeleteVertexArrays(1, &vao);
}


void MyGL::moveMouseToCenter() {
    QCursor::setPos(this->mapToGlobal(QPoint(width() / 2, height() / 2)));
}

void MyGL::initializeGL()
{
    // Create an OpenGL context using Qt's QOpenGLFunctions_3_2_Core class
    // If you were programming in a non-Qt context you might use GLEW (GL Extension Wrangler)instead
    initializeOpenGLFunctions();
    // Print out some information about the current OpenGL context
    debugContextVersion();

    // Set a few settings/modes in OpenGL rendering
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    // Set the color with which the screen is filled at the start of each render call.
    glClearColor(0.37f, 0.74f, 1.0f, 1);

    printGLErrorLog();

    // Create a Vertex Attribute Object
    glGenVertexArrays(1, &vao);

    //Create the instance of the world axes
    m_worldAxes.createVBOdata();

    // Create and set up the diffuse shader
    m_progLambert.create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");
    // Create and set up the flat lighting shader
    m_progFlat.create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");
    m_progInstanced.create(":/glsl/instanced.vert.glsl", ":/glsl/lambert.frag.glsl");

    // Set a color with which to draw geometry.
    // This will ultimately not be used when you change
    // your program to render Chunks with vertex colors
    // and UV coordinates
    m_progLambert.setGeometryColor(glm::vec4(0,1,0,1));

    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.
    glBindVertexArray(vao);
}

void MyGL::resizeGL(int w, int h) {
    //This code sets the concatenated view and perspective projection matrices used for
    //our scene's camera view.
    m_player.setCameraWidthHeight(static_cast<unsigned int>(w), static_cast<unsigned int>(h));
    glm::mat4 viewproj = m_player.mcr_camera.getViewProj();

    // Upload the view-projection matrix to our shaders (i.e. onto the graphics card)

    m_progLambert.setViewProjMatrix(viewproj);
    m_progFlat.setViewProjMatrix(viewproj);

    printGLErrorLog();
}


// MyGL's constructor links tick() to a timer that fires 60 times per second.
// We're treating MyGL as our game engine class, so we're going to perform
// all per-frame actions here, such as performing physics updates on all
// entities in the scene.

void MyGL::tick() {
    time++;
    long long ct = QDateTime::currentMSecsSinceEpoch();
    float dt = 0.001 * (ct - m_currentMSecsSinceEpoch);
    m_currentMSecsSinceEpoch = ct;

    m_player.tick(dt, m_inputs);

    update(); // Calls paintGL() as part of a larger QOpenGLWidget pipeline
    sendPlayerDataToGUI(); // Updates the info in the secondary window displaying player data
    //generates chunks based on player position
    //zone player is in
    int minx = floor(m_player.mcr_position.x/64)*64;
    int miny = floor(m_player.mcr_position.z/64)*64;

    //does rendering stuff
    for(int dx = minx-128; dx <= minx+128; dx+=64) {
        for(int dy = miny-128; dy <= miny+128; dy+=64) {
            if(m_terrain.m_generatedTerrain.find(toKey(dx, dy)) == m_terrain.m_generatedTerrain.end()){
                m_terrain.m_generatedTerrain.insert(toKey(dx, dy));
                for(int ddx = dx; ddx < dx + 64; ddx+=16) {
                    for(int ddy = dy; ddy < dy + 64; ddy+=16) {
                        //qDebug() << "creating ground for " << ddx << ddy;
                        m_terrain.createGroundThread(glm::vec2(ddx, ddy));
                    }
                }
            }
        }
    }

    //checks for additional structures for rendering, but not as often since structure threads can finish at staggered times
    if(time%30 == 0) {
        for(int dx = minx-128; dx <= minx+192; dx+=64) {
            for(int dy = miny-128; dy <= miny+192; dy+=64) {
                for(int ddx = dx; ddx < dx + 64; ddx+=16) {
                    for(int ddy = dy; ddy < dy + 64; ddy+=16) {
                        if(m_terrain.hasChunkAt(ddx, ddy)){
                            Chunk* c = m_terrain.getChunkAt(ddx, ddy).get();
                            if(c->dataGen && c->blocksChanged){
                                c->blocksChanged = false;
                                m_terrain.createVBOThread(c);
                            }
                        }
                    }
                }

            }
        }
    }
}

void MyGL::sendPlayerDataToGUI() const {
    emit sig_sendPlayerPos(m_player.posAsQString());
    emit sig_sendPlayerVel(m_player.velAsQString());
    emit sig_sendPlayerAcc(m_player.accAsQString());
    emit sig_sendPlayerLook(m_player.lookAsQString());
    glm::vec2 pPos(m_player.mcr_position.x, m_player.mcr_position.z);
    glm::ivec2 chunk(16 * glm::ivec2(glm::floor(pPos / 16.f)));
    glm::ivec2 zone(64 * glm::ivec2(glm::floor(pPos / 64.f)));
    emit sig_sendPlayerChunk(QString::fromStdString("( " + std::to_string(chunk.x) + ", " + std::to_string(chunk.y) + " )"));
    emit sig_sendPlayerTerrainZone(QString::fromStdString("( " + std::to_string(zone.x) + ", " + std::to_string(zone.y) + " )"));
}

// This function is called whenever update() is called.
// MyGL's constructor links update() to a timer that fires 60 times per second,
// so paintGL() called at a rate of 60 frames per second.
void MyGL::paintGL() {
    // Clear the screen so that we only see newly drawn images
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_progFlat.setViewProjMatrix(m_player.mcr_camera.getViewProj());
    m_progLambert.setViewProjMatrix(m_player.mcr_camera.getViewProj());
    m_progInstanced.setViewProjMatrix(m_player.mcr_camera.getViewProj());

    renderTerrain();

    glDisable(GL_DEPTH_TEST);
    m_progFlat.setModelMatrix(glm::mat4());
    m_progFlat.setViewProjMatrix(m_player.mcr_camera.getViewProj());
    m_progFlat.draw(m_worldAxes);
    glEnable(GL_DEPTH_TEST);
}

// TODO: Change this so it renders the nine zones of generated
// terrain that surround the player (refer to Terrain::m_generatedTerrain
// for more info)

void MyGL::renderTerrain() {
    //chunk player is in
    int renderDist = 512;
    float x = floor(m_player.mcr_position.x/16.f)*16;
    float y = floor(m_player.mcr_position.z/16.f)*16;

    m_terrain.draw(x-renderDist, x+renderDist, y-renderDist, y+renderDist, &m_progLambert);
    //m_terrain.draw(0, 1024, 0, 1024, &m_progInstanced);
}


void MyGL::keyPressEvent(QKeyEvent *e) {
    float amount = 2.0f;
    if(e->modifiers() & Qt::ShiftModifier){
        amount = 10.0f;
    }
    // http://doc.qt.io/qt-5/qt.html#Key-enum
    // This could all be much more efficient if a switch
    // statement were used, but I really dislike their
    // syntax so I chose to be lazy and use a long
    // chain of if statements instead
    if (e->key() == Qt::Key_Escape) {
        QApplication::quit();
    } else if (e->key() == Qt::Key_Right) {
        m_player.rotateOnUpGlobal(-amount);
    } else if (e->key() == Qt::Key_Left) {
        m_player.rotateOnUpGlobal(amount);
    } else if (e->key() == Qt::Key_Up) {
        m_player.rotateOnRightLocal(-amount);
    } else if (e->key() == Qt::Key_Down) {
        m_player.rotateOnRightLocal(amount);
    } else if (e->key() == Qt::Key_W) {
        m_inputs.wPressed = true;
    } else if (e->key() == Qt::Key_S) {
        m_inputs.sPressed = true;
    } else if (e->key() == Qt::Key_D) {
        m_inputs.dPressed = true;
    } else if (e->key() == Qt::Key_A) {
        m_inputs.aPressed = true;
    } else if (e->key() == Qt::Key_Q) {
        m_inputs.qPressed = true;
    } else if (e->key() == Qt::Key_E) {
        m_inputs.ePressed = true;
    } else if (e->key() == Qt::Key_F) {
        m_inputs.fPressed = true;
    } else if (e->key() ==Qt::Key_Space) {
        m_inputs.spacePressed = true;
    }
}

void MyGL::keyReleaseEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_W) {
        m_inputs.wPressed = false;
    } else if (e->key() == Qt::Key_S) {
        m_inputs.sPressed = false;
    } else if (e->key() == Qt::Key_D) {
        m_inputs.dPressed = false;
    } else if (e->key() == Qt::Key_A) {
        m_inputs.aPressed = false;
    } else if (e->key() == Qt::Key_Q) {
        m_inputs.qPressed = false;
    } else if (e->key() == Qt::Key_E) {
        m_inputs.ePressed = false;
    } else if (e->key() == Qt::Key_F) {
        m_inputs.fPressed = false;
    } else if (e->key() ==Qt::Key_Space) {
        m_inputs.spacePressed = false;
    }
}

void MyGL::mouseMoveEvent(QMouseEvent *e) {
    // TODO
    glm::vec2 pos(e->pos().x(), e->pos().y());
    if(e->buttons() & Qt::LeftButton)
    {
        // Rotation
        glm::vec2 diff = 0.2f * (pos - m_mousePosPrev);
        m_mousePosPrev = pos;
        m_inputs.mouseX = diff.x;
        m_inputs.mouseY = diff.y;
    }
}

void MyGL::mousePressEvent(QMouseEvent *e) {
    // TODO
    if(e->buttons() & Qt::LeftButton)
    {
        m_mousePosPrev = glm::vec2(e->pos().x(), e->pos().y());

        glm::vec3 cam_pos = m_player.mcr_camera.mcr_position;
        glm::vec3 ray_dir = m_player.getLook() * 3.f;

        float dist;
        glm::ivec3 block_pos;

        if (m_terrain.gridMarch(cam_pos, ray_dir, &dist, &block_pos)) {
            m_terrain.setBlockAt(block_pos.x, block_pos.y, block_pos.z, EMPTY);
        }
    }

    if (e->buttons() & Qt::RightButton) {
        glm::vec3 cam_pos = m_player.mcr_camera.mcr_position;
        glm::vec3 ray_dir = m_player.getLook() * 3.f;

        float dist;
        glm::ivec3 block_pos;

        if (m_terrain.gridMarch(cam_pos, ray_dir, &dist, &block_pos)) {
            BlockType b = m_terrain.getBlockAt(glm::vec3(block_pos));
            if (m_terrain.getBlockAt(glm::vec3(block_pos.x, block_pos.y, block_pos.z-1)) == EMPTY) {
                m_terrain.setBlockAt(block_pos.x, block_pos.y, block_pos.z-1, b);
            } else {
                if (m_terrain.getBlockAt(glm::vec3(block_pos.x-1, block_pos.y, block_pos.z)) == EMPTY) {
                    m_terrain.setBlockAt(block_pos.x-1, block_pos.y, block_pos.z, b);
                } else {
                    if (m_terrain.getBlockAt(glm::vec3(block_pos.x, block_pos.y+1, block_pos.z)) == EMPTY) {
                        m_terrain.setBlockAt(block_pos.x, block_pos.y+1, block_pos.z, b);
                    } else {
                        if (m_terrain.getBlockAt(glm::vec3(block_pos.x, block_pos.y-1, block_pos.z)) == EMPTY) {
                            m_terrain.setBlockAt(block_pos.x, block_pos.y-1, block_pos.z, b);
                        } else {
                            if (m_terrain.getBlockAt(glm::vec3(block_pos.x, block_pos.y, block_pos.z+1)) == EMPTY) {
                                m_terrain.setBlockAt(block_pos.x, block_pos.y, block_pos.z+1, b);
                            } else {
                                if (m_terrain.getBlockAt(glm::vec3(block_pos.x+1, block_pos.y, block_pos.z)) == EMPTY) {
                                    m_terrain.setBlockAt(block_pos.x+1, block_pos.y, block_pos.z, b);
                                }
                            }
                        }
                    }
                }
            }

        }
    }
}

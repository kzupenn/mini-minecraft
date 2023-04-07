#include "mygl.h"
#include <glm_includes.h>

#include <iostream>
#include <QApplication>
#include <QKeyEvent>
#include <thread>

#include "algo/perlin.h"
#include "scene/biome.h"
#include "scene/font.h"
#include "scene/inventory.h"
#include "scene/structure.h"
#include <QDateTime>

MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
      m_worldAxes(this),
      m_progLambert(this), m_progFlat(this), m_progOverlay(this), m_progInstanced(this),
      m_terrain(this), m_player(glm::vec3(48.f, 129.f, 48.f), m_terrain, this),
      time(0), m_block_texture(this), m_font_texture(this), m_inventory_texture(this), m_currentMSecsSinceEpoch(QDateTime::currentMSecsSinceEpoch()),
      m_mousePosPrev(glm::vec2(width() / 2, height() / 2)),
      ip("localhost")
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

    m_player.m_inventory.createVBOdata();
    m_player.m_inventory.hotbar.createVBOdata();
    Item a = Item(this, DIAMOND_HOE, 1);
    Item b = Item(this, DIAMOND_LEGGINGS, 1);
    Item c = Item(this, GOLD_NUGGET, 64);
    Item d = Item(this, IRON_NUGGET, 8);
    Item e = Item(this, IRON_BOOTS, 1);
    Item f = Item(this, STONE_SWORD, 1);
    Item g = Item(this, DIAMOND_SWORD, 1);
    Item h = Item(this, IRON_CHESTPLATE, 1);
    Item i = Item(this, STRING, 1);
    Item j = Item(this, IRON_HELMET, 1);
    Item k = Item(this, IRON_INGOT, 9);
    m_player.m_inventory.addItem(a);
    m_player.m_inventory.addItem(b);
    m_player.m_inventory.addItem(c);
    m_player.m_inventory.addItem(d);
    m_player.m_inventory.addItem(e);
    m_player.m_inventory.addItem(f);
    m_player.m_inventory.addItem(g);
    m_player.m_inventory.addItem(h);
    m_player.m_inventory.addItem(i);
    m_player.m_inventory.addItem(j);
    m_player.m_inventory.addItem(k);
    m_player.m_inventory.addItem(j);
    m_player.m_inventory.addItem(j);
    m_player.m_inventory.addItem(j);
    m_player.m_inventory.addItem(j);
    m_player.m_inventory.addItem(j);
    m_player.m_inventory.addItem(j);
    m_player.m_inventory.addItem(j);
    m_player.m_inventory.addItem(j);
    m_player.m_inventory.addItem(j);
    m_player.m_inventory.addItem(j);
    m_player.m_inventory.addItem(j);
    m_player.m_inventory.addItem(j);
    m_player.m_inventory.addItem(j);
    m_player.m_inventory.addItem(j);
    m_player.m_inventory.addItem(j);
    m_player.m_inventory.addItem(j);



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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Set the color with which the screen is filled at the start of each render call.
    glClearColor(0.37f, 0.74f, 1.0f, 1);

    printGLErrorLog();

    // Create a Vertex Attribute Object
    glGenVertexArrays(1, &vao);

    //load texture into memory and store on gpu
    m_block_texture.create(":/textures/block_item_textures.png");
    m_block_texture.load(0);

    m_font_texture.create(":/textures/ascii.png");
    m_font_texture.load(1);

    m_inventory_texture.create(":/textures/inventory.png");
    m_inventory_texture.load(2);

    overlayTransform = glm::scale(glm::mat4(1), glm::vec3(1.f/width(), 1.f/height(), 1.f));

    //Create the instance of the world axes
    m_worldAxes.createVBOdata();

    // Create and set up the diffuse shader
    m_progLambert.create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");
    // Create and set up the flat lighting shader
    m_progFlat.create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");
    m_progOverlay.create(":/glsl/overlay.vert.glsl", ":/glsl/overlay.frag.glsl");
    m_progInstanced.create(":/glsl/instanced.vert.glsl", ":/glsl/instanced.frag.glsl");

    // Set a color with which to draw geometry.
    // This will ultimately not be used when you change
    // your program to render Chunks with vertex colors
    // and UV coordinates
    //m_progLambert.setGeometryColor(glm::vec4(0,1,0,1));


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
    m_progOverlay.setViewProjMatrix(viewproj);

    overlayTransform = glm::scale(glm::mat4(1), glm::vec3(1.f/w, 1.f/h, 1.f));

    m_block_texture.bind(0);
    //m_font_texture.bind(1);

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
    for(int dx = minx-192; dx <= minx+192; dx+=64) {
        for(int dy = miny-192; dy <= miny+192; dy+=64) {
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
        for(int dx = minx-192; dx <= minx+192; dx+=64) {
            for(int dy = miny-192; dy <= miny+192; dy+=64) {
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

    m_progLambert.setTime(time);
    m_progFlat.setViewProjMatrix(m_player.mcr_camera.getViewProj());
    m_progLambert.setViewProjMatrix(m_player.mcr_camera.getViewProj());
    m_progInstanced.setViewProjMatrix(m_player.mcr_camera.getViewProj());
    m_block_texture.bind(0);
    renderTerrain();

    glDisable(GL_DEPTH_TEST);
//    m_progFlat.setModelMatrix(glm::mat4());
//    m_progFlat.setViewProjMatrix(m_player.mcr_camera.getViewProj());
//    m_progFlat.draw(m_worldAxes);

    m_progOverlay.setModelMatrix(glm::mat4());
    m_progOverlay.setViewProjMatrix(overlayTransform);
    m_font_texture.bind(0);

//    Font f2 = Font(this, "HELLO WORLD :)", glm::vec2(0, 70), glm::vec4(1,1,1,1), 50);
//    f2.createVBOdata();
//    m_progOverlay.draw(f2);
//    glEnable(GL_DEPTH_TEST);

    //inventory gui
    m_inventory_texture.bind(0);
    m_progOverlay.setModelMatrix(glm::translate(glm::mat4(1), glm::vec3(0, -height()+10, 0)));
    m_progOverlay.draw(m_player.m_inventory.hotbar);

    if(m_player.m_inventory.showInventory) {
        m_progOverlay.setModelMatrix(glm::mat4(1));
        m_progOverlay.draw(m_player.m_inventory);
    }

    //inventory items
    m_block_texture.bind(0);
    if(m_player.m_inventory.showInventory){
        for(int i = 0; i < m_player.m_inventory.items.size(); i++) {
            std::optional<Item>& item = m_player.m_inventory.items[i];
            if(item) {
                item.value().createVBOdata();
                m_progOverlay.setModelMatrix(glm::translate(glm::mat4(1), glm::vec3(-550.f*158.f/256.f+36.f/256.f*550.f*(i%9), -550.f*32.f/256.f-(i/9)*36.f/256.f*550.f, 0))*
                                             glm::scale(glm::mat4(1), glm::vec3(60,60,0)));
                m_progOverlay.draw(item.value());
            }
        }
        for(int i = 0; i < m_player.m_inventory.hotbar.items.size(); i++){
            std::optional<Item>& item = m_player.m_inventory.hotbar.items[i];
            if(item) {
                item.value().createVBOdata();
                m_progOverlay.setModelMatrix(glm::translate(glm::mat4(1), glm::vec3(-550.f*158.f/256.f+36.f/256.f*550.f*(i%9), -550.f*148.f/256.f-(i/9)*36.f/256.f*550.f, 0))*
                                             glm::scale(glm::mat4(1), glm::vec3(60,60,0)));
                m_progOverlay.draw(item.value());
            }
        }
    }

    for(int i = 0; i < m_player.m_inventory.hotbar.items.size(); i++){
        std::optional<Item>& item = m_player.m_inventory.hotbar.items[i];
        if(item) {
            item.value().createVBOdata();
            m_progOverlay.setModelMatrix(glm::translate(glm::mat4(1), glm::vec3(-450+i*100 + 20, 30-height(), 0))*
                                         glm::scale(glm::mat4(1), glm::vec3(60,60,0)));
            m_progOverlay.draw(item.value());
        }
    }

    //inventory numbers
    m_font_texture.bind(0);
    if(m_player.m_inventory.showInventory){
        for(int i = 0; i < m_player.m_inventory.items.size(); i++) {
            std::optional<Item>& item = m_player.m_inventory.items[i];
            if(item && item->item_count > 1) {
                Font ff = Font(this, std::to_string(item->item_count), glm::vec4(1,1,1,1), 30);
                ff.createVBOdata();
                m_progOverlay.setModelMatrix(glm::translate(glm::mat4(1), glm::vec3(-550.f*158.f/256.f+36.f/256.f*550.f*(i%9)+65-ff.width, -5-550.f*32.f/256.f-(i/9)*36.f/256.f*550.f, 0)));
                m_progOverlay.draw(ff);
            }
        }
        for(int i = 0; i < m_player.m_inventory.hotbar.items.size(); i++){
            std::optional<Item>& item = m_player.m_inventory.hotbar.items[i];
            if(item && item->item_count > 1) {
                Font ff = Font(this, std::to_string(item->item_count), glm::vec4(1,1,1,1), 30);
                ff.createVBOdata();
                m_progOverlay.setModelMatrix(glm::translate(glm::mat4(1), glm::vec3(-550.f*158.f/256.f+36.f/256.f*550.f*(i%9)+65-ff.width, -5-550.f*148.f/256.f-(i/9)*36.f/256.f*550.f, 0)));
                m_progOverlay.draw(ff);
            }
        }
    }

    for(int i = 0; i < m_player.m_inventory.hotbar.items.size(); i++){
        std::optional<Item>& item = m_player.m_inventory.hotbar.items[i];
        if(item && item->item_count > 1) {
            Font ff = Font(this, std::to_string(item->item_count), glm::vec4(1,1,1,1), 35);
            ff.createVBOdata();
            m_progOverlay.setModelMatrix(glm::translate(glm::mat4(1), glm::vec3(-450+i*100 + 96-ff.width, 10-height(), 0)));
            m_progOverlay.draw(ff);
        }
    }

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
    } else if (e->key() == Qt::Key_W) {
        m_inputs.wPressed = true;
    } else if (e->key() == Qt::Key_S) {
        m_inputs.sPressed = true;
    } else if (e->key() == Qt::Key_D) {
        m_inputs.dPressed = true;
    } else if (e->key() == Qt::Key_A) {
        m_inputs.aPressed = true;
    } else if (e->key() == Qt::Key_Shift) {
        m_inputs.lshiftPressed = true;
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
    } else if (e->key() == Qt::Key_Shift) {
        m_inputs.lshiftPressed = false;
    } else if (e->key() == Qt::Key_F) {
        m_inputs.fPressed = false;
    } else if (e->key() ==Qt::Key_Space) {
        m_inputs.spacePressed = false;
    }
    //inventory hotkeys
    else if (e->key() == Qt::Key_E) {
        m_player.m_inventory.showInventory = !m_player.m_inventory.showInventory;
    } else if (e->key() == Qt::Key_Q) {
        m_player.m_inventory.showInventory = !m_player.m_inventory.showInventory;
    } else if (e->key() == Qt::Key_1) {
        m_player.m_inventory.hotbar.selected = 0;
        m_player.m_inventory.hotbar.createVBOdata();
    } else if (e->key() == Qt::Key_2) {
        m_player.m_inventory.hotbar.selected = 1;
        m_player.m_inventory.hotbar.createVBOdata();
    } else if (e->key() == Qt::Key_3) {
        m_player.m_inventory.hotbar.selected = 2;
        m_player.m_inventory.hotbar.createVBOdata();
    } else if (e->key() == Qt::Key_4) {
        m_player.m_inventory.hotbar.selected = 3;
        m_player.m_inventory.hotbar.createVBOdata();
    } else if (e->key() == Qt::Key_5) {
        m_player.m_inventory.hotbar.selected = 4;
        m_player.m_inventory.hotbar.createVBOdata();
    } else if (e->key() == Qt::Key_6) {
        m_player.m_inventory.hotbar.selected = 5;
        m_player.m_inventory.hotbar.createVBOdata();
    } else if (e->key() == Qt::Key_7) {
        m_player.m_inventory.hotbar.selected = 6;
        m_player.m_inventory.hotbar.createVBOdata();
    } else if (e->key() == Qt::Key_8) {
        m_player.m_inventory.hotbar.selected = 7;
        m_player.m_inventory.hotbar.createVBOdata();
    } else if (e->key() == Qt::Key_9) {
        m_player.m_inventory.hotbar.selected = 8;
        m_player.m_inventory.hotbar.createVBOdata();
    }
}

void MyGL::mouseMoveEvent(QMouseEvent *e) {
    // TODO
    const float spd = 0.6;
    glm::vec2 pos(e->pos().x(), e->pos().y());
    glm::vec2 diff = spd * (pos - m_mousePosPrev);
    m_mousePosPrev = pos;
    m_inputs.mouseX = diff.x;
    m_inputs.mouseY = diff.y;
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
    } else if (e->buttons() & Qt::RightButton) {
        float bound = 3.f;
        glm::vec3 cam_pos = m_player.mcr_camera.mcr_position;
        glm::vec3 ray_dir = m_player.getLook() * bound;

        float dist;
        glm::ivec3 block_pos;
        if (m_terrain.gridMarch(cam_pos, ray_dir, &dist, &block_pos)) {
            BlockType b = m_terrain.getBlockAt(glm::vec3(block_pos));
            if (m_terrain.getBlockAt(glm::vec3(block_pos.x, block_pos.y, block_pos.z-1)) == EMPTY) {
                m_terrain.setBlockAt(block_pos.x, block_pos.y, block_pos.z-1, b);
            } else if (m_terrain.getBlockAt(glm::vec3(block_pos.x-1, block_pos.y, block_pos.z)) == EMPTY) {
                m_terrain.setBlockAt(block_pos.x-1, block_pos.y, block_pos.z, b);
            } else if (m_terrain.getBlockAt(glm::vec3(block_pos.x, block_pos.y+1, block_pos.z)) == EMPTY) {
                m_terrain.setBlockAt(block_pos.x, block_pos.y+1, block_pos.z, b);
            } else if (m_terrain.getBlockAt(glm::vec3(block_pos.x, block_pos.y-1, block_pos.z)) == EMPTY) {
                m_terrain.setBlockAt(block_pos.x, block_pos.y-1, block_pos.z, b);
            } else if (m_terrain.getBlockAt(glm::vec3(block_pos.x, block_pos.y, block_pos.z+1)) == EMPTY) {
                m_terrain.setBlockAt(block_pos.x, block_pos.y, block_pos.z+1, b);
            } else if (m_terrain.getBlockAt(glm::vec3(block_pos.x+1, block_pos.y, block_pos.z)) == EMPTY) {
                m_terrain.setBlockAt(block_pos.x+1, block_pos.y, block_pos.z, b);
            }
        }
    }
}

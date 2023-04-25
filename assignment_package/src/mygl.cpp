#include "mygl.h"
#include <glm_includes.h>

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <QApplication>
#include <QKeyEvent>

#include "algo/perlin.h"
#include "scene/biome.h"
#include "scene/font.h"
#include "scene/inventory.h"
#include "scene/runnables.h"
#include "scene/structure.h"
#include "server/getip.h"
#include <QDateTime>

MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
      m_worldAxes(this),
      m_progLambert(this), m_progFlat(this), m_progOverlay(this), m_progInstanced(this), m_progPostProcess(this), m_progSky(this),
      m_terrain(this), m_player(glm::vec3(48.f, 129.f, 48.f), m_terrain, this, QString("Player")),
      m_time(0), m_block_texture(this), m_font_texture(this), m_inventory_texture(this), m_icon_texture(this), m_currentMSecsSinceEpoch(QDateTime::currentMSecsSinceEpoch()),
      ip("localhost"),
      m_frame(this, this->width(), this->height(), this->devicePixelRatio()), m_quad(this), m_sky(this),
      m_rectangle(this), m_crosshair(this), m_mychat(this), m_heart(this),
      m_halfheart(this), m_fullheart(this), m_armor(this), m_fullarmor(this), m_halfarmor(this), m_skin_texture(this),
      deathMsg1(this), deathMsg2(this),
      mouseMove(false), chatMode(false), drawSky(false)
{
    // Connect the timer to a function so that when the timer ticks the function is executed
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(tick()));
}

//move stuff from constructor to here so that it can be called upon entering game scene
void MyGL::start(bool joinServer, QString username) {
    setFocusPolicy(Qt::ClickFocus);

    setMouseTracking(true); // MyGL will track the mouse's movements even if a mouse button is not pressed
    setCursor(Qt::BlankCursor); // Make the cursor invisible

    //set up overlays
    overlayTransform = glm::scale(glm::mat4(1), glm::vec3(1.f/width(), 1.f/height(), 1.f));
    m_crosshair.createVBOdata();
    m_rectangle.createVBOdata();
    m_heart.createVBOdata();
    m_halfheart.createVBOdata();
    m_fullheart.createVBOdata();
    m_armor.createVBOdata();
    m_halfarmor.createVBOdata();
    m_fullarmor.createVBOdata();
    
    //player model
    m_player.createVBOdata();

    //noise function distribution tests
    //distTest();
    //biomeDist();

    //check if we need to host a server
    if(!joinServer) {
        SERVER = mkU<Server>(1);
        while(!SERVER->setup);
        ip = getIP().data();
    }

    verified_server = false;
    init_client();

    //block until we get world spawn info
    while(!verified_server);
    m_player.setState(m_terrain.worldSpawn, glm::vec3(), 0, 0, AIR);
    m_player.name = username;

    PlayerJoinPacket pjp = PlayerJoinPacket(true, 0, username);
    send_packet(&pjp);

    m_player.m_inventory.createVBOdata();
    m_player.m_inventory.hotbar.createVBOdata();
    Item a = Item(this, DIAMOND_HOE, 1, true);
    Item b = Item(this, DIAMOND_LEGGINGS, 1, true);
    Item bb = Item(this, GOLDEN_LEGGINGS, 1, true);
    Item c = Item(this, GOLD_NUGGET, 64, true);
    Item d = Item(this, IRON_NUGGET, 8, true);
    Item e = Item(this, IRON_BOOTS, 1, true);
    Item f = Item(this, STONE_SWORD, 1, true);
    Item g = Item(this, DIAMOND_SWORD, 1, true);
    Item h = Item(this, IRON_CHESTPLATE, 1, true);
    Item i = Item(this, STRING, 1, true);
    Item j = Item(this, IRON_HELMET, 1, true);
    Item k = Item(this, IRON_INGOT, 9, true);
    Item l = Item(this, DIRT_, 12, true);
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
    m_player.m_inventory.addItem(k, 26);
    m_player.m_inventory.addItem(j);
    m_player.m_inventory.addItem(j);
    m_player.m_inventory.addItem(j);
    m_player.m_inventory.addItem(l);

    m_player.m_inventory.armor[0] = j;
    m_player.m_inventory.armor[1] = h;
    m_player.m_inventory.armor[2] = b;
    m_player.m_inventory.armor[3] = e;
    m_player.armor = m_player.m_inventory.calcArmor();

    // Tell the timer to redraw 60 times per second
    m_timer.start(16);
}

MyGL::~MyGL() {
    makeCurrent();
    glDeleteVertexArrays(1, &vao);
    close_client();
    SERVER->shutdown();
    m_frame.destroy();
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
    glDepthFunc(GL_LEQUAL);
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

    m_skin_texture.create(":/textures/steve.png");
    m_skin_texture.load(4);
    
    m_icon_texture.create(":/textures/icons.png");
    m_icon_texture.load(5);

    overlayTransform = glm::scale(glm::mat4(1), glm::vec3(1.f/width(), 1.f/height(), 1.f));

    //Create the instance of the world axes
    m_worldAxes.createVBOdata();

    // Create and set up the diffuse shader
    m_progLambert.create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");
    // Create and set up the flat lighting shader
    m_progFlat.create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");
    m_progInstanced.create(":/glsl/instanced.vert.glsl", ":/glsl/instanced.frag.glsl");
    m_progPostProcess.create(":/glsl/post.vert.glsl", ":/glsl/post.frag.glsl");
    m_progOverlay.create(":/glsl/overlay.vert.glsl", ":/glsl/overlay.frag.glsl");
    m_progSky.create(":/glsl/sky.vert.glsl", ":/glsl/sky.frag.glsl");
    // Set a color with which to draw geometry.
    // This will ultimately not be used when you change
    // your program to render Chunks with vertex colors
    // and UV coordinates
    //m_progLambert.setGeometryColor(glm::vec4(0,1,0,1));


    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.
    glBindVertexArray(vao);

    m_frame.create();
    m_quad.createVBOdata();
    deathMsg1.setText("You died!");
    deathMsg2.setText("press enter to respawn");
    m_sky.createVBOdata();
}

void MyGL::resizeGL(int w, int h) {
    //This code sets the concatenated view and perspective projection matrices used for
    //our scene's camera view.
    m_player.setCameraWidthHeight(static_cast<unsigned int>(w), static_cast<unsigned int>(h));
    glm::mat4 viewproj = m_player.mcr_camera.getViewProj();
    overlayTransform = glm::scale(glm::mat4(1), glm::vec3(1.f/w, 1.f/h, 1.f));

    // Upload the view-projection matrix to our shaders (i.e. onto the graphics card)

    m_progLambert.setViewProjMatrix(viewproj);
    m_progFlat.setViewProjMatrix(viewproj);
    m_progOverlay.setViewProjMatrix(viewproj);
    m_progSky.setViewProjMatrix(glm::inverse(viewproj));

    m_progSky.setDimensions(width(), height());
    m_progSky.setEye(m_player.m_position);

    overlayTransform = glm::scale(glm::mat4(1), glm::vec3(1.f/w, 1.f/h, 1.f));

    //m_block_texture.bind(0);
    //m_font_texture.bind(1);

    printGLErrorLog();

    m_frame.resize(w, h, 1.f);
    m_frame.destroy();
    m_frame.create();
}


// MyGL's constructor links tick() to a timer that fires 60 times per second.
// We're treating MyGL as our game engine class, so we're going to perform
// all per-frame actions here, such as performing physics updates on all
// entities in the scene.

void MyGL::tick() {
    m_time++;
    long long ct = QDateTime::currentMSecsSinceEpoch();
    float dt = 0.001 * (ct - m_currentMSecsSinceEpoch);
    m_currentMSecsSinceEpoch = ct;

    //server tick
    if(SERVER){
        if(SERVER->setup) {
            SERVER->tick();
        }
    }

    if (mouseMove) updateMouse();
    m_player.tick(dt, m_inputs);
    setupTerrainThreads();

    update(); // Calls paintGL() as part of a larger QOpenGLWidget pipeline

    ItemType inHand, onHead, onChest, onLeg, onFoot;
    if(!m_player.m_inventory.hotbar.items[m_player.m_inventory.hotbar.selected]) inHand = AIR;
    else inHand = m_player.m_inventory.hotbar.items[m_player.m_inventory.hotbar.selected]->type;
    if(!m_player.m_inventory.armor[0]) onHead = AIR;
    else onHead = m_player.m_inventory.armor[0]->type;
    if(!m_player.m_inventory.armor[1]) onChest = AIR;
    else onChest = m_player.m_inventory.armor[1]->type;
    if(!m_player.m_inventory.armor[2]) onLeg = AIR;
    else onLeg = m_player.m_inventory.armor[2]->type;
    if(!m_player.m_inventory.armor[3]) onFoot = AIR;
    else onFoot = m_player.m_inventory.armor[3]->type;
    PlayerStatePacket pp = PlayerStatePacket(m_player.getPos(),
                                             m_player.getVelocity(),
                                             m_player.getTheta(),
                                             m_player.getPhi(),
                                             inHand, onHead, onChest, onLeg, onFoot,
                                             m_player.m_flightMode);
    if(!m_player.isDead) send_packet(&pp);

    sendPlayerDataToGUI(); // Updates the info in the secondary window displaying player data
    //generates chunks based on player position
    //zone player is in

}

void MyGL::setupTerrainThreads() {
    //does rendering stuff
    int minx = floor(m_player.mcr_position.x/64)*64;
    int miny = floor(m_player.mcr_position.z/64)*64;
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
    if(m_time%30 == 0) {
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

void MyGL::sendPlayerDataToGUI() const{
    emit sig_sendServerIP(QString::fromStdString(ip));
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
    m_frame.bindFrameBuffer();
    glViewport(0,0,this->width(), this->height());

    //set sky color
    int modVal = 1000;
    float time = (float)((m_time+(modVal*4)) % (modVal*4)) / (modVal*4);
    glm::vec3 daySky = glm::vec3(0.37f, 0.74f, 1.0f);
    glm::vec3 nightSky = glm::vec3(2.0/255.0, 1.0/255.0, 78.0/255.0);
    if (time <= 0.5) {
        glm::vec3 sky = daySky;

        if (time*2 < 0.25) {
            sky = glm::mix(daySky, nightSky, (0.25 - time*2)/0.25);
        }
        if (time*2 > 0.75) {
            sky = glm::mix(nightSky, daySky, (1 - time*2)/0.25);
        }

        glClearColor(sky.x, sky.y, sky.z, 1);
    } else {
        glm::vec3 sky = nightSky;

        glClearColor(sky.x, sky.y, sky.z, 1.0);
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    m_progPostProcess.setType(m_player.getType());
    m_frame.bindToTextureSlot(3);

    m_progLambert.setTime(m_time);
    m_progFlat.setViewProjMatrix(m_player.mcr_camera.getViewProj());
    m_progLambert.setViewProjMatrix(m_player.mcr_camera.getViewProj());
    m_progInstanced.setViewProjMatrix(m_player.mcr_camera.getViewProj());

    m_progSky.setViewProjMatrix(glm::inverse(m_player.mcr_camera.getViewProj()));
    m_progSky.setDimensions(width(), height());
    m_progSky.setEye(m_player.mcr_position);
    m_progSky.setTime(m_time);

    if(drawSky) m_progSky.draw(m_sky);

    renderTerrain();
    renderEntities();

    glBindFramebuffer(GL_FRAMEBUFFER, this->defaultFramebufferObject());
    glViewport(0,0,this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_progPostProcess.setType(m_player.getType());
    m_frame.bindToTextureSlot(3);

    renderOverlays();
}

// TODO: Change this so it renders the nine zones of generated
// terrain that surround the player (refer to Terrain::m_generatedTerrain
// for more info)

void MyGL::renderTerrain() {
    m_block_texture.bind(0);
    //chunk player is in
    int renderDist = 256;
    float x = floor(m_player.mcr_position.x/16.f)*16;
    float y = floor(m_player.mcr_position.z/16.f)*16;

    m_terrain.draw(x-renderDist, x+renderDist, y-renderDist, y+renderDist, &m_progLambert);
    //m_terrain.draw(0, 1024, 0, 1024, &m_progInstanced);
}
void MyGL::renderOverlays() {
    glDisable(GL_DEPTH_TEST);
    m_progPostProcess.drawPostProcess(m_quad, m_frame.getTextureSlot());

    m_progFlat.setViewProjMatrix(overlayTransform);
    m_progOverlay.setViewProjMatrix(overlayTransform);

    //chat gui
    m_font_texture.bind(0);
    float shiftChat = 0;

    chatQueue_mutex.lock();
    while(!chatQueue.empty()) {
        m_chat.push_front(Font(this, chatQueue.front().first, chatQueue.front().second));
        if(m_chat.size() > 20) m_chat.pop_back();
        chatQueue.pop();
    }

    if(chatMode) {
        m_progFlat.setModelMatrix(glm::translate(glm::mat4(), glm::vec3(-width()+10, -height()+148, 0))*
                                  glm::scale(glm::mat4(), glm::vec3(30*m_mychat.width+10, 34, 0)));
        m_progFlat.draw(m_rectangle);
        m_progOverlay.setModelMatrix(glm::translate(glm::mat4(), glm::vec3(-width()+12, -height()+150, 0))*
                                  glm::scale(glm::mat4(), glm::vec3(30,30,0)));
        m_progOverlay.draw(m_mychat);
        shiftChat = 60;
    }
    for(int i = 0; i < m_chat.size(); i++) {
        m_progFlat.setModelMatrix(glm::translate(glm::mat4(), glm::vec3(-width()+10, -height()+148+34*i+shiftChat, 0))*
                                  glm::scale(glm::mat4(), glm::vec3(30*m_chat[i].width+4, 34, 0)));
        m_progFlat.draw(m_rectangle);
        m_progOverlay.setModelMatrix(glm::translate(glm::mat4(), glm::vec3(-width()+12, -height()+150+34*i+shiftChat, 0))*
                                  glm::scale(glm::mat4(), glm::vec3(30,30,0)));
        m_progOverlay.draw(m_chat[i]);
    }
    chatQueue_mutex.unlock();

    //crosshair
    m_progFlat.setModelMatrix(glm::mat4());

    m_progFlat.draw(m_crosshair);

    //armor and health bars
    m_icon_texture.bind(0);
    for(int i = 0; i < 10; i++) {
        m_progOverlay.setModelMatrix(glm::translate(glm::mat4(), glm::vec3(-450 + 35*i, -height()+110, 0))*
                                     glm::scale(glm::mat4(), glm::vec3(35,35,0)));
        m_progOverlay.draw(m_heart);
    }
    int fullhps = 0;
    for(; fullhps < m_player.health/2; fullhps++) {
        m_progOverlay.setModelMatrix(glm::translate(glm::mat4(), glm::vec3(-450 + 35*fullhps, -height()+110, 0))*
                                     glm::scale(glm::mat4(), glm::vec3(35,35,0)));
        m_progOverlay.draw(m_fullheart);
    }
    if(m_player.health%2) {
        m_progOverlay.setModelMatrix(glm::translate(glm::mat4(), glm::vec3(-450 + 35*fullhps, -height()+110, 0))*
                                     glm::scale(glm::mat4(), glm::vec3(35,35,0)));
        m_progOverlay.draw(m_halfheart);
    }

    for(int i = 0; i < 10; i++) {
        m_progOverlay.setModelMatrix(glm::translate(glm::mat4(), glm::vec3(450 - 35*i-35, -height()+110, 0))*
                                     glm::scale(glm::mat4(), glm::vec3(35,35,0)));
        m_progOverlay.draw(m_armor);
    }
    int fullars = 0;
    for(; fullars < m_player.armor/2; fullars++) {
        int i = 9-fullars;
        m_progOverlay.setModelMatrix(glm::translate(glm::mat4(), glm::vec3(450 - 35*i-35, -height()+110, 0))*
                                     glm::scale(glm::mat4(), glm::vec3(35,35,0)));
        m_progOverlay.draw(m_fullarmor);
    }
    if(m_player.armor%2) {
        m_progOverlay.setModelMatrix(glm::translate(glm::mat4(), glm::vec3(450 - 35*(9-fullars)-35, -height()+110, 0))*
                                     glm::scale(glm::mat4(), glm::vec3(35,35,0)));
        m_progOverlay.draw(m_halfarmor);
    }

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
        //inventory
        for(int i = 0; i < m_player.m_inventory.items.size(); i++) {
            std::optional<Item>& item = m_player.m_inventory.items[i];
            if(item) {
                item->draw(&m_progOverlay, m_block_texture, m_font_texture,
                           60, 30, glm::vec3(-550.f*158.f/256.f+36.f/256.f*550.f*(i%9), -550.f*32.f/256.f-(i/9)*36.f/256.f*550.f, 0), glm::vec3(-550.f*158.f/256.f+36.f/256.f*550.f*(i%9)+65, -5-550.f*32.f/256.f-(i/9)*36.f/256.f*550.f, 0));
            }
        }
        //hotbar
        for(int i = 0; i < m_player.m_inventory.hotbar.items.size(); i++){
            std::optional<Item>& item = m_player.m_inventory.hotbar.items[i];
            if(item) {
                item->draw(&m_progOverlay, m_block_texture, m_font_texture,
                            60, 30,
                           glm::vec3(-550.f*158.f/256.f+36.f/256.f*550.f*(i%9), -550.f*148.f/256.f-(i/9)*36.f/256.f*550.f, 0),
                           glm::vec3(-550.f*158.f/256.f+36.f/256.f*550.f*(i%9)+65, -5-550.f*148.f/256.f-(i/9)*36.f/256.f*550.f, 0));
            }
        }
        //armor
        for(int i = 0; i < 4; i++) {
            std::optional<Item> item = m_player.m_inventory.armor[i];
            if(item){
                float dx = -550.f*158.f/256.f;
                float dy = 550.f*118.f/256.f-36.f/256.f*550.f*i;
                item->draw(&m_progOverlay, m_block_texture, m_font_texture,
                            60, 30,
                           glm::vec3(dx, dy, 0),
                           glm::vec3(dx+65, -5+dy, 0));
            }
        }
    }

    for(int i = 0; i < m_player.m_inventory.hotbar.items.size(); i++){
        std::optional<Item>& item = m_player.m_inventory.hotbar.items[i];
        if(item) {
            item->draw(&m_progOverlay, m_block_texture, m_font_texture,
                       60, 35, glm::vec3(-450+i*100 + 20, 30-height(), 0), glm::vec3(-450+i*100 + 96, 10-height(), 0));
        }
    }

    if(m_player.m_inventory.showInventory) {
        QPoint cur = mapFromGlobal(QCursor::pos());
        if(m_cursor_item) {
            m_cursor_item->draw(&m_progOverlay, m_block_texture, m_font_texture,
                                60, 30, glm::vec3(2*cur.x()-width()+32, -2*cur.y()+height()-31, 0), glm::vec3(2*cur.x()-width()+97, -5-2*cur.y()+height()-31, 0));
        }
        m_progFlat.setModelMatrix(glm::translate(glm::mat4(1), glm::vec3(2*cur.x()-width()+65, -2*cur.y()+height(), 0)));
        m_progFlat.draw(m_crosshair);
    }

    //death gui
    if(m_player.isDead){
        m_progFlat.setModelMatrix(glm::translate(glm::mat4(), glm::vec3(-width(), -height(), 0))*glm::scale(glm::mat4(), glm::vec3(2*width(), 2*height(), 1)));
        m_progFlat.draw(m_rectangle);
        m_font_texture.bind(0);
        m_progOverlay.setModelMatrix(glm::translate(glm::mat4(), glm::vec3(-deathMsg1.width*50, 50, 0)) * glm::scale(glm::mat4(), glm::vec3(100, 100, 1)));
        m_progOverlay.draw(deathMsg1);
        m_progOverlay.setModelMatrix(glm::translate(glm::mat4(), glm::vec3(-deathMsg2.width*20, -100, 0)) * glm::scale(glm::mat4(), glm::vec3(40, 40, 1)));
        m_progOverlay.draw(deathMsg2);
    }
    glEnable(GL_DEPTH_TEST);
}

void MyGL::renderEntities() {
    //lock queue for rendering and create vbos
    m_multiplayers_mutex.lock();
    itemQueue_mutex.lock();
    while(!itemQueue.empty()) {
        itemQueue.front()->createVBOdata();
        itemQueue.pop();
    }
    //player arm
    m_player.drawArm(&m_progLambert, m_skin_texture);
    m_player.drawCubeDisplay(&m_progFlat);
    //players
    for(std::map<int, uPtr<Player>>::iterator it = m_multiplayers.begin(); it != m_multiplayers.end(); it++) {
        Player* cur = it->second.get();
        if (!cur->created) cur->createVBOdata();
        cur->orientCamera();
        if (glm::length(cur->getVelocity()) > 0.00005) {
            if (!cur->swinging && cur->stopped) {
                cur->start_swing = m_time;
                cur->swinging = true;
                cur->stopped = false;
                cur->swing_dir *= -1;
            }
        } else {
            cur->swinging = false;
        }
        cur->draw(&m_progLambert, m_skin_texture, m_time);
    }
    m_multiplayers_mutex.unlock();
    //item entites
    for(auto& it: m_terrain.item_entities) {
        ItemEntity ite = it.second;
        m_progOverlay.setModelMatrix(glm::translate(glm::mat4(), ite.mcr_position) *
                                     glm::rotate(glm::mat4(), ite.a, glm::vec3(0, 1, 0)));
        m_progOverlay.draw(ite.item);
    }
    //unlock mutex to allow modifications to queue
    itemQueue_mutex.unlock();
}


void MyGL::keyPressEvent(QKeyEvent *e) {
    if(m_player.isDead) {
        //checks for respawn
        if(e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
            m_player.setState(m_terrain.worldSpawn, glm::vec3(0), 0, 0, AIR);
            m_player.isDead = false;
            m_player.health = 20;
            RespawnPacket rp = RespawnPacket(client_id);
            send_packet(&rp);
        }
        return;
    }
    // http://doc.qt.io/qt-5/qt.html#Key-enum
    // This could all be much more efficient if a switch
    // statement were used, but I really dislike their
    // syntax so I chose to be lazy and use a long
    // chain of if statements instead
    if(chatMode && !m_player.m_inventory.showInventory) {
        if(e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
            if(!m_mychat.getText().empty()){
                m_chat_mutex.lock();
                m_chat.push_front(Font(this, m_player.name.toStdString() + ": "+m_mychat.getText(), glm::vec4(1)));
                if(m_chat.size() > 20) m_chat.pop_back();
                m_chat_mutex.unlock();
                ChatPacket cpp = ChatPacket(client_id, QString::fromStdString(m_mychat.getText()));
                send_packet(&cpp);
            }
            chatMode = false;
        }
        else if(e->key() == Qt::Key_Escape) {
            chatMode = false;
        }
        else if(e->key() == Qt::Key_Delete || e->key() == Qt::Key_Backslash || e->key() == Qt::Key_Clear || e->key() == 16777219) {
            std::string ss = m_mychat.getText();
            ss.pop_back();
            m_mychat.setText(ss);
        }
        else if(e->key() != Qt::Key_Shift){
            if(e->modifiers() & Qt::ShiftModifier){
                m_mychat.setText(m_mychat.getText() + char(e->key()));
            }
            else {
                m_mychat.setText(m_mychat.getText() + char(std::tolower(e->key())));
            }
        }
    }
    else if(m_player.m_inventory.showInventory) {
        if (e->key() == Qt::Key_E || e->key() == Qt::Key_Escape) {
            m_player.m_inventory.showInventory = !m_player.m_inventory.showInventory;
            mouseMove = !m_player.m_inventory.showInventory;
        }
    }
    else {
        if (e->key() == Qt::Key_Escape) {
            open = false;
            if(SERVER){
                SERVER->open = false;
            }
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
        } else if (e->key() == Qt::Key_F) {
            m_inputs.fPressed = true;
        } else if (e->key() == Qt::Key_Space) {
            m_inputs.spacePressed = true;
        } else if (e->key() == Qt::Key_Right) {
            m_inputs.mouseX = 5.f;
        } else if (e->key() == Qt::Key_Left) {
            m_inputs.mouseX = -5.f;
        } else if (e->key() == Qt::Key_Up) {
            m_inputs.mouseY = -5.f;
        } else if (e->key() == Qt::Key_Down) {
            m_inputs.mouseY = 5.f;
        }     //inventory hotkeys
        else if (e->key() == Qt::Key_E) {
            m_player.m_inventory.showInventory = !m_player.m_inventory.showInventory;
            mouseMove = !m_player.m_inventory.showInventory;
        } else if (e->key() == Qt::Key_Q) { //ejecting an item
//            if(m_player.m_inventory.hotbar.items[m_player.m_inventory.hotbar.selected]) {
//                m_player.m_inventory.hotbar.items[m_player.m_inventory.hotbar.selected]->item_count--;
//                ItemEntityStatePacket bcp = ItemEntityStatePacket(-1, m_player.m_inventory.hotbar.items[m_player.m_inventory.hotbar.selected]->type, m_player.m_inventory.hotbar.items[m_player.m_inventory.hotbar.selected]->item_count, m_player.m_position);
//                send_packet(&bcp);
//                if(m_player.m_inventory.hotbar.items[m_player.m_inventory.hotbar.selected]->item_count == 0) {
//                    m_player.m_inventory.hotbar.items[m_player.m_inventory.hotbar.selected].reset();
//                }
//                else {
//                    m_player.m_inventory.hotbar.items[m_player.m_inventory.hotbar.selected]->count_text.setText(std::to_string(m_player.m_inventory.hotbar.items[m_player.m_inventory.hotbar.selected]->item_count));
//                }
//            }
        } else if (e->key() == Qt::Key_M) { //drawing sky
            drawSky = !drawSky;
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
        } else if(e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
            chatMode = true;
            m_mychat.setText("");
        }
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
    } else if (e->key() == Qt::Key_Space) {
        m_inputs.spacePressed = false;
    }
}

void MyGL::updateMouse() {
    float sens = 0.15;
    QPoint cur = QWidget::mapFromGlobal(QCursor::pos());
    m_inputs.mouseX = sens * (cur.x() - width() / 2);
    m_inputs.mouseY = sens * (cur.y() - height() / 2);
    moveMouseToCenter();
}

void MyGL::mousePressEvent(QMouseEvent *e) {
    //disallow mouse inputs while in death screen
    if(m_player.isDead) return;

    if(m_player.m_inventory.showInventory) {
        QPoint cur = 2*mapFromGlobal(QCursor::pos());
        cur = QPoint(cur.x()-width(), height()-cur.y());
        for(int i = 0; i < 4; i++) {
            float dx = -550.f*158.f/256.f-36.f/256.f*550.f;
            float dy = 550.f*118.f/256.f-36.f/256.f*550.f*i;
            if(cur.x() >= dx && cur.x() <= dx+36.f/256.f*550.f
                    && cur.y() <= dy+36.f/256.f*550.f && cur.y() >= dy) {
                std::optional<Item> foo = m_player.m_inventory.armor[i];
                if(e->buttons() & Qt::LeftButton && m_player.m_inventory.isArmor(m_cursor_item, i)) {
                    m_player.m_inventory.armor[i] = m_cursor_item;
                    m_cursor_item = foo;
                    m_player.armor = m_player.m_inventory.calcArmor();
                }
                break;
            }
        }
        for(int i = 0; i < m_player.m_inventory.items.size(); i++) {
            float dx = -550.f*158.f/256.f+36.f/256.f*550.f*(i%9);
            float dy = -550.f*32.f/256.f-(i/9)*36.f/256.f*550.f;
            if(cur.x() >= dx-36.f/256.f*550.f && cur.x() <= dx &&
                    cur.y() <= dy+36.f/256.f*550.f && cur.y() >= dy) {
                std::optional<Item> foo = m_player.m_inventory.items[i];
                if(e->buttons() & Qt::LeftButton) {
                    if(foo && m_cursor_item) {
                        m_cursor_item->merge(foo.value());
                        if(foo->item_count == 0){
                            foo.reset();
                        }
                    }
                    m_player.m_inventory.items[i] = m_cursor_item;
                    m_cursor_item = foo;
                }
                else if (e->buttons() & Qt::RightButton) {
                    if(m_cursor_item.has_value()) {
                        if(!m_player.m_inventory.items[i].has_value()) {
                            m_player.m_inventory.items[i] = Item(this, m_cursor_item->type, 1, true);
                            m_cursor_item-> item_count--;

                            if(m_cursor_item->item_count == 0) m_cursor_item.reset();
                            else m_cursor_item->count_text.setText(std::to_string(m_cursor_item->item_count));
                        }
                        else if(m_player.m_inventory.items[i]->type == m_cursor_item->type && m_player.m_inventory.items[i] ->item_count < m_player.m_inventory.items[i] -> max_count) {
                            m_cursor_item->item_count --;
                            if(m_cursor_item->item_count == 0) m_cursor_item.reset();
                            else m_cursor_item->count_text.setText(std::to_string(m_cursor_item->item_count));

                            m_player.m_inventory.items[i]->item_count ++;
                            m_player.m_inventory.items[i]->count_text.setText(std::to_string(m_player.m_inventory.items[i]->item_count));
                        }
                    }
                    else {
                        if(m_player.m_inventory.items[i].has_value()) {
                           int toMerge = (m_player.m_inventory.items[i]->item_count+1)/2;
                            m_cursor_item = Item(this, m_player.m_inventory.items[i]->type, toMerge, true);

                            m_player.m_inventory.items[i]->item_count -= toMerge;
                            if(m_player.m_inventory.items[i]->item_count == 0) m_player.m_inventory.items[i].reset();
                            else m_player.m_inventory.items[i]->count_text.setText(std::to_string(m_player.m_inventory.items[i]->item_count));

                        }
                    }
                }
                break;
            }
        }
        for(int i = 0; i < m_player.m_inventory.hotbar.items.size(); i++){
            float dx = -550.f*158.f/256.f+36.f/256.f*550.f*(i%9);
            float dy = -550.f*148.f/256.f-(i/9)*36.f/256.f*550.f;
            if(cur.x() >= dx-36.f/256.f*550.f && cur.x() <= dx  &&
                    cur.y() <= dy+36.f/256.f*550.f && cur.y() >= dy) {
                std::optional<Item> foo = m_player.m_inventory.hotbar.items[i];
                if(e->buttons() & Qt::LeftButton) {
                    if(foo && m_cursor_item) {
                        m_cursor_item->merge(foo.value());
                        if(foo->item_count == 0) {
                            foo.reset();
                        }
                    }
                    m_player.m_inventory.hotbar.items[i] = m_cursor_item;
                    m_cursor_item = foo;
                }
                else if (e->buttons() & Qt::RightButton) {
                    if(m_cursor_item.has_value()) {
                        if(!m_player.m_inventory.hotbar.items[i].has_value()) {
                            m_player.m_inventory.hotbar.items[i] = Item(this, m_cursor_item->type, 1, true);
                            m_cursor_item-> item_count--;

                            if(m_cursor_item->item_count == 0) m_cursor_item.reset();
                            else m_cursor_item->count_text.setText(std::to_string(m_cursor_item->item_count));
                        }
                        else if(m_player.m_inventory.hotbar.items[i]->type == m_cursor_item->type && m_player.m_inventory.hotbar.items[i] ->item_count < m_player.m_inventory.hotbar.items[i] -> max_count) {
                            m_cursor_item->item_count --;
                            if(m_cursor_item->item_count == 0) m_cursor_item.reset();
                            else m_cursor_item->count_text.setText(std::to_string(m_cursor_item->item_count));

                            m_player.m_inventory.hotbar.items[i]->item_count ++;
                            m_player.m_inventory.hotbar.items[i]->count_text.setText(std::to_string(m_player.m_inventory.hotbar.items[i]->item_count));
                        }
                    }
                    else {
                        if(m_player.m_inventory.hotbar.items[i].has_value()) {
                            int toMerge = (m_player.m_inventory.hotbar.items[i]->item_count+1)/2;
                            m_cursor_item = Item(this, m_player.m_inventory.hotbar.items[i]->type, toMerge, true);

                            m_player.m_inventory.hotbar.items[i]->item_count -= toMerge;

                            if(m_player.m_inventory.hotbar.items[i]->item_count == 0) m_player.m_inventory.hotbar.items[i].reset();
                            else m_player.m_inventory.hotbar.items[i]->count_text.setText(std::to_string(m_player.m_inventory.hotbar.items[i]->item_count));
                        }
                    }
                }
                break;
            }
        }
    }
    else {
        float bound = 4.f;
        if(e->buttons() & Qt::LeftButton)
        {
            glm::vec3 cam_pos = m_player.mcr_camera.mcr_position;
            glm::vec3 ray_dir = m_player.getLook() * bound;

            float hit = 3.f; float step = 0.3f;
            float cur = 0; bool found = false;
            glm::vec3 ray = glm::normalize(m_player.getLook());
            glm::vec3 hit_direction;
            m_multiplayers_mutex.lock();
            while (cur <= hit && !found) {
                glm::vec3 pt = m_player.mcr_camera.m_position + cur * ray;
                //qDebug() << "PT= " << pt.x << " " << pt.y << " " << pt.z;
                for (auto &a : m_multiplayers) {
                    Player* p = a.second.get();
                    //qDebug() << p->m_position.x << " " << p->m_position.y << " " << p->m_position.z;
                    if (p -> inBoundingBox(pt)) {
                        found = true; p -> hit = true;
                        hit_direction = p -> m_position - m_player.m_position;
                        HitPacket hp = HitPacket(0, a.first, hit_direction);
                        qDebug() << "bop!";
                        send_packet(&hp);
                        break;
                    }
                }
                cur += step;
            }
            m_multiplayers_mutex.unlock();
            if (!found) {
                float dist; glm::ivec3 block_pos; Direction d;
                if (m_terrain.gridMarch(cam_pos, ray_dir, &dist, &block_pos, d)) {
                    qDebug() << block_pos.x << " " << block_pos.y << " " << block_pos.z;
                    //m_terrain.changeBlockAt(block_pos.x, block_pos.y, block_pos.z, EMPTY);
                    //Chunk* c = m_terrain.getChunkAt(block_pos.x, block_pos.z).get();

                    m_terrain.changeBlockAt(block_pos.x, block_pos.y, block_pos.z, EMPTY);
                    m_terrain.renderChange(m_terrain.getChunkAt(block_pos.x, block_pos.z).get(), block_pos.x, block_pos.z);

                    BlockChangePacket bcp = BlockChangePacket(toKey(block_pos.x, block_pos.z), block_pos.y, EMPTY);
                    send_packet(&bcp);
                }
            }
        } else if (e->buttons() & Qt::RightButton) {
            glm::vec3 cam_pos = m_player.mcr_camera.mcr_position;
            glm::vec3 ray_dir = glm::normalize(m_player.getLook()) * bound;

            float dist;
            glm::ivec3 block_pos; Direction dir;
            bool found = m_terrain.gridMarch(cam_pos, ray_dir, &dist, &block_pos, dir);
            if (found) {
                //TO DO: make block placed be the block in the inventory slot
                BlockType type = m_terrain.getBlockAt(block_pos.x, block_pos.y, block_pos.z);
                glm::ivec3 neighbor = glm::ivec3(dirToVec(dir)) + block_pos;
                //m_terrain.setBlockAt(neighbor.x, neighbor.y, neighbor.z, type);
                qDebug() << block_pos.x << " " << block_pos.y << " " << block_pos.z;
                qDebug() << QString::fromStdString(glm::to_string(neighbor));
                qDebug() << dist;
                m_terrain.changeBlockAt(neighbor.x, neighbor.y, neighbor.z, type);
                m_terrain.renderChange(m_terrain.getChunkAt(neighbor.x, neighbor.z).get(), neighbor.x, neighbor.z);
                BlockChangePacket bcp = BlockChangePacket(toKey(neighbor.x, neighbor.z), neighbor.y, type);
                send_packet(&bcp);
            }
        }
    }
}

void MyGL::run_client() {
    QByteArray buffer;
    qDebug() << "client listening";
    buffer.resize(BUFFER_SIZE);
    while (open)
    {
        int bytes_received = recv(client_fd, buffer.data(), buffer.size(), 0);
        if (bytes_received < 0)
        {
            qDebug() << "Failed to receive message";
            break;
        }
        else if (bytes_received == 0)
        {
            qDebug() << "Connection closed by server";
            break;
        }
        else
        {
            buffer.resize(bytes_received);
            Packet* pp = bufferToPacket(buffer);
            packet_processer(pp);
            delete(pp);
            buffer.resize(BUFFER_SIZE);
        }
    }
}

void MyGL::init_client() {
    // create client socket
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        qDebug() << "Failed to create client socket";
        return;
    }

    // connect to server
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    if (inet_pton(AF_INET, &ip[0], &server_address.sin_addr) <= 0)
    {
        qDebug() << "Invalid server address";
        return;
    }
    if (::connect(client_fd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0)
    {
        qDebug() << "Failed to connect to server";
        return;
    }

    // create thread to receive messages
    open = true;
    ClientWorker* cw = new ClientWorker(this);
    QThreadPool::globalInstance()->start(cw);
}

void MyGL::

send_packet(Packet* packet) {
    QByteArray buffer;
    switch(packet->type) {
    case PLAYER_STATE:{
        buffer = (dynamic_cast<PlayerStatePacket*>(packet))->packetToBuffer();
        break;
    }
    case PLAYER_JOIN: {
        buffer = (dynamic_cast<PlayerJoinPacket*>(packet))->packetToBuffer();
        break;
    }
    case CHAT: {
        buffer = (dynamic_cast<ChatPacket*>(packet))->packetToBuffer();
        break;
    }
    case BLOCK_CHANGE: {
        buffer = (dynamic_cast<BlockChangePacket*>(packet))->packetToBuffer();
        break;
    }
    case ITEM_ENTITY_STATE: {
        buffer = (dynamic_cast<ItemEntityStatePacket*>(packet))->packetToBuffer();
        break;
    }
    case HIT: {
        buffer = (dynamic_cast<HitPacket*>(packet))->packetToBuffer();
        break;
    }
    case PLAYER_RESPAWN: {
        buffer = (dynamic_cast<RespawnPacket*>(packet))->packetToBuffer();
        break;
    }
    default:
        break;
    }
    int bytes_sent = send(client_fd, buffer.data(), buffer.size(), 0);
}

void MyGL::close_client() {
    open = false;
}

void MyGL::packet_processer(Packet* packet) {
    switch(packet->type) {
    case PLAYER_STATE:{
        PlayerStatePacket* thispack = dynamic_cast<PlayerStatePacket*>(packet);
        m_multiplayers_mutex.lock();
        if(m_multiplayers.find(thispack->player_id) != m_multiplayers.end()) {
            m_multiplayers[thispack->player_id]->setState(thispack->player_pos, thispack->player_velo, thispack->player_theta, thispack->player_phi, thispack->player_hand);
            m_multiplayers[thispack->player_id]->inHand = thispack->player_hand;
            m_multiplayers[thispack->player_id]->isDead = false;
            itemQueue_mutex.lock();
            if(thispack->player_helmet == AIR) {
                m_multiplayers[thispack->player_id]->m_inventory.armor[0].reset();
            }
            else {
                m_multiplayers[thispack->player_id]->m_inventory.armor[0] = Item(this, thispack->player_helmet, 1, false);
                itemQueue.push(&m_multiplayers[thispack->player_id]->m_inventory.armor[0].value());
            }
            if(thispack->player_chest == AIR) {
                m_multiplayers[thispack->player_id]->m_inventory.armor[1].reset();
            }
            else {
                m_multiplayers[thispack->player_id]->m_inventory.armor[1] = Item(this, thispack->player_chest, 1, false);
                itemQueue.push(&m_multiplayers[thispack->player_id]->m_inventory.armor[1].value());
            }
            if(thispack->player_leg == AIR) {
                m_multiplayers[thispack->player_id]->m_inventory.armor[2].reset();
            }
            else {
                m_multiplayers[thispack->player_id]->m_inventory.armor[2] = Item(this, thispack->player_leg, 1, false);
                itemQueue.push(&m_multiplayers[thispack->player_id]->m_inventory.armor[2].value());
            }
            if(thispack->player_boots == AIR) {
                m_multiplayers[thispack->player_id]->m_inventory.armor[3].reset();
            }
            else {
                m_multiplayers[thispack->player_id]->m_inventory.armor[3] = Item(this, thispack->player_boots, 1, false);
                itemQueue.push(&m_multiplayers[thispack->player_id]->m_inventory.armor[3].value());
            }
            itemQueue_mutex.unlock();
        }
        m_multiplayers_mutex.unlock();
        break;
    }
    case WORLD_INIT:{
        WorldInitPacket* thispack = dynamic_cast<WorldInitPacket*>(packet);
        //TO do: set seed somewhere
        m_time = thispack->time;
        m_terrain.worldSpawn = thispack->spawn;
        client_id = thispack->pid;
        verified_server = true;
        m_multiplayers_mutex.lock();
        for(std::pair<int, QString> pp: thispack->players) {
            m_multiplayers[pp.first] = mkU<Player>(Player(glm::vec3(0), nullptr, this, pp.second));
        }
        m_multiplayers_mutex.unlock();
        break;
    }
    case PLAYER_JOIN:{
        PlayerJoinPacket* thispack = dynamic_cast<PlayerJoinPacket*>(packet);
        if(thispack->join){
            chatQueue_mutex.lock();
            chatQueue.push(std::make_pair(thispack->name.toStdString()+" joined the game.", glm::vec4(1,1,0,0)));
            chatQueue_mutex.unlock();
            m_multiplayers_mutex.lock();
            m_multiplayers[thispack->player_id] = mkU<Player>(Player(glm::vec3(0), nullptr, this, thispack->name));
            m_multiplayers_mutex.unlock();
        }
        else {
            chatQueue_mutex.lock();
            chatQueue.push(std::make_pair(thispack->name.toStdString()+" has left the game.", glm::vec4(1,1,0,0)));
            chatQueue_mutex.unlock();
            m_multiplayers_mutex.lock();
            m_multiplayers.erase(thispack->player_id);
            m_multiplayers_mutex.unlock();
        }
        break;
    }
    case CHUNK_CHANGE:{
        ChunkChangePacket* thispack = dynamic_cast<ChunkChangePacket*>(packet);
        glm::ivec2 chunkp = toCoords(thispack->chunkPos);
        qDebug() << "chunk change received";
        for(std::pair<vec3, BlockType> &p: thispack->changes) {
            qDebug() << p.first.x << p.first.y << p.first.z;
            m_terrain.changeBlockAt(chunkp.x+p.first.x, p.first.y, chunkp.y+p.first.z, p.second);
        }
        break;
    }
    case BLOCK_CHANGE:{
        BlockChangePacket* thispack = dynamic_cast<BlockChangePacket*>(packet);
        glm::ivec2 xz = toCoords(thispack->chunkPos);
        qDebug() << xz.x << thispack->yPos << xz.y << thispack->newBlock;
        if(m_terrain.hasChunkAt(xz.x, xz.y) && m_terrain.getBlockAt(xz.x, thispack->yPos, xz.y) == thispack->newBlock) return;
        m_terrain.changeBlockAt(xz.x, thispack->yPos, xz.y, thispack->newBlock);

        m_terrain.renderChange(m_terrain.getChunkAt(xz.x, xz.y).get(), xz.x, xz.y);
        break;
    }
    case CHAT: {
        ChatPacket* thispack = dynamic_cast<ChatPacket*>(packet);
        //make names for player class later
        std::string s1 = m_multiplayers[thispack->player_id]->name.toStdString();
        std::string s2 = thispack->message.toStdString();
        chatQueue.push(std::make_pair(s1+": "+s2, glm::vec4(1)));
        break;
    }
    case ITEM_ENTITY_STATE: {
        ItemEntityStatePacket* thispack = dynamic_cast<ItemEntityStatePacket*>(packet);
        m_terrain.item_entities_mutex.lock();
        m_terrain.item_entities.insert(std::make_pair(thispack->entity_id, ItemEntity(thispack->pos, Item(this, thispack->type, thispack->count, false), this)));
        itemQueue_mutex.lock();
        itemQueue.push(&(m_terrain.item_entities.at(thispack->entity_id).item));
        itemQueue_mutex.unlock();
        m_terrain.item_entities_mutex.unlock();
        break;
    }
    case DELETE_ITEM_ENTITY: {
        ItemEntityDeletePacket* thispack = dynamic_cast<ItemEntityDeletePacket*>(packet);
        m_terrain.item_entities_mutex.lock();
        m_terrain.item_entities.erase(thispack->entity_id);
        m_terrain.item_entities_mutex.unlock();
        break;
    }
    case HIT: {
        HitPacket* thispack = dynamic_cast<HitPacket*>(packet);
        if(thispack->target == client_id) {
            qDebug() << thispack->damage;
            m_player.knockback(thispack->direction);
            m_player.health = max(0, m_player.health-thispack->damage);
        }
        else {
            // TO DO: make other player blush
        }
        break;
    }
    case PLAYER_DEATH: {
        DeathPacket* thispack = dynamic_cast<DeathPacket*>(packet);
        std::string s1, s2;
        qDebug() << "me" << client_id;
        qDebug() << "dead" << thispack->victim << thispack->killer;
        if(thispack->victim == client_id) {
            s1 = m_player.name.toStdString();
            m_player.isDead = true;
        }
        else if(m_multiplayers[thispack->victim]) {
            s1 = m_multiplayers[thispack->victim]->name.toStdString();
            m_multiplayers[thispack->victim]->isDead = true;
        }
        if(thispack->killer == client_id) s2 = m_player.name.toStdString();
        else if(m_multiplayers[thispack->killer]) s2 = m_multiplayers[thispack->killer]->name.toStdString();
        if(thispack->killer == thispack->victim) {
            chatQueue.push(std::make_pair(s1 + " commit toaster bath", glm::vec4(1)));
        }
        else {
            chatQueue.push(std::make_pair(s1 + " was slain by " + s2, glm::vec4(1)));
        }
        break;
    }
    default:
        qDebug() << "unknown packet type: " << packet->type;
        break;
    }
}

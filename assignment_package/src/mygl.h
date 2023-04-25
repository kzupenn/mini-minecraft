#ifndef MYGL_H
#define MYGL_H

#include "openglcontext.h"
#include "scene/crosshair.h"
#include "scene/icons.h"
#include "scene/rectangle.h"
#include "shaderprogram.h"
#include "scene/worldaxes.h"
#include "scene/camera.h"
#include "scene/terrain.h"
#include "scene/player.h"
#include "texture.h"
#include "server/server.h"
#include "framebuffer.h"
#include "quad.h"
#include "scene/cube.h"

#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <netinet/in.h>
#include <queue>
#include <smartpointerhelp.h>

#include "server/port.h"
#define BUFFER_SIZE 5000

class MyGL : public OpenGLContext
{
    Q_OBJECT
private:
    glm::mat4 overlayTransform;
    WorldAxes m_worldAxes; // A wireframe representation of the world axes. It is hard-coded to sit centered at (32, 128, 32).
    Crosshair m_crosshair; //crosshair so we know where we're looking
    ShaderProgram m_progLambert;// A shader program that uses lambertian reflection
    ShaderProgram m_progFlat;// A shader program that uses "flat" reflection (no shadowing at all)
    ShaderProgram m_progOverlay; //for overlays
    ShaderProgram m_progInstanced;// A shader program that is designed to be compatible with instanced rendering
    ShaderProgram m_progPostProcess;
    ShaderProgram m_progSky;


    GLuint vao; // A handle for our vertex array object. This will store the VBOs created in our geometry classes.
                // Don't worry too much about this. Just know it is necessary in order to render geometry.

    Terrain m_terrain; // All of the Chunks that currently comprise the world.
    Player m_player; // The entity controlled by the user. Contains a camera to display what it sees as well.
    FrameBuffer m_frame;
    Quad m_quad;
    Quad m_sky;

    InputBundle m_inputs; // A collection of variables to be updated in keyPressEvent, mouseMoveEvent, mousePressEvent, etc.
    std::optional<Item> m_cursor_item; // Item player is handling in inventory mode

    QTimer m_timer; // Timer linked to tick(). Fires approximately 60 times per second.
    int m_time; //to get tick number
    bool mouseMove;

    Texture m_block_texture;
    Texture m_font_texture;
    Texture m_inventory_texture;
    Texture m_icon_texture;

    Heart m_heart;
    HalfHeart m_halfheart;
    FullHeart m_fullheart;

    Armor m_armor;
    HalfArmor m_halfarmor;
    FullArmor m_fullarmor;
    
    Texture m_skin_texture;

    long long m_currentMSecsSinceEpoch;

    //multiplayer stuffs
    //client vars
    void receive_messages(void*arg);

    int client_fd; //socket connection
    int client_id; //server-side client fd, used as a player id
    struct sockaddr_in server_address;
    bool open;

    std::mutex m_chat_mutex;
    std::deque<Font> m_chat;
    Font m_mychat;
    //if the player is in chat mode, redirect all keyboard inputs to chat
    bool chatMode;
    //the player dies when they are killed
    bool isDead;
    //a gray transparent rectangle, used primarily for font background
    Rectangle m_rectangle;

    void packet_parser(Packet*);
    void init_client(); //initializes the client
    void send_packet(Packet*);
    void close_client(); //closes client

    //needed to bind vbos on main thread
    std::mutex chatQueue_mutex;
    std::queue<std::pair<std::string, glm::vec4>> chatQueue;

    //drawing items in main thread
    std::mutex itemQueue_mutex;
    std::queue<Drawable*> itemQueue;

    //server, if hosting
    uPtr<Server> SERVER;

    //info from server
    std::atomic_bool verified_server;
    std::mutex m_multiplayers_mutex;
    std::map<int, uPtr<Player>> m_multiplayers;
    std::mutex m_entites_mutex;
    std::vector<Entity> m_entities; //collection of all non-player entities


    void moveMouseToCenter(); // Forces the mouse position to the screen's center. You should call this
                              // from within a mouse move event after reading the mouse movement so that
                              // your mouse stays within the screen bounds and is always read.

    void sendPlayerDataToGUI() const;
    Font deathMsg1, deathMsg2;

public:
    explicit MyGL(QWidget *parent = nullptr);
    ~MyGL();

    // Called once when MyGL is initialized.
    // Once this is called, all OpenGL function
    // invocations are valid (before this, they
    // will cause segfaults)
    void initializeGL() override;
    // Called whenever MyGL is resized.
    void resizeGL(int w, int h) override;
    // Called whenever MyGL::update() is called.
    // In the base code, update() is called from tick().
    void paintGL() override;

    //servers
    void start(bool isMultiplayer, QString username); //starts the client
    void run_client(); //runs the client
    void packet_processer(Packet*);
    std::string ip;

    // Called from paintGL().
    // Calls Terrain::draw().
    void renderTerrain();
    void renderEntities();
    void renderOverlays();
    void setupTerrainThreads();


protected:
    // Automatically invoked when the user
    // presses a key on the keyboard
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);

    // Automatically invoked when the user
    // moves the mouse
    void updateMouse();
    // Automatically invoked when the user
    // presses a mouse button
    void mousePressEvent(QMouseEvent *e);


private slots:
    void tick(); // Slot that gets called ~60 times per second by m_timer firing.

signals:
    void sig_sendPlayerPos(QString) const;
    void sig_sendPlayerVel(QString) const;
    void sig_sendPlayerAcc(QString) const;
    void sig_sendPlayerLook(QString) const;
    void sig_sendPlayerChunk(QString) const;
    void sig_sendPlayerTerrainZone(QString) const;
    void sig_sendServerIP(QString) const;
};


#endif // MYGL_H

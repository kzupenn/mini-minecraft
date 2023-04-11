#include "player.h"
#include <QString>
#include <iostream>

Player::Player(glm::vec3 pos, const Terrain &terrain, OpenGLContext* m_context)
    : Entity(pos, m_context), m_velocity(0,0,0), m_acceleration(0,0,0),
      m_camera(pos + glm::vec3(0, 3.5f, 0)), mcr_terrain(terrain),
      theta(0), phi(0), mcr_camera(m_camera), m_flightMode(true),
      airtime(0), maxair(45), in_liquid(false), bott_in_liquid(false)
{}

Player::~Player()
{}

void Player::tick(float dT, InputBundle &input) {
    processInputs(input);
    orientCamera();
    computePhysics(dT);
}

void Player::orientCamera() {
    glm::mat4 trot = glm::rotate(glm::mat4(1.f), glm::radians(theta), glm::vec3(0, 1, 0));
    glm::mat4 prot = glm::rotate(glm::mat4(1.f), glm::radians(phi), glm::vec3(1, 0, 0));
    m_camera.m_forward = glm::vec3(trot * prot * glm::vec4(0, 0, -1, 0));
    m_camera.m_right = glm::vec3(trot * prot * glm::vec4(1, 0, 0, 0));
    m_camera.m_up = glm::vec3(trot * prot * glm::vec4(0, 1, 0, 0));
    m_forward = m_camera.m_forward;
    m_right = m_camera.m_right;
    m_up = m_camera.m_up;
}

void Player::processInputs(InputBundle &inputs) {
    // TODO: Update the Player's velocity and acceleration based on the
    // state of the inputs.
    float SPEED = 2;
    if (m_flightMode) SPEED *= 5;
    if (inputs.mouseX) {
        theta -= inputs.mouseX;
        if (theta > 360 || theta < -360) theta = 0;
        inputs.mouseX = 0.f;
    }
    if (inputs.mouseY) {
        phi = glm::clamp(phi - inputs.mouseY, -90.f, 90.f);
        inputs.mouseY = 0.f;
    }
    if (inputs.fPressed) {
        m_flightMode = !m_flightMode;
        inputs.fPressed = false;
    }
    if (inputs.wPressed) {
        if (m_flightMode) m_acceleration += m_forward * SPEED;
        else m_acceleration += glm::normalize(
                                glm::vec3(m_forward.x, 0, m_forward.z)) * SPEED;
    }
    if (inputs.sPressed) {
        if (m_flightMode) m_acceleration -= m_forward * SPEED;
        else m_acceleration -= glm::normalize(
                                glm::vec3(m_forward.x, 0, m_forward.z)) * SPEED;
    }
    if (inputs.dPressed) {
        if (m_flightMode) m_acceleration += m_right * SPEED;
        else m_acceleration += glm::normalize(
                                glm::vec3(m_right.x, 0, m_right.z)) * SPEED;
    }
    if (inputs.aPressed) {
        if (m_flightMode) m_acceleration -= m_right * SPEED;
        else m_acceleration -= glm::normalize(
                                glm::vec3(m_right.x, 0, m_right.z)) * SPEED;
    }
    if (inputs.ePressed && m_flightMode) {
        m_acceleration += glm::vec3(0, 1, 0) * SPEED;
    }
    if (inputs.qPressed && m_flightMode) {
        m_acceleration -= glm::vec3(0, 1, 0) * SPEED;
    }
    if (inputs.spacePressed) {
        if (m_flightMode) m_acceleration += glm::vec3(0, 1, 0) * SPEED * 1.5f;
        else if (bott_in_liquid) m_acceleration += glm::vec3(0, 1, 0) * SPEED * 1.5f;
        else if (!checkAirborne()) airtime = maxair;
    }
}

bool Player::checkAirborne() {
    std::vector<glm::vec3> corners = {glm::vec3(m_position.x + 0.3, m_position.y, m_position.z + 0.3),
                                     glm::vec3(m_position.x - 0.3, m_position.y, m_position.z + 0.3),
                                     glm::vec3(m_position.x + 0.3, m_position.y, m_position.z - 0.3),
                                     glm::vec3(m_position.x - 0.3, m_position.y, m_position.z - 0.3)};
    glm::vec3 down(0, -0.0001, 0);
    for (auto &c : corners) {
        float dist; glm::ivec3 outblock; Direction d;
        if (mcr_terrain.gridMarch(c, down, &dist, &outblock, d)) return false;
    }
    return true;
}

void Player::computePhysics(float dT) {
    // TODO: Update the PlayerE's position based on its acceleration
    // and velocity, and also perform collision detection.
    m_velocity *= 0.8f;
    m_velocity += m_acceleration * dT;
    if (!m_flightMode) {
        if (airtime > 0) {
            m_velocity += glm::vec3(0, 1, 0) * 3.8f * dT * airtime / (maxair / 1.5f);
            airtime--;
        }
        if (bott_in_liquid) m_velocity += glm::vec3(0, -1.1f, 0) * dT;
        else m_velocity += glm::vec3(0, -4.1f, 0) * dT;
        checkCollision();
    }
    if (in_liquid) moveAlongVector(m_velocity / 1.5f);
    else moveAlongVector(m_velocity);
    glm::vec3 cur = m_camera.m_position;
    if (mcr_terrain.hasChunkAt(cur.x, cur.z)) camera_block = mcr_terrain.getBlockAt(cur);
//    qDebug() << m_position.x << " " << m_position.y << " " << m_position.z;
    m_acceleration = glm::vec3(0);
}

void Player::checkCollision()
{
    glm::vec3 p = m_position;
    std::vector<glm::vec3> corners = {glm::vec3(p.x+0.3, p.y+1.8f, p.z-0.3),
                                     glm::vec3(p.x+0.3, p.y+1.8f, p.z+0.3),
                                     glm::vec3(p.x-0.3, p.y+1.8f, p.z+0.3),
                                     glm::vec3(p.x-0.3, p.y+1.8f, p.z-0.3),
                                     glm::vec3(p.x+0.3, p.y+1, p.z-0.3),
                                     glm::vec3(p.x+0.3, p.y+1, p.z+0.3),
                                     glm::vec3(p.x-0.3, p.y+1, p.z+0.3),
                                     glm::vec3(p.x-0.3, p.y+1, p.z-0.3),
                                     glm::vec3(p.x+0.3, p.y, p.z-0.3),
                                     glm::vec3(p.x+0.3, p.y, p.z+0.3),
                                     glm::vec3(p.x-0.3, p.y, p.z+0.3),
                                     glm::vec3(p.x-0.3, p.y, p.z-0.3)};
    bool liquid = true;
    in_liquid = false; bott_in_liquid = false;
    for (int i = 0; i < corners.size(); i++) {
        if (i % 4 == 3) {
            in_liquid = in_liquid || liquid;
            bott_in_liquid = bott_in_liquid || (i < 4 && in_liquid);
            liquid = true;
        }
        glm::vec3 cur = corners[i];
        BlockType bt = mcr_terrain.getBlockAt(cur.x, cur.y, cur.z);
        liquid = liquid && (bt == WATER || bt == LAVA);
    }

    glm::vec3 min = glm::vec3(m_velocity.x, m_velocity.y, m_velocity.z);
    for (auto &origin : corners) {            
        float x, y, z;
        glm::ivec3 b;
        Direction d;
        bool xF = mcr_terrain.gridMarch(origin, glm::vec3(m_velocity.x, 0, 0),
                                    &x, &b, d);
        bool yF = mcr_terrain.gridMarch(origin, glm::vec3(0, m_velocity.y, 0),
                                    &y, &b, d);
        bool zF = mcr_terrain.gridMarch(origin, glm::vec3(0, 0, m_velocity.z),
                                    &z, &b, d);

        if (xF && x < glm::abs(min.x)) {
            min.x = x * glm::sign(min.x);
        }
        if (yF && y < glm::abs(min.y)) {
            min.y = y * glm::sign(min.y);
        }
        if (zF && z < glm::abs(min.z)) {
            min.z = z * glm::sign(min.z);
        }
    }
    m_velocity.x = min.x;
    m_velocity.y = min.y;
    m_velocity.z = min.z;
}


void Player::setCameraWidthHeight(unsigned int w, unsigned int h) {
    m_camera.setWidthHeight(w, h);
}

void Player::moveAlongVector(glm::vec3 dir) {
    Entity::moveAlongVector(dir);
    m_camera.moveAlongVector(dir);
}
void Player::moveForwardLocal(float amount) {
    Entity::moveForwardLocal(amount);
    m_camera.moveForwardLocal(amount);
}
void Player::moveRightLocal(float amount) {
    Entity::moveRightLocal(amount);
    m_camera.moveRightLocal(amount);
}
void Player::moveUpLocal(float amount) {
    Entity::moveUpLocal(amount);
    m_camera.moveUpLocal(amount);
}
void Player::moveForwardGlobal(float amount) {
    Entity::moveForwardGlobal(amount);
    m_camera.moveForwardGlobal(amount);
}
void Player::moveRightGlobal(float amount) {
    Entity::moveRightGlobal(amount);
    m_camera.moveRightGlobal(amount);
}
void Player::moveUpGlobal(float amount) {
    Entity::moveUpGlobal(amount);
    m_camera.moveUpGlobal(amount);
}
void Player::rotateOnForwardLocal(float degrees) {
    Entity::rotateOnForwardLocal(degrees);
    m_camera.rotateOnForwardLocal(degrees);
}
void Player::rotateOnRightLocal(float degrees) {
    Entity::rotateOnRightLocal(degrees);
    m_camera.rotateOnRightLocal(degrees);
}
void Player::rotateOnUpLocal(float degrees) {
    Entity::rotateOnUpLocal(degrees);
    m_camera.rotateOnUpLocal(degrees);
}
void Player::rotateOnForwardGlobal(float degrees) {
    Entity::rotateOnForwardGlobal(degrees);
    m_camera.rotateOnForwardGlobal(degrees);
}
void Player::rotateOnRightGlobal(float degrees) {
    Entity::rotateOnRightGlobal(degrees);
    m_camera.rotateOnRightGlobal(degrees);
}
void Player::rotateOnUpGlobal(float degrees) {
    Entity::rotateOnUpGlobal(degrees);
    m_camera.rotateOnUpGlobal(degrees);
}

QString Player::posAsQString() const {
    std::string str("( " + std::to_string(m_position.x) + ", " + std::to_string(m_position.y) + ", " + std::to_string(m_position.z) + ")");
    return QString::fromStdString(str);
}
QString Player::velAsQString() const {
    std::string str("( " + std::to_string(m_velocity.x) + ", " + std::to_string(m_velocity.y) + ", " + std::to_string(m_velocity.z) + ")");
    return QString::fromStdString(str);
}
QString Player::accAsQString() const {
    std::string str("( " + std::to_string(m_acceleration.x) + ", " + std::to_string(m_acceleration.y) + ", " + std::to_string(m_acceleration.z) + ")");
    return QString::fromStdString(str);
}
QString Player::lookAsQString() const {
    std::string str("( " + std::to_string(m_forward.x) + ", " + std::to_string(m_forward.y) + ", " + std::to_string(m_forward.z) + ")");
    return QString::fromStdString(str);
}

void Player::setType(BlockType bt) {
    camera_block = bt;
}

int Player::getType() {
    if (camera_block == EMPTY) return 0;
    else if (camera_block == WATER) return 1;
    else if (camera_block == LAVA) return 2;
    else return 3;
}

glm::vec3 Player::getLook()
{
    return this->m_forward;
}

float Player::getPhi() {
    return phi;
}

float Player::getTheta() {
    return theta;
}

void Player::setState(glm::vec3 p, float f1, float f2) {
    m_position = p;
    m_camera.setPos(p + glm::vec3(0, 1.5f, 0));
    theta = f1;
    phi = f2;
}

void Player::createVBOdata() {
    std::vector<glm::vec4> pos, nor, col, inter;
    std::vector<int> idx;
    //Front face
    //UR
    pos.emplace_back(1.0f, 1.0f, 1.0f, 1.0f);
    //LR
    pos.emplace_back(1.0f, 0.0f, 1.0f, 1.0f);
    //LL
    pos.emplace_back(0.0f, 0.0f, 1.0f, 1.0f);
    //UL
    pos.emplace_back(0.0f, 1.0f, 1.0f, 1.0f);

    //Right face
    //UR
    pos.emplace_back(1.0f, 1.0f, 0.0f, 1.0f);
    //LR
    pos.emplace_back(1.0f, 0.0f, 0.0f, 1.0f);
    //LL
    pos.emplace_back(1.0f, 0.0f, 1.0f, 1.0f);
    //UL
    pos.emplace_back(1.0f, 1.0f, 1.0f, 1.0f);

    //Left face
    //UR
    pos.emplace_back(0.0f, 1.0f, 1.0f, 1.0f);
    //LR
    pos.emplace_back(0.0f, 0.0f, 1.0f, 1.0f);
    //LL
    pos.emplace_back(0.0f, 0.0f, 0.0f, 1.0f);
    //UL
    pos.emplace_back(0.0f, 1.0f, 0.0f, 1.0f);

    //Back face
    //UR
    pos.emplace_back(0.0f, 1.0f, 0.0f, 1.0f);
    //LR
    pos.emplace_back(0.0f, 0.0f, 0.0f, 1.0f);
    //LL
    pos.emplace_back(1.0f, 0.0f, 0.0f, 1.0f);
    //UL
    pos.emplace_back(1.0f, 1.0f, 0.0f, 1.0f);

    //Top face
    //UR
    pos.emplace_back(1.0f, 1.0f, 0.0f, 1.0f);
    //LR
    pos.emplace_back(1.0f, 1.0f, 1.0f, 1.0f);
    //LL
    pos.emplace_back(0.0f, 1.0f, 1.0f, 1.0f);
    //UL
    pos.emplace_back(0.0f, 1.0f, 0.0f, 1.0f);

    //Bottom face
    //UR
    pos.emplace_back(1.0f, 0.0f, 1.0f, 1.0f);
    //LR
    pos.emplace_back(1.0f, 0.0f, 0.0f, 1.0f);
    //LL
    pos.emplace_back(0.0f, 0.0f, 0.0f, 1.0f);
    //UL
    pos.emplace_back(0.0f, 0.0f, 1.0f, 1.0f);

//    for(int i = 0; i < pos.size(); i++) {
//        pos[i] += glm::vec4(m_position, 1.f);
//        //qDebug() << pos[i].x << " " << pos[i].y << " " << pos[i].z << " ";
//    }

    //Front
    for(int i = 0; i < 4; i++){
        nor.emplace_back(0,0,1,0);
    }
    //Right
    for(int i = 0; i < 4; i++){
        nor.emplace_back(1,0,0,0);
    }
    //Left
    for(int i = 0; i < 4; i++){
        nor.emplace_back(-1,0,0,0);
    }
    //Back
    for(int i = 0; i < 4; i++){
        nor.emplace_back(0,0,-1,0);
    }
    //Top
    for(int i = 0; i < 4; i++){
        nor.emplace_back(0,1,0,0);
    }
    //Bottom
    for(int i = 0; i < 4; i++){
        nor.emplace_back(0,-1,0,0);
    }

    //colors
    for(int i = 0; i < pos.size(); i++) {
        col.emplace_back(1, 0, 0, 1);
    }

    for(int i = 0; i < 6; i++){
        idx.push_back(i*4);
        idx.push_back(i*4+1);
        idx.push_back(i*4+2);
        idx.push_back(i*4);
        idx.push_back(i*4+2);
        idx.push_back(i*4+3);
    }

    m_count = idx.size();

    for (int i = 0; i < pos.size(); i++) {
       inter.push_back(pos[i]);
       inter.push_back(nor[i]);
       inter.push_back(col[i]);
    }

    generateIdx();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size()* sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    generateInter();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufInter);
    mp_context->glBufferData(GL_ARRAY_BUFFER, inter.size() * sizeof(glm::vec4), inter.data(), GL_STATIC_DRAW);

}
GLenum Player::drawMode() {
    return GL_TRIANGLES;
}

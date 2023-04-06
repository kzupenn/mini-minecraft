#include "player.h"
#include <QString>
#include <iostream>

Player::Player(glm::vec3 pos, const Terrain &terrain)
    : Entity(pos), m_velocity(0,0,0), m_acceleration(0,0,0),
      m_camera(pos + glm::vec3(0, 1.5f, 0)), mcr_terrain(terrain),
      theta(0), phi(0), mcr_camera(m_camera), m_flightMode(true),
      airtime(0), maxair(45)
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
        float dist; glm::ivec3 outblock;
        if (mcr_terrain.gridMarch(c, down, &dist, &outblock, false)) return false;
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
        m_velocity += glm::vec3(0, -4.1f, 0) * dT;
        checkCollision();
    }
    moveAlongVector(m_velocity);
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

    glm::vec3 min = glm::vec3(m_velocity.x, m_velocity.y, m_velocity.z);

    for (auto &origin : corners) {
        float x, y, z;
        glm::ivec3 b;
        bool xF = mcr_terrain.gridMarch(origin, glm::vec3(m_velocity.x, 0, 0),
                                    &x, &b, false);
        bool yF = mcr_terrain.gridMarch(origin, glm::vec3(0, m_velocity.y, 0),
                                    &y, &b, false);
        bool zF = mcr_terrain.gridMarch(origin, glm::vec3(0, 0, m_velocity.z),
                                    &z, &b, false);

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

glm::vec3 Player::getLook()
{
    return this->m_forward;
}

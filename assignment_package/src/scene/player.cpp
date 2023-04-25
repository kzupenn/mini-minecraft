#include "player.h"
#include <QString>
#include <iostream>


Player::Player(glm::vec3 pos, const Terrain &terrain, OpenGLContext* m_context, QString n)
    : Entity(pos), m_inventory(m_context, 27, true), m_velocity(0,0,0), m_acceleration(0,0,0),
      m_camera(pos + glm::vec3(0, 1.5f, 0)), mcr_terrain(terrain),
      theta(0), phi(0), mcr_camera(m_camera), m_flightMode(true),
      airtime(0), maxair(40), in_liquid(false), bott_in_liquid(false), name(n),
      head(Prism(m_context, glm::ivec3(8, 8, 8), glm::ivec2(0, 0), glm::ivec2(31, 15))),
      torso(Prism(m_context, glm::ivec3(8, 12, 4), glm::ivec2(16, 16), glm::ivec2(39, 31))),
      right_arm(Prism(m_context, glm::ivec3(4, 12, 4), glm::ivec2(40, 16), glm::ivec2(55, 31))),
      right_leg(Prism(m_context, glm::ivec3(4, 12, 4), glm::ivec2(0, 16), glm::ivec2(15, 31))),
      left_arm(Prism(m_context, glm::ivec3(4, 12, 4), glm::ivec2(40, 16), glm::ivec2(55, 31))),
      left_leg(Prism(m_context, glm::ivec3(4, 12, 4), glm::ivec2(0, 16), glm::ivec2(15, 31))),
      parts {head, torso, right_arm, right_leg, left_arm, left_leg},
      display(m_context), start_swing(0), swinging(false), stopped(true),
      created(false), swing_dir(1), shift(false), hit(0),
      health(20), armor(0), inHand(AIR)
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
    float SPEED = 1.25f;
    if (m_flightMode) SPEED *= 4;
    if (inputs.mouseX) {
        theta -= inputs.mouseX;
        if (theta > 360 || theta < -360) theta = 0;
        inputs.mouseX = 0.f;
    }
    if (inputs.mouseY) {
        float eps = 0.001;
        phi = glm::clamp(phi - inputs.mouseY, -90.f + eps, 90.f - eps);
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
    if (inputs.spacePressed && m_flightMode) {
        m_acceleration += glm::vec3(0, 1, 0) * SPEED;
    }
    if (inputs.lshiftPressed) {
        if (m_flightMode) m_acceleration -= glm::vec3(0, 1, 0) * SPEED;
        else shift = true;
    }
    if (inputs.spacePressed) {
        if (m_flightMode) m_acceleration += glm::vec3(0, 1, 0) * SPEED * 1.5f;
        else if (bott_in_liquid) m_acceleration += glm::vec3(0, 1, 0) * SPEED * 1.3f;
        else if (!checkAirborne()) airtime = maxair;
    }
}

bool Player::checkAirborne() {
    std::vector<glm::vec3> corners = {glm::vec3(m_position.x + 0.3, m_position.y, m_position.z + 0.3),
                                     glm::vec3(m_position.x - 0.3, m_position.y, m_position.z + 0.3),
                                     glm::vec3(m_position.x + 0.3, m_position.y, m_position.z - 0.3),
                                     glm::vec3(m_position.x - 0.3, m_position.y, m_position.z - 0.3)};

    glm::vec3 down(0, -0.125, 0);
    for (auto &c : corners) {
        float dist; glm::ivec3 outblock; Direction d;
        mcr_terrain.gridMarch(c, down, &dist, &outblock, d);
        if (dist <= 0.10) return false;
    }
    return true;
}

void Player::computePhysics(float dT) {
    // TODO: Update the PlayerE's position based on its acceleration
    // and velocity, and also perform collision detection.
    m_velocity *= 0.8f;
    m_velocity += m_acceleration * dT;
    if (!m_flightMode && mcr_terrain.hasChunkAt(m_position.x, m_position.z)) {
        if (airtime > 0) {
            m_velocity += glm::vec3(0, 1, 0) * 3.95f * dT * airtime / (maxair / 1.5f);
            airtime--;
        }
        if (hit > 0) hit--;
        if (bott_in_liquid) m_velocity += glm::vec3(0, -0.9f, 0) * dT;
        else m_velocity += glm::vec3(0, -4.1f, 0) * dT;
        checkCollision();
    }
    glm::vec3 vout = m_velocity;
    if (shift) {
        vout.x *= 1.75;
        vout.z *= 1.75;
        shift = false;
    }
    if (in_liquid) vout /= 1.5f;
    moveAlongVector(vout);
    glm::vec3 cur = m_camera.m_position;
    if (mcr_terrain.hasChunkAt(cur.x, cur.z)) camera_block = mcr_terrain.getBlockAt(cur);
    m_acceleration = glm::vec3(0);
}

void Player::checkCollision()
{
    glm::vec3 p = m_position;
    std::vector<glm::vec3> corners = {glm::vec3(p.x+0.3, p.y+2.f, p.z-0.3),
                                     glm::vec3(p.x+0.3, p.y+2.f, p.z+0.3),
                                     glm::vec3(p.x-0.3, p.y+2.f, p.z+0.3),
                                     glm::vec3(p.x-0.3, p.y+2.f, p.z-0.3),
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
            liquid = true;
        }
        glm::vec3 cur = corners[i];
        BlockType bt = mcr_terrain.getBlockAt(cur.x, cur.y, cur.z);
        liquid = liquid && (bt == WATER || bt == LAVA);
    }
    for (int i = 0; i < 4; i++) {
        glm::vec3 cur = corners[i];
        BlockType bt = mcr_terrain.getBlockAt(cur.x, p.y - 0.1f, cur.z);
        bott_in_liquid = bott_in_liquid || (bt == WATER || bt == LAVA);
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
        float eps = 0.21f;
        if (xF && x < glm::abs(min.x)) {
            min.x = x * glm::sign(min.x);
            if (min.x <= eps) min.x = 0;
        }
        if (yF && y < glm::abs(min.y)) {
            min.y = y * glm::sign(min.y);
            if (min.y <= eps) min.y = 0;
        }
        if (zF && z < glm::abs(min.z)) {
            min.z = z * glm::sign(min.z);
            if (min.z <= eps) min.z = 0;
        }
    }
    //qDebug() << min.x << " " << min.y << " " << min.z;
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

glm::vec3 Player::getVelocity()
{
    return m_velocity;
}

float Player::getPhi() {
    return phi;
}

float Player::getTheta() {
    return theta;
}

void Player::setState(glm::vec3 p, glm::vec3 v, float f1, float f2, ItemType i) {
    m_position = p;
    m_velocity = v;
    m_camera.setPos(p + glm::vec3(0, 1.5f, 0));
    theta = f1;
    phi = f2;
    inHand = i;
}

void Player::draw(ShaderProgram* m_prog, Texture& skin, float tick) {
    skin.bind(0);
    float ratio = 1.8f / 32;
    glm::mat4 sc = glm::scale(glm::mat4(1), glm::vec3(ratio));
    glm::mat4 horiz = glm::rotate(glm::mat4(1), -glm::atan(m_forward.z, m_forward.x), glm::vec3(0, 1, 0));
    float len = glm::sqrt(pow(m_forward.x, 2) + pow(m_forward.z, 2));
    glm::mat4 vert = glm::rotate(glm::mat4(1), glm::atan(m_forward.y, len), glm::vec3(0, 0, 1));
    //torso
    glm::mat4 torso_trans = glm::translate(glm::mat4(1), glm::vec3(0, 24, 0));
    m_prog->setModelMatrix(glm::translate(glm::mat4(1), m_position) *
                           horiz *
                           sc *
                           torso_trans);
    m_prog->drawInterleaved(torso);
    //head
    glm::mat4 head_trans = glm::translate(glm::mat4(1), glm::vec3(0, 24, 0));
    m_prog->setModelMatrix(glm::translate(glm::mat4(1), m_position) *
                           sc *
                           head_trans *
                           horiz *
                           vert *
                           glm::translate(glm::mat4(1), glm::vec3(0, 8, 0)));
    //right arm
    m_prog->drawInterleaved(head);
    glm::mat4 outleft = glm::rotate(glm::mat4(1), glm::radians(-20.f), glm::vec3(1, 0, 0));
    glm::mat4 outright = glm::rotate(glm::mat4(1), glm::radians(20.f), glm::vec3(1, 0, 0));
    float period = 15.f;
    float off = 45 * sin((tick - start_swing) / period) * swing_dir;
//    qDebug() << off << " " << swinging << " " << stopped;
    if (!swinging && glm::abs(off) < 1.f) stopped = true;
    if (stopped) off = 0;
    glm::mat4 rot1 = glm::rotate(glm::mat4(1), glm::radians(off), glm::vec3(0, 0, 1));
    glm::mat4 rot2 = glm::rotate(glm::mat4(1), glm::radians(-off), glm::vec3(0, 0, 1));
    glm::mat4 right_arm_trans = glm::translate(glm::mat4(1), glm::vec3(0, 24, -4));
    m_prog->setModelMatrix(glm::translate(glm::mat4(1), m_position) *
                           horiz *
                           sc *
                           right_arm_trans *
                           rot1 *
                           outright);
    m_prog->drawInterleaved(right_arm);
    //left arm
    glm::mat4 left_arm_trans = glm::translate(glm::mat4(1), glm::vec3(0, 24, 4));
    m_prog->setModelMatrix(glm::translate(glm::mat4(1), m_position) *
                           horiz *
                           sc *
                           left_arm_trans *
                           rot2 *
                           outleft);
    m_prog->drawInterleaved(left_arm);
    //right leg
    glm::mat4 right_leg_trans = glm::translate(glm::mat4(1), glm::vec3(0, 12, -2));
    m_prog->setModelMatrix(glm::translate(glm::mat4(1), m_position) *
                           horiz *
                           sc *
                           right_leg_trans *
                           rot2);
    m_prog->drawInterleaved(right_leg);
    //left leg
    glm::mat4 left_leg_trans = glm::translate(glm::mat4(1), glm::vec3(0, 12, 2));
    m_prog->setModelMatrix(glm::translate(glm::mat4(1), m_position) *
                           horiz *
                           sc *
                           left_leg_trans *
                           rot1);
    m_prog->drawInterleaved(left_leg);
}

void Player::drawArm(ShaderProgram* m_prog, Texture& skin) {
    skin.bind(0);
    float ratio = 1.8f / 52;
    glm::mat4 sc = glm::scale(glm::mat4(1), glm::vec3(ratio));
    glm::mat4 cam_to_arm = glm::translate(glm::mat4(1), glm::vec3(23, -12.5, 12));
    glm::mat4 inward = glm::rotate(glm::mat4(1), glm::radians(-7.5f), glm::vec3(0, 1, 0));
    glm::mat4 outward = glm::rotate(glm::mat4(1), glm::radians(110.f), glm::vec3(0, 0, 1));
    glm::mat4 horiz = glm::rotate(glm::mat4(1), -glm::atan(m_forward.z, m_forward.x), glm::vec3(0, 1, 0));
    float len = glm::sqrt(pow(m_forward.x, 2) + pow(m_forward.z, 2));
    float ang = glm::atan(m_forward.y, len);
    glm::mat4 vert = glm::rotate(glm::mat4(1), ang, glm::vec3(0, 0, 1));
    m_prog->setModelMatrix(glm::translate(glm::mat4(1), m_position) *
                           glm::translate(glm::mat4(1), glm::vec3(0, 1.5, 0)) *
                           horiz *
                           vert *
                           sc *
                           cam_to_arm *
                           inward *
                           outward);
    m_prog->drawInterleaved(right_arm);
}

void Player::drawCubeDisplay(ShaderProgram* m_prog) {
    float bound = 4.f;
    float dist;
    glm::ivec3 block_pos; Direction dir;
    glm::vec3 ray = glm::normalize(m_forward) * bound;
    bool found = mcr_terrain.gridMarch(mcr_camera.m_position, ray, &dist, &block_pos, dir);
    if (found) {
        m_prog->setModelMatrix(glm::translate(glm::mat4(1), glm::vec3(block_pos)));
        m_prog->draw(display);
    }
}

void Player::createVBOdata() {
    bool b = hit > 0;
    for (Prism p : parts) p.setHit(b);
    head.createVBOdata();
    torso.createVBOdata();
    right_arm.createVBOdata();
    right_leg.createVBOdata();
    left_arm.createVBOdata();
    left_leg.createVBOdata();
    display.createVBOdata();
    created = true;
}

void Player::knockback(glm::vec3 kb) {
    m_velocity+=kb;
}


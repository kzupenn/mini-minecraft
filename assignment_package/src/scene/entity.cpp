#include "entity.h"
#include <QDebug>

//Entity::Entity()
//    :  Entity(glm::vec3(0,0,0))
//{}

Entity::Entity(glm::vec3 pos)
    : m_forward(0,0,-1), m_right(1,0,0), m_up(0,1,0),
      m_position(pos), mcr_position(m_position), dim(glm::vec3(0))
{}

Entity::Entity(glm::vec3 pos, glm::vec3 dim)
    : m_forward(0,0,-1), m_right(1,0,0), m_up(0,1,0),
      m_position(pos), mcr_position(m_position), dim(dim)
{}

Entity::Entity(const Entity &e)
    : m_forward(e.m_forward), m_right(e.m_right), m_up(e.m_up),
      m_position(e.m_position), mcr_position(m_position), dim(e.dim)
{}

Entity::~Entity()
{}

bool Entity::inBoundingBox(glm::vec3 pt) {
    bool x = glm::abs(m_position.x - pt.x) <= dim.x / 2;
    bool z = glm::abs(m_position.z - pt.z) <= dim.z / 2;
    bool y = (pt.y - m_position.y) <= dim.y;
//    qDebug() << x << " " << y << " " << z;
    return x && y && z;
}

void Entity::moveAlongVector(glm::vec3 dir) {
    m_position += dir;
}

void Entity::moveForwardLocal(float amount) {
    m_position += amount * m_forward;
}

void Entity::moveRightLocal(float amount) {
    m_position += amount * m_right;
}

void Entity::moveUpLocal(float amount) {
    m_position += amount * m_up;
}

void Entity::moveForwardGlobal(float amount) {
    m_position += glm::vec3(0, 0, amount);
}

void Entity::moveRightGlobal(float amount) {
    m_position += glm::vec3(amount, 0, 0);
}

void Entity::moveUpGlobal(float amount) {
    m_position += glm::vec3(0, amount, 0);
}

void Entity::rotateOnForwardLocal(float degrees) {
    float rad = glm::radians(degrees);
    glm::mat4 rot(glm::rotate(glm::mat4(), rad, m_forward));
    m_right = glm::vec3(rot * glm::vec4(m_right, 0.f));
    m_up = glm::vec3(rot * glm::vec4(m_up, 0.f));
}

void Entity::rotateOnRightLocal(float degrees) {
    float rad = glm::radians(degrees);
    glm::mat4 rot(glm::rotate(glm::mat4(), rad, m_right));
    m_forward = glm::vec3(rot * glm::vec4(m_forward, 0.f));
    m_up = glm::vec3(rot * glm::vec4(m_up, 0.f));
}

void Entity::rotateOnUpLocal(float degrees) {
    float rad = glm::radians(degrees);
    glm::mat4 rot(glm::rotate(glm::mat4(), rad, m_up));
    m_right = glm::vec3(rot * glm::vec4(m_right, 0.f));
    m_forward = glm::vec3(rot * glm::vec4(m_forward, 0.f));
}

void Entity::rotateOnForwardGlobal(float degrees) {
    float rad = glm::radians(degrees);
    m_forward = glm::vec3(glm::rotate(glm::mat4(), rad, glm::vec3(0,0,1)) * glm::vec4(m_forward, 0.f));
    m_right = glm::vec3(glm::rotate(glm::mat4(), rad, glm::vec3(0,0,1)) * glm::vec4(m_right, 0.f));
    m_up = glm::vec3(glm::rotate(glm::mat4(), rad, glm::vec3(0,0,1)) * glm::vec4(m_up, 0.f));
}

void Entity::rotateOnRightGlobal(float degrees) {
    float rad = glm::radians(degrees);
    m_forward = glm::vec3(glm::rotate(glm::mat4(), rad, glm::vec3(1,0,0)) * glm::vec4(m_forward, 0.f));
    m_right = glm::vec3(glm::rotate(glm::mat4(), rad, glm::vec3(1,0,0)) * glm::vec4(m_right, 0.f));
    m_up = glm::vec3(glm::rotate(glm::mat4(), rad, glm::vec3(1,0,0)) * glm::vec4(m_up, 0.f));
}

void Entity::rotateOnUpGlobal(float degrees) {
    float rad = glm::radians(degrees);
    m_forward = glm::vec3(glm::rotate(glm::mat4(), rad, glm::vec3(0,1,0)) * glm::vec4(m_forward, 0.f));
    m_right = glm::vec3(glm::rotate(glm::mat4(), rad, glm::vec3(0,1,0)) * glm::vec4(m_right, 0.f));
    m_up = glm::vec3(glm::rotate(glm::mat4(), rad, glm::vec3(0,1,0)) * glm::vec4(m_up, 0.f));
}

glm::vec3 Entity::getPos() {
    return glm::vec3(m_position);
}

#include "core/Node.h"

Node::Node() : id(-1), name("") {}

Node::Node(int id, const std::string& name) : id(id), name(name) {}

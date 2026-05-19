#pragma once
#include <string>

struct Node {
    int id;
    std::string name;

    Node();
    Node(int id, const std::string& name);
};

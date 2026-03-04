#ifndef SYMBOLE_H
#define SYMBOLE_H

#include "type.h"
#include <string>

class Symbole {
public:
    Symbole(std::string name, Type type, int index) : name(name), type(type), index(index) {}
    std::string getName() { return name; }
    Type getType() { return type; }
    int getIndex() { return index; }

private:
    std::string name;
    Type type;
    int index;
};

#endif

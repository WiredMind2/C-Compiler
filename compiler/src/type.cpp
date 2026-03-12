#include "type.h"

#include <iostream>

Type type_from_string(const std::string& str)
{
    if (str == "int")
        return INT;
    if (str == "char")
        return CHAR;
    if (str == "double")
        return DOUBLE;
    if (str == "void")
        return VOID;

    std::cerr << "Unknown type " << str << std::endl;
    exit(1);
}
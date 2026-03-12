#ifndef TYPE_H
#define TYPE_H

#include <string>

typedef enum {
    INT,
    CHAR,
    DOUBLE,
    VOID
} Type;

/**
 * Convert a string representation to a Type enum.
 * @param str The type string (e.g., "int", "char", "double", "void")
 * @return The corresponding Type enum value
 */
Type type_from_string(const std::string& str);

#endif

#pragma once

#include <iostream>
#include <string>

#ifdef _WIN32
    #ifdef _EXPORT_DLL_
        #define EXPORT __declspec(dllexport)
    #else
        #define EXPORT __declspec(dllimport)
    #endif
#else
    #define EXPORT
#endif

EXPORT extern const std::string var;

EXPORT void hello();

// EXPORT class hi {
class EXPORT hi {
public:
    hi() {
        std::cout << "hello construct." << std::endl;
    }
};
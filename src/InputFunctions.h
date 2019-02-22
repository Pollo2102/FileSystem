#ifndef INPUT_FUNCTIONS_H
#define INPUT_FUNCTIONS_H

#include <string.h>
#include <iostream>

class InputFunctions
{
public:
    InputFunctions();

    void inputCommand();
    void parseCommand(std::string command);
};




#endif // !INPUT_FUNCTIONS_H

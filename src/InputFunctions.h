#ifndef INPUT_FUNCTIONS_H
#define INPUT_FUNCTIONS_H

#include <string.h>
#include <iostream>
#include <vector>

class InputFunctions
{
public:
    InputFunctions();

    std::vector<std::string> inputCommand();
    void parseCommand(std::string command);
};




#endif // !INPUT_FUNCTIONS_H

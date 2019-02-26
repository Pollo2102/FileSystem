#include <cstdio>
#include "InputFunctions.h"

InputFunctions::InputFunctions()
{

}

std::vector<std::string> InputFunctions::inputCommand()
{
    char *commandString = new char;
    
    std::vector<std::string> args;
    
    std::cout << "Type in your command: \n";
    
    std::cin.getline(commandString, 200);

    char *argumentsTemp = new char;
    argumentsTemp = strtok(commandString, " ");
    printf("%s\n", argumentsTemp);

    std::string tempString;

    while (argumentsTemp != nullptr)
    {
        tempString = argumentsTemp;
        std::cout << tempString + " ";
        argumentsTemp = strtok(nullptr, " ");

        args.push_back(tempString);
        tempString.clear();
    }
    
    delete argumentsTemp;
    delete commandString;

    return args;
}


void InputFunctions::parseCommand(std::string command)
{
    
}
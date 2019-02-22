#include <cstdio>
#include "InputFunctions.h"
#include <vector>

InputFunctions::InputFunctions()
{

}

void InputFunctions::inputCommand()
{
    char *commandString = new char;
    
    std::vector<std::string> args;

    while (true/* !strcmp(commandString, "exit") */)
    {
        std::cout << "Ingrese su comando: \n";
        
        std::cin.getline(commandString, 100);

        char *argumentsTemp = new char;
        argumentsTemp = strtok(commandString, " ");
        printf("%s\n", argumentsTemp);

        std::string tempString;

        while (argumentsTemp != NULL)
        {
            tempString = argumentsTemp;
            std::cout << tempString + " ";
            argumentsTemp = strtok(NULL, " ");

            args.push_back(tempString);
            tempString.clear();
        }

        
        commandString[0] = 0;
        
        delete argumentsTemp;
    }

    delete commandString;
}


void InputFunctions::parseCommand(std::string command)
{
    
}
#include "FileSystem.h"
#include <iostream>


int main(int argc, char const *argv[])
{
    FileSystem customFS;

    customFS.createDatabase();
    customFS.loadDatabase();
    customFS.createTable();

    return 0;
}
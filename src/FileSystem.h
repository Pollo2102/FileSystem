#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "DataBlock_Sructure.h"
#include <string>

#define DATABASE_SIZE 500000000 // Fixes-size = 500MB

class FileSystem
{
public:
    FileSystem();
    ~FileSystem();

    void createDatabase();
    void dropDatabase();
    void loadDatabase();
    
    void createTable();
    void dropTable();

    void insertData();
    void deleteData();
    void updateData();
    void selectData();

    // Auxiliary Functions

    long getEmptyIndexPosition();
    std::string setTableName();
    void setTableColumnNames(u_int32_t tableColumns[], char tableNames[][400]);
    uint64_t getEmptyDataBlockPosition();

};


/*
This FileSystem is created with the scope of 
simulating a barebones database and most of
the basic functions expected from a database.
These functions include:
Creating and dropping tables
Inserting, deleting, updating and showing data from tables.

Database specifications are as follows:

Block size is defined at the time of the database creation 
by the user. The allowed range is between 512 and 8192 bytes.

Free Data Blocks are managed through a bitmap

Database entries are fixed-size.

Database is using an extended organization. (Pointer to next block)

The Database is managed through a console interface.
*/

#endif // !FILESYSTEM_H

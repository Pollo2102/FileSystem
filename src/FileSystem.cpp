#include "FileSystem.h"
#include <cstdio>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include <bitset>
#include <boost/multiprecision/cpp_int.hpp>

#define DB_DIR "../DataBases/"

FileSystem::FileSystem()
{
}

//===============================================================
//==========================DATABASE=============================
//===============================================================

//Creates a DataBase using parameters specified by the user.
void FileSystem::createDatabase()
{

    //////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////VARIABLE DECLARATION//////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////

    std::string DBName;
    uint64_t DBSize = 0;
    uint16_t BlockSize = 0;
    superBlock SB;                  // Declaration of the super block
    blockGroupDescriptorTable BGDT; // Declaration of the block that contains metadata about the Database Index, tables, etc.
    tableIndex TI;

    //////////////////////////////////////////////////////////////////////////////////////
    //////////////////////USER INPUT FOR DATABASE INFORMATION/////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////

    std::cout << "Type in the name of the DataBase you wish to create: \n";
    std::cin >> DBName;

    DBName += ".db";
    DBName.insert(0, DB_DIR);

    std::cout << "Type in the size in MB of the Database you wish to create.\n";
    std::cin >> DBSize;

    DBSize *= 1000000;

    std::cout << "Type in the size in Bytes of the Data Blocks.\n";
    std::cin >> BlockSize;

    if (BlockSize < 512)
        BlockSize = 512;
    else if (BlockSize > 8192)
        BlockSize = 8192;

    //////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////DATABASE CALCULATIONS//////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////

    uint32_t sizeOfMetadataBlocks = ((sizeof(superBlock) + sizeof(blockGroupDescriptorTable) + sizeof(tableIndex) * 100));

    uint32_t numberOfDataBlocks = (DBSize - sizeOfMetadataBlocks) / BlockSize;

    
    std::vector<char> dataBlockBitmap(ceil((float)numberOfDataBlocks / (float)8));

    //Data in the char vector is initialized to 0 automatically
    
    std::vector<char> dataBlocks(BlockSize);

    for(size_t i = 0; i < dataBlockBitmap.size(); i++)
    {
        for(uint8_t j = 0; j < 8; j++)
        {
            dataBlockBitmap.at(i) |= (0 << j);
        }
    }
    std::cout << "Error in generating the dataBlock Bitmap\n";

    SB.indexCount = 100;
    SB.blocks_count = numberOfDataBlocks;
    SB.s_state = 0;
    SB.dataBlockSize = sizeOfMetadataBlocks;
    strcpy(SB.volume_name, DBName.c_str());

    BGDT.first_index = (sizeof(superBlock) + sizeof(blockGroupDescriptorTable) + dataBlockBitmap.size()) - 1;
    BGDT.firstDataBlock = (sizeof(superBlock) + sizeof(blockGroupDescriptorTable) + dataBlockBitmap.size() + (sizeof(tableIndex) * 100)) - 1;
    BGDT.block_bitmap = (sizeof(superBlock) + sizeof(blockGroupDescriptorTable));
    BGDT.freeBlocks_count = numberOfDataBlocks;
    BGDT.freeInodes_count = 100;
    BGDT.tableCount = 0;

    TI.usedTableSpace = false;
    TI.size = 0;
    TI.tablePosition = 0;

    //////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////WRITING DATA INTO THE DATABASE FILE/////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////

    /*
        Writing order:
        -superBlock
        -blockGroupDescriptorTable
        -dataBlock Bitmap
        -indexBlock
        -All Datablocks
    */

    std::ofstream file(DBName, std::ios::out | std::ios::binary);

    if (!file)
    {
        std::cout << "There was a mistake while opening the file.\n";
        return;
    }

    file.write(reinterpret_cast<const char *>(&SB), sizeof(superBlock));
    file.write(reinterpret_cast<const char *>(&BGDT), sizeof(blockGroupDescriptorTable));

    for(size_t i = 0; i < dataBlockBitmap.size(); i++)
    {
        file.write(reinterpret_cast<const char *>(&dataBlockBitmap.at(i)), sizeof(char));
    }
    

    for (size_t i = 0; i < 100; i++)
    {
        file.write(reinterpret_cast<const char *>(&TI), sizeof(tableIndex));
    }

    std::cout << "Error in generating the dataBlocks\n";
    for (size_t i = 0; i < numberOfDataBlocks; i++)
    {
        for(int j = 0; j < dataBlocks.size(); j++)
        {
            file.write(reinterpret_cast<const char *>(&dataBlocks.at(j)), sizeof(char));
            
        }
    }

    file.close();
}

// Deletes a Database File from the system
void FileSystem::dropDatabase()
{
    std::string file = DB_DIR;
    std::string DBName;
    printf("Type in the name of the database you wish to delete: \n");
    std::cin >> DBName;

    file += (DBName + ".dat");

    const char *filename = file.c_str();

    remove(filename);

    std::cout << "DataBase \"" + DBName + "\" deleted succesfully!\n";

    delete filename;
}

//===============================================================
//==========================TABLES===============================
//===============================================================

void FileSystem::createTable()
{
}

void FileSystem::dropTable(char *tableName)
{
}

//===============================================================
//==========================DATA===============================
//===============================================================

void FileSystem::insertData()
{
}

void FileSystem::deleteData()
{
}

void FileSystem::updateData()
{
}

void FileSystem::selectData()
{
}
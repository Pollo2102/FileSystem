#include "FileSystem.h"

#include <fstream>
#include <iostream>
#include <cstdio>

#include <cmath>

#include <cstring>
#include <sstream>

#include <vector>

#include <boost/multiprecision/cpp_int.hpp>

#define DB_DIR "../DataBases/"

static superBlock *loadedSuperBlock = nullptr;
static blockGroupDescriptorTable *loadedBGDT = nullptr;
static std::fstream *loadedDB = new std::fstream;
static std::string loadedDBName;

FileSystem::FileSystem()
{
}

FileSystem::~FileSystem()
{
    delete loadedSuperBlock;
    delete loadedBGDT;
    delete loadedDB;
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
    double DBSize = 0;
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

    for (size_t i = 0; i < dataBlockBitmap.size(); i++)
    {
        for (uint8_t j = 0; j < 8; j++)
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

    for (size_t i = 0; i < dataBlockBitmap.size(); i++)
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
        for (int j = 0; j < dataBlocks.size(); j++)
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

void FileSystem::loadDatabase()
{
    std::string DBName;
    std::cout << "Type in the name of the Database you wish to load.\n";
    std::cin >> DBName;

    std::cout << std::endl;

    DBName.insert(0, DB_DIR);
    DBName += ".db";

    loadedDB->open(DBName, std::ios::in);

    if (loadedDB->fail())
    {
        std::cout << "Database loading error!\n";
        return;
    }

    loadedDB->read(reinterpret_cast<char *>(&loadedSuperBlock), sizeof(superBlock));
    loadedDB->read(reinterpret_cast<char *>(&loadedBGDT), sizeof(blockGroupDescriptorTable));
    loadedDB->close();
    loadedDBName = DBName;
}

//===============================================================
//==========================TABLES===============================
//===============================================================

void FileSystem::createTable()
{
    if (loadedSuperBlock != nullptr)
    {
        try 
        {
            long indexPosition = getEmptyIndexPosition();
            tableIndex tmpTI;

            std::string tableName = setTableName();

            setTableColumnNames(tmpTI.tableColumns, tmpTI.tableNames);
            tmpTI.tablePosition = getEmptyDataBlockPosition();
            tmpTI.size = loadedSuperBlock->dataBlockSize;

            loadedDB->open(loadedDBName, std::ios::out | std::ios::binary);
            loadedDB->seekp(indexPosition);
            loadedDB->write(reinterpret_cast<const char *>(&tmpTI), sizeof(tableIndex));
            loadedDB->close();
            std::cout << "Data Table created successfully\n";

        }
        catch(std::exception e)
        {
            std::cout << "Error during the process of creating the table.\n";
            std::cout << "Exception Specifications: " << e.what() << std::endl;
        }
    }
}

void FileSystem::dropTable()
{
    char tempTableName[64];
    std::cout << "Type the name of the table you wish to delete.\n";
    std::cin >> tempTableName;
    tableIndex tempIndex;

    loadedDB->open(loadedDBName, std::ios::in | std::ios::binary);
    loadedDB->seekg(loadedBGDT->first_index);
    loadedDB->read(reinterpret_cast<char *>(&tempIndex), sizeof(tableIndex));

    uint8_t tableCount = 1;

    while(tableCount != 100)
    {
        if (tempIndex.tableName == tempTableName)
        {
            long tempIndexPosition = (uint64_t)loadedDB->tellg() - (uint64_t)sizeof(tableIndex);
            tempIndex.usedTableSpace = 0;
            loadedDB->close();
            loadedDB->open(loadedDBName, std::ios::out | std::ios::binary);
            loadedDB->seekp(tempIndexPosition);
            loadedDB->write(reinterpret_cast<const char *>(&tempIndex), sizeof(tableIndex));
            loadedDB->close();
            std::cout << "Data deleted successfully\n";
            return;
        }
        loadedDB->read(reinterpret_cast<char *>(&tempIndex), sizeof(tableIndex));
    }
    loadedDB->close();
    std::cout << "Table not found\n";
}

//===============================================================
//==========================DATA=================================
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

//===============================================================
//======================AUXILIARY FUNCTIONS======================
//===============================================================

long FileSystem::getEmptyIndexPosition()
{
    if (loadedBGDT != nullptr)
    {
        loadedDB->seekg(loadedBGDT->first_index);
        long indexPos = loadedDB->tellg();
        tableIndex tempTI;
        while (!loadedDB->eof())
        {
            loadedDB->read(reinterpret_cast<char *>(&tempTI), sizeof(tableIndex));
            if (tempTI.usedTableSpace == true)
            {
                loadedDB->close();
                return indexPos;
            }
            indexPos = loadedDB->tellg();
        }
        loadedDB->close();
    }
}

std::string FileSystem::setTableName()
{
    std::string tempName;
    std::cout << "Type the name you wish to use for the table\n";
    std::cin >> tempName;

    return tempName;
}

void FileSystem::setTableColumnNames(uint32_t tableColumns[], char tableNames[][400])
{
    /* 
        Data format 32 bits
        -Bit 0 indicates if the column is being used
        -Bit 1-2 indicates the data type.
        -Bits 3-31 indicate the size of the char, if the data 
        type happens to be a char.
     */
    bool quit = false;
    uint8_t columnCounter = 0;

    while ((!quit) && (columnCounter < 100))
    {
        uint32_t dataType = 0;
        std::cout << "Type in the type of data you wish to use.\n 0 - STOP\n 1 - INT\n 2 - DOUBLE\n 3 - CHAR\n";
        std::cin >> dataType;
        if (dataType == 0)
            break;
        dataType <<= 1;
        dataType |= 1;

        if (dataType == 0b00000111)
        {
            uint16_t charSize = 0;

            std::cout << "Type in the CHAR size. (Upper limit of 4000)\n";
            std::cin >> charSize;
            if (charSize > 4000)
                charSize = 4000;

            dataType |= (charSize << 8);
        }

        std::string columnName;

        std::cout << "Type the name of the column: \n";
        std::cin >> columnName;

        tableColumns[columnCounter] = dataType;
        strcpy(tableNames[columnCounter], columnName.c_str());
    }
    std::cout << "Columns registered succesfully.\n";
}

uint64_t FileSystem::getEmptyDataBlockPosition()
{
    uint64_t dataBlockPosition;

    loadedDB->open(loadedDBName, std::ios::in | std::ios::binary);
    loadedDB->seekg(loadedBGDT->block_bitmap);

    std::vector<char> dataBlocks(ceil((float)loadedSuperBlock->blocks_count / (float)8));

    for (size_t i = 0; i < dataBlocks.size(); i++)
    {
        loadedDB->read(reinterpret_cast<char *>(dataBlocks.at(i)), sizeof(char));
    }

    loadedDB->close();
    loadedDB->open(loadedDBName, std::ios::out | std::ios::binary);

    for (size_t i = 0; i < dataBlocks.size(); i++)
    {
        for (size_t j = 0; j < 8; j++)
        {
            if ((dataBlocks.at(i) & (1 << j)) == 0)
            {
                dataBlocks.at(i) |= (1 << j);
                loadedDB->seekg(loadedBGDT->block_bitmap);
                for (size_t i = 0; i < dataBlocks.size(); i++)
                {
                    loadedDB->write(reinterpret_cast<const char *>(&dataBlocks.at(i)), sizeof(char));
                }
                loadedDB->close();
                dataBlockPosition = j + (i * 8);
                dataBlockPosition = (loadedBGDT->firstDataBlock) + (dataBlockPosition * loadedSuperBlock->dataBlockSize);
                return dataBlockPosition;
            }
        }
    }
}
#include "FileSystem.h"

#include <fstream>
#include <iostream>
#include <cstdio>

#include <cmath>

#include <cstring>
#include <sstream>

#include <vector>

//#include <boost/multiprecision/cpp_int.hpp>

#define DB_DIR "../DataBases/"

static superBlock loadedSuperBlock;
static blockGroupDescriptorTable loadedBGDT;
static std::fstream loadedDB;
static std::string loadedDBName;

FileSystem::FileSystem()
{
}

FileSystem::~FileSystem()
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

    file += (DBName + ".db");

    const char *filename = file.c_str();

    if (!remove(filename))
        std::cout << "Database \"" + DBName + "\" deleted succesfully!\n";
    else
        std::cout << "Database \"" + DBName + "\" was not found!\n";

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

    loadedDB.open(DBName, std::ios::in | std::ios::binary);

    if (loadedDB.fail())
    {
        std::cout << "Database loading error!\n";
        return;
    }

    loadedDB.seekg(0);
    loadedDB.read(reinterpret_cast<char *>(&loadedSuperBlock), sizeof(superBlock));
    loadedDB.read(reinterpret_cast<char *>(&loadedBGDT), sizeof(blockGroupDescriptorTable));
    loadedDB.close();
    loadedDBName = DBName;
    std::cout << "Database loaded succesfully\n";
}

//===============================================================
//==========================TABLES===============================
//===============================================================

void FileSystem::createTable()
{
    if (!loadedDBName.empty())
    {
        try
        {
            long indexPosition = getEmptyIndexPosition();
            if (indexPosition == 0)
                return;

            tableIndex tmpTI;

            std::string tableName = setTableName();

            strcpy(tmpTI.tableName, tableName.c_str());
            setTableColumnNames(tmpTI.tableColumns, tmpTI.tableNames);
            tmpTI.tablePosition = getEmptyDataBlockPosition();//check
            tmpTI.size = loadedSuperBlock.dataBlockSize;

            loadedDB.open(loadedDBName, std::ios::in | std::ios::out | std::ios::binary | std::ios::ate);
            loadedDB.seekp(indexPosition);
            loadedDB.write(reinterpret_cast<const char *>(&tmpTI), sizeof(tableIndex));
            loadedDB.close();
            std::cout << "Data Table created successfully\n\n\n";
        }
        catch (std::exception e)
        {
            std::cout << "Error during the process of creating the table.\n";
            std::cout << "Exception Specifications: " << e.what() << std::endl;
        }
    }
    else
    {
        std::cout << "Database not loaded\n\n";
    }
    
}

void FileSystem::dropTable()
{
    char tempTableName[64] = {0};    
    std::cout << "Type the name of the table you wish to delete.\n";
    std::cin >> tempTableName;
    tableIndex tempIndex;

    loadedDB.open(loadedDBName, std::ios::in | std::ios::binary);
    loadedDB.seekg(loadedBGDT.first_index);
    loadedDB.read(reinterpret_cast<char *>(&tempIndex), sizeof(tableIndex));

    uint8_t tableCount = 1;

    while (tableCount != 100)
    {
        if ((!strcmp(tempIndex.tableName, tempTableName)) && (tempIndex.usedTableSpace == true))
        {
            long tempIndexPosition = (uint64_t)loadedDB.tellg() - (uint64_t)sizeof(tableIndex);
            tempIndex.usedTableSpace = 0;
            deleteDatablockPointers(tempIndex.tablePosition);
            loadedDB.close();
            loadedDB.open(loadedDBName, std::ios::in | std::ios::out | std::ios::binary | std::ios::ate);
            loadedDB.seekp(tempIndexPosition);
            loadedDB.write(reinterpret_cast<const char *>(&tempIndex), sizeof(tableIndex));
            loadedDB.close();
            std::cout << "Data deleted successfully\n";
            return;
        }
        loadedDB.read(reinterpret_cast<char *>(&tempIndex), sizeof(tableIndex));
        tableCount++;
    }
    loadedDB.close();
    std::cout << "Table not found\n";
}

//===============================================================
//==========================DATA=================================
//===============================================================

void FileSystem::insertData()
{
    std::string tableName;
    std::cout << "Type the name of the table you wish to insert into.\n";
    std::cin >> tableName;

    uint64_t tablePosition = findTable(tableName);
    tableIndex TI;

    if (tablePosition == 0)
    {
        std::cout << "Data table could not be found\n\n";
        return;
    }

    loadedDB.open(loadedDBName, std::ios::in | std::ios::binary);

    if (!loadedDB)
    {
        std::cout << "Database file could not be found!\n\n";
        return;
    }

    loadedDB.read(reinterpret_cast<char *>(&TI), sizeof(tableIndex));
    loadedDB.close();

    std::vector<char> data;

    int valor1;
    double valor2;

    for (uint8_t i = 0; i < 100; i++)
    {
        if (TI.tableColumns[i] == 0)
            break;

        if ((TI.tableColumns[i] & 0b00000111) == 0b00000001) // int
        {
            std::cout << "Type in the value of: " << TI.tableNames[i] << std::endl;
            std::cin >> valor1;

            for (size_t j = 0; j < sizeof(int); j++)
            {
                data.push_back(valor1 << j);
            }
        }
        else if ((TI.tableColumns[i] & 0b00000111) == 0b00000011) // double
        {
            std::cout << "Type in the value of: " << TI.tableNames[i] << std::endl;
            std::cin >> valor1;

            for (size_t j = 0; j < sizeof(double); j++)
            {
                data.push_back(valor1 << j);
            }
        }
        else if ((TI.tableColumns[i] & 0b00000111) == 0b00000101) // char
        {
            std::string valor3;
            std::cout << "Type in the value of: " << TI.tableNames[i] << std::endl;
            std::cin >> valor3;

            for (uint32_t i = 0; i < (TI.tableColumns[i] >> 3); i++)
            {
                if (i < data.size())
                    data.push_back(valor3.at(i));
                data.push_back(0);
            }
        }

        writeDataIntoTable(data, tablePosition, data.size(), 0, false);
    }
}

void FileSystem::deleteData()
{
    std::cout << "Not yet implemented.\n\n\n";
}

void FileSystem::updateData()
{
    std::cout << "Not yet implemented.\n\n\n";   
}

void FileSystem::selectData()
{
}

void FileSystem::mainMenu()
{
    int option;

    while (true)
    {
        std::cout << "---------------------------Database File System----------------------------------\n\n";
        std::cout << "0 - Create Database\n";
        std::cout << "1 - Drop Database\n";
        std::cout << "2 - Load Database\n";
        std::cout << "3 - Create Table\n";
        std::cout << "4 - Drop Table\n";
        std::cout << "5 - Insert Data\n";
        std::cout << "6 - Delete Data\n";
        std::cout << "7 - Update Data\n";
        std::cout << "8 - Select Data\n";
        std::cout << "9 - Exit\n\n";
        std::cout << "Type the number of the command you wish to execute: \n";
        
        std::cin >> option;

        switch (option)
        {
            case 0:
                createDatabase();
                break;
            
            case 1:
                dropDatabase();
                break;

            case 2:
                loadDatabase();
                break;

            case 3:
                createTable();
                break;

            case 4:
                dropTable();
                break;

            case 5:
                insertData();
                break;

            case 6:
                deleteData();
                break;

            case 7:
                updateData();
                break;     

            case 8:
                selectData();
                break;

            case 9:
                return;
        
            default:
                std::cout << "Invalid command\n\n";
                break;
        }
    }

}

//===============================================================
//======================AUXILIARY FUNCTIONS======================
//===============================================================

long FileSystem::getEmptyIndexPosition()
{
    if (!loadedDBName.empty())
    {
        loadedDB.open(loadedDBName, std::ios::in | std::ios::binary);
        loadedDB.seekg(loadedBGDT.first_index);
        long indexPos = loadedDB.tellg();
        tableIndex tempTI;
        while (!loadedDB.eof())
        {
            loadedDB.read(reinterpret_cast<char *>(&tempTI), sizeof(tableIndex));
            if (tempTI.usedTableSpace == false)
            {
                loadedDB.close();
                return indexPos;
            }
            indexPos = loadedDB.tellg();
        }
        loadedDB.close();
    }
    std::cout << "No Database is loaded!!\n\n";
    return 0;
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

        std::cout << "\nType the name of the column: \n";
        std::cin >> columnName;
        std::cout << std::endl;

        tableColumns[columnCounter] = dataType;
        strcpy(tableNames[columnCounter], columnName.c_str());
        columnCounter++;
    }
    std::cout << "Columns registered succesfully.\n";
}

uint64_t FileSystem::getEmptyDataBlockPosition()
{
    uint64_t dataBlockPosition;

    loadedDB.open(loadedDBName, std::ios::in | std::ios::binary);
    loadedDB.seekg(loadedBGDT.block_bitmap);

    std::vector<char> dataBlocks(ceil((float)loadedSuperBlock.blocks_count / (float)8));

    for (size_t i = 0; i < dataBlocks.size(); i++)
    {
        loadedDB.read(reinterpret_cast<char *>(&dataBlocks.at(i)), sizeof(char));
    }

    loadedDB.close();
    loadedDB.open(loadedDBName, std::ios::in | std::ios::out | std::ios::binary | std::ios::ate);

    for (size_t i = 0; i < dataBlocks.size(); i++)
    {
        for (size_t j = 0; j < 8; j++)
        {
            if ((dataBlocks.at(i) & (1 << j)) == 0)
            {
                dataBlocks.at(i) |= (1 << j);
                loadedDB.seekp(loadedBGDT.block_bitmap);
                for (size_t i = 0; i < dataBlocks.size(); i++)
                {
                    loadedDB.write(reinterpret_cast<const char *>(&dataBlocks.at(i)), sizeof(char));
                }
                loadedDB.close();
                dataBlockPosition = j + (i * 8);
                dataBlockPosition = (loadedBGDT.firstDataBlock) + (dataBlockPosition * loadedSuperBlock.dataBlockSize);
                return dataBlockPosition;
            }
        }
    }
    std::cout << "No empty datablocks found.\n\n";
    return 0;
}

void FileSystem::deleteDatablockPointers(uint32_t datablockPointer)
{
    std::vector<char> dataBlock(loadedSuperBlock.dataBlockSize);
    loadedDB.open(loadedDBName, std::ios::in | std::ios::out | std::ios::binary | std::ios::ate);
    loadedDB.seekg(loadedBGDT.firstDataBlock + (datablockPointer * loadedSuperBlock.dataBlockSize));

    for (size_t i = 0; i < loadedSuperBlock.dataBlockSize; i++)
    {
        loadedDB.read(reinterpret_cast<char *>(&dataBlock.at(i)), sizeof(char));
    }

    uint32_t tempPointer = 0;

    tempPointer |= (dataBlock.at(loadedSuperBlock.blocks_count - 4));
    tempPointer |= (dataBlock.at(loadedSuperBlock.blocks_count - 3) << 1);
    tempPointer |= (dataBlock.at(loadedSuperBlock.blocks_count - 2) << 2);
    tempPointer |= (dataBlock.at(loadedSuperBlock.blocks_count - 1) << 3);

    if (tempPointer == 0)
    {
        std::cout << "Delete operation finished.\n";
        loadedDB.close();
        return;
    }
    else
    {
        //escribir 0 en el datablock pointer
        dataBlock.at(loadedSuperBlock.blocks_count - 4) = 0;
        dataBlock.at(loadedSuperBlock.blocks_count - 3) = 0;
        dataBlock.at(loadedSuperBlock.blocks_count - 2) = 0;
        dataBlock.at(loadedSuperBlock.blocks_count - 1) = 0;
        loadedDB.seekg(loadedBGDT.firstDataBlock + (datablockPointer * loadedSuperBlock.dataBlockSize));

        for (size_t i = 0; i < loadedSuperBlock.dataBlockSize; i++)
        {
            loadedDB.write(reinterpret_cast<const char *>(&dataBlock.at(i)), sizeof(char));
        }

        loadedDB.close();
        deleteDatablockPointers(tempPointer);
    }
}

uint64_t FileSystem::findTable(std::string tableName)
{
    loadedDB.open(loadedDBName, std::ios::in | std::ios::binary);

    if (!loadedDB)
    {
        std::cout << "Error loading the database file.\n";
        return 0;
    }

    tableIndex TI;
    uint64_t tablePosition;
    char tableN[64] = {0};
    strcpy(tableN, tableName.c_str());

    loadedDB.seekg(loadedBGDT.first_index);
    tablePosition = loadedDB.tellg();
    loadedDB.read(reinterpret_cast<char *>(&TI), sizeof(tableIndex));

    for (size_t i = 0; i < 100; i++)
    {
        if (!strcmp(TI.tableName, tableN))
        {
            loadedDB.close();
            return tablePosition;
        }
        else
        {
            loadedDB.read(reinterpret_cast<char *>(&TI), sizeof(tableIndex));
            tablePosition = loadedDB.tellg();
        }
    }
    std::cout << "Table was not found!\n\n";
    loadedDB.close();
    return 0;
}

void FileSystem::writeDataIntoTable(std::vector<char> &tableData, uint64_t tablePosition, uint16_t dataSize, uint16_t readSpaceLeft, bool writePending)
{
    loadedDB.open(loadedDBName, std::ios::out | std::ios::in | std::ios::binary | std::ios::ate);

    if (!loadedDB)
    {
        std::cout << "Error while trying to access the database file.\n\n";
        return;
    }
    
    uint64_t tablePos = tablePosition;
    loadedDB.seekp(tablePosition);
    std::vector<char> temp(tableData.size());    
    uint32_t spaceLeft = loadedSuperBlock.dataBlockSize - 4;
    uint8_t usedReg = 0;

    if (readSpaceLeft != 0)
    {
        loadedDB.seekp(readSpaceLeft, std::ios::cur);
        spaceLeft -= readSpaceLeft;
    }

    while (true)
    {
        if (tableData.size() < spaceLeft)
        {
            if (!writePending)
                loadedDB.read(reinterpret_cast<char *>(&usedReg), sizeof(char));
            if (!usedReg)
            {
                if (!writePending)
                {
                    usedReg = 1;
                    loadedDB.seekp(-1, std::ios::cur);
                    loadedDB.write(reinterpret_cast<const char *>(&usedReg), sizeof(char));
                }
                for(size_t i = 0; i < tableData.size(); i++)
                {
                    loadedDB.write(reinterpret_cast<const char *>(&tableData.at(i)), sizeof(char));
                }
                std::cout << "Data written into the table successfully;\n\n";
                loadedDB.close();
                return;
            }
            else
            {
                loadedDB.seekp(dataSize, std::ios::cur);
                spaceLeft -= (dataSize + 1);
            }
        }
        else
        {
            if (!writePending)
            {
                loadedDB.read(reinterpret_cast<char *>(&usedReg), sizeof(char));
                spaceLeft -= 1;
            }
            if (!usedReg)
            {
                if (spaceLeft != 0)
                {
                    for(size_t i = 0; i < spaceLeft; i++)
                    {
                        loadedDB.write(reinterpret_cast<const char *>(&tableData.at(i)), sizeof(char));
                        spaceLeft--;
                    }
                    for(size_t i = 0; i < spaceLeft; i++)
                    {
                        tableData.erase(tableData.begin());
                    }
                }
                uint64_t newPosition = getEmptyDataBlockPosition();
                uint32_t writePosition = (newPosition - loadedBGDT.firstDataBlock) / loadedSuperBlock.dataBlockSize;
                loadedDB.write(reinterpret_cast<const char *>(&writePosition), sizeof(uint32_t));
                loadedDB.close();
                writeDataIntoTable(tableData, newPosition, tableData.size(), 0, true);
            }
            else
            {
                uint16_t rSpaceLeft = tableData.size() - spaceLeft;
                loadedDB.seekp(spaceLeft, std::ios::cur);
                uint32_t readAddress;
                loadedDB.read(reinterpret_cast<char *>(&readAddress), sizeof(uint32_t));
                uint64_t searchAddress = loadedBGDT.firstDataBlock + (readAddress * loadedSuperBlock.dataBlockSize);
                loadedDB.close();
                writeDataIntoTable(tableData, searchAddress, tableData.size(), rSpaceLeft, 0);
            }
        }        
    }
}
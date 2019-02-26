#ifndef DATABLOCKS_H
#define DATABLOCKS_H

#include <stdint.h>

// DataBlock that contains metadata about the DataBase
struct superBlock 
{
	uint32_t indexCount; // Number of index registers
	uint32_t blocks_count; // Number of Datablocks in the DataBase
	bool s_state; //Database state (unmounted(0) or error(1))
	uint32_t dataBlockSize; // Size of each Data Block (512 - 8192 bytes)
	char volume_name[64]; //name of the DataBase
};

struct blockGroupDescriptorTable // Information about the DataBase
{
	uint64_t first_index; // Byte of the first Index
	uint64_t firstDataBlock; // Byte of the first Data Block
	uint64_t block_bitmap; // Byte that indicates the position of the block bitmap
	uint64_t freeBlocks_count; // Number of available Data Blocks
	uint64_t freeInodes_count; // Number of available Indexes
	uint64_t tableCount; // Number of tables created at the time of reading
};


// struct blockBitmap
// {
// 	bool state[333080]; // Bitmap that indicates the available and used Data Blocks
// };

// struct indexBitmap
// {
// 	bool state[10]; // Bitmap that indicates the number of indexes being used
// };


struct tableIndex
{
	bool usedTableSpace; // Indicates if the index is being used by a created table
	char tableName[64]; // Name of the table
	uint64_t size; // Table size
	uint64_t tablePosition; // Position of Table in bytes
	uint16_t tableColumns[100]; // Byte 0 is the type of data. Bytes 1-2 represent the length of the data, if it is of type char[]
	char tableNames[100][100]; //100 names for columns of size 100
};



#endif // !DATABLOCKS_H



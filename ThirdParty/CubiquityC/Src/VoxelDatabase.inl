/*******************************************************************************
* The MIT License (MIT)
*
* Copyright (c) 2016 David Williams and Matthew Williams
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*******************************************************************************/

#include "PolyVox/Impl/Timer.h"
#include "PolyVox/Impl/Utility.h"

#include "SQLiteUtils.h"

#include <climits>

namespace Cubiquity
{
	// From http://stackoverflow.com/a/776550
	// Should only be used on unsigned types.
	template <typename T> 
	T rotateLeft(T val)
	{
		return (val << 1) | (val >> (sizeof(T)*CHAR_BIT-1));
	}

	/// Constructor
	template <typename VoxelType>
	VoxelDatabase<VoxelType>::VoxelDatabase()
		:PolyVox::PagedVolume<VoxelType>::Pager()
	{
	}

	/// Destructor
	template <typename VoxelType>
	VoxelDatabase<VoxelType>::~VoxelDatabase()
	{
		EXECUTE_SQLITE_FUNC( sqlite3_finalize(mSelectChunkStatement) );
		EXECUTE_SQLITE_FUNC( sqlite3_finalize(mSelectOverrideChunkStatement) );
		EXECUTE_SQLITE_FUNC( sqlite3_finalize(mInsertOrReplaceBlockStatement) );
		EXECUTE_SQLITE_FUNC( sqlite3_finalize(mInsertOrReplaceOverrideChunkStatement) );
		EXECUTE_SQLITE_FUNC( sqlite3_finalize(mSelectPropertyStatement) );
		EXECUTE_SQLITE_FUNC( sqlite3_finalize(mInsertOrReplacePropertyStatement) );

		if (sqlite3_db_readonly(mDatabase, "main") == 0)
		{
			POLYVOX_LOG_TRACE("Vacuuming database...");
			try
			{
				PolyVox::Timer timer;
				EXECUTE_SQLITE_FUNC(sqlite3_exec(mDatabase, "VACUUM;", 0, 0, 0));
				POLYVOX_LOG_TRACE("Vacuumed database in ", timer.elapsedTimeInMilliSeconds(), "ms");
			}
			catch (DatabaseError& e)
			{
				// It seems that vacuuming of the database can fail even when opened in readwrite mode, if other processes are still
				// accessing the database. This can happen if multiple volumes are sharing the database. This shouldn't really matter
				// as the database will probably get vacuumed at some point in the future, and it's not essential anyway.
				POLYVOX_LOG_WARNING("Failed to vacuum database. Error message was as follows:\n\t", e.what());
			}
		}

		EXECUTE_SQLITE_FUNC(sqlite3_close(mDatabase));
	}

	template <typename VoxelType>
	VoxelDatabase<VoxelType>* VoxelDatabase<VoxelType>::createEmpty(const std::string& pathToNewVoxelDatabase)
	{
		// Make sure that the provided path doesn't already exist.
		// If the file is NULL then we don't need to (and can't) close it.
		FILE* file = fopen(pathToNewVoxelDatabase.c_str(), "rb");
		if (file != NULL)
		{
			fclose(file);
			POLYVOX_THROW(std::invalid_argument, "Cannot create a new voxel database as the provided filename (", pathToNewVoxelDatabase, ") already exists");
		}

		POLYVOX_LOG_INFO("Creating empty voxel database as '", pathToNewVoxelDatabase, "'");
		VoxelDatabase<VoxelType>* voxelDatabase = new VoxelDatabase<VoxelType>;
		EXECUTE_SQLITE_FUNC(sqlite3_open_v2(pathToNewVoxelDatabase.c_str(), &(voxelDatabase->mDatabase), SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL));

		// Create the 'Properties' table.
		EXECUTE_SQLITE_FUNC(sqlite3_exec(voxelDatabase->mDatabase, "CREATE TABLE Properties(Name TEXT PRIMARY KEY, Value TEXT);", 0, 0, 0));

		// Create the 'Blocks' table. Not sure we need 'ASC' here, but it's in the example (http://goo.gl/NLHjQv) as is the default anyway.
		EXECUTE_SQLITE_FUNC(sqlite3_exec(voxelDatabase->mDatabase, "CREATE TABLE Blocks(Region INTEGER PRIMARY KEY ASC, Data BLOB);", 0, 0, 0));

		voxelDatabase->initialize();
		return voxelDatabase;
	}

	template <typename VoxelType>
	VoxelDatabase<VoxelType>* VoxelDatabase<VoxelType>::createFromVDB(const std::string& pathToExistingVoxelDatabase, WritePermission writePermission)
	{
		// When creating a new empty voxel database the user can pass an empty string to signify that 
		// the database will be temporary, but when creating from a VDB a valid path must be provided.
		POLYVOX_THROW_IF(pathToExistingVoxelDatabase.empty(), std::invalid_argument, "Path must not be an empty string");

		POLYVOX_LOG_INFO("Creating voxel database from '", pathToExistingVoxelDatabase, "'");
		VoxelDatabase<VoxelType>* voxelDatabase = new VoxelDatabase<VoxelType>;
		int flags = (writePermission == WritePermissions::ReadOnly) ? SQLITE_OPEN_READONLY : SQLITE_OPEN_READWRITE;
		EXECUTE_SQLITE_FUNC(sqlite3_open_v2(pathToExistingVoxelDatabase.c_str(), &(voxelDatabase->mDatabase), flags, NULL));

		// If the database was requested with write permissions but only read-only was possible, then SQLite opens it in read-only mode 
		// instead. This is undisirable for us as we would rather know that it has failed. In one case this was due to a user having the
		// VDB in source control, and is is desirable to give an error so that the user knows they need to check out the database.
		if ((writePermission == WritePermissions::ReadWrite) && (sqlite3_db_readonly(voxelDatabase->mDatabase, "main") == 1))
		{
			EXECUTE_SQLITE_FUNC(sqlite3_close(voxelDatabase->mDatabase));
			POLYVOX_THROW(std::runtime_error, "Voxel database could not be opened with requested 'write' permissions (only read-only was possible)");
		}

		voxelDatabase->initialize();
		return voxelDatabase;
	}

	template <typename VoxelType>
	void VoxelDatabase<VoxelType>::initialize(void)
	{
		// Disable syncing
		EXECUTE_SQLITE_FUNC(sqlite3_exec(mDatabase, "PRAGMA synchronous = OFF", 0, 0, 0));

		// Now create the 'OverrideChunks' table. Not sure we need 'ASC' here, but it's in the example (http://goo.gl/NLHjQv) and is the default anyway.
		// Note that the table cannot already exist because it's created as 'TEMP', and is therefore stored in a seperate temporary database.
		// It appears this temporary table is not shared between connections (multiple volumes using the same VDB) which is probably desirable for us
		// as it means different instances of the volume can be modified (but not commited to) without interfering with each other (http://goo.gl/aDKyId).
		EXECUTE_SQLITE_FUNC(sqlite3_exec(mDatabase, "CREATE TEMP TABLE OverrideChunks(Region INTEGER PRIMARY KEY ASC, Data BLOB);", 0, 0, 0));

		// Now build the 'insert or replace' prepared statements
		EXECUTE_SQLITE_FUNC(sqlite3_prepare_v2(mDatabase, "INSERT OR REPLACE INTO Blocks (Region, Data) VALUES (?, ?)", -1, &mInsertOrReplaceBlockStatement, NULL));
		EXECUTE_SQLITE_FUNC(sqlite3_prepare_v2(mDatabase, "INSERT OR REPLACE INTO OverrideChunks (Region, Data) VALUES (?, ?)", -1, &mInsertOrReplaceOverrideChunkStatement, NULL));

		// Now build the 'select' prepared statements
		EXECUTE_SQLITE_FUNC(sqlite3_prepare_v2(mDatabase, "SELECT Data FROM Blocks WHERE Region = ?", -1, &mSelectChunkStatement, NULL));
		EXECUTE_SQLITE_FUNC(sqlite3_prepare_v2(mDatabase, "SELECT Data FROM OverrideChunks WHERE Region = ?", -1, &mSelectOverrideChunkStatement, NULL));

		// Now build the 'select' and 'insert or replace' prepared statements
		EXECUTE_SQLITE_FUNC(sqlite3_prepare_v2(mDatabase, "SELECT Value FROM Properties WHERE Name = ?", -1, &mSelectPropertyStatement, NULL));
		EXECUTE_SQLITE_FUNC(sqlite3_prepare_v2(mDatabase, "INSERT OR REPLACE INTO Properties (Name, Value) VALUES (?, ?)", -1, &mInsertOrReplacePropertyStatement, NULL));
	}

	template <typename VoxelType>
	void VoxelDatabase<VoxelType>::pageIn(const PolyVox::Region& region, typename PolyVox::PagedVolume<VoxelType>::Chunk* pChunk)
	{
		POLYVOX_ASSERT(pChunk, "Attempting to page in NULL chunk");

		PolyVox::Timer timer;

		int64_t key = regionToKey(region);

		const void* compressedData = nullptr;
		int compressedLength = 0;

		// First we try and read the data from the OverrideChunks table
		// Based on: http://stackoverflow.com/a/5308188
		sqlite3_reset(mSelectOverrideChunkStatement);
		sqlite3_bind_int64(mSelectOverrideChunkStatement, 1, key);
		if(sqlite3_step(mSelectOverrideChunkStatement) == SQLITE_ROW)
        {
			// I think the last index is zero because our select statement only returned one column.
			compressedLength = sqlite3_column_bytes(mSelectOverrideChunkStatement, 0);
			compressedData = sqlite3_column_blob(mSelectOverrideChunkStatement, 0);
        }
		else
		{
			// In this case the chunk data wasn't found in the override table, so we go to the real Chunks table.
			sqlite3_reset(mSelectChunkStatement);
			sqlite3_bind_int64(mSelectChunkStatement, 1, key);
			if(sqlite3_step(mSelectChunkStatement) == SQLITE_ROW)
			{
				// I think the last index is zero because our select statement only returned one column.
				compressedLength = sqlite3_column_bytes(mSelectChunkStatement, 0);
				compressedData = sqlite3_column_blob(mSelectChunkStatement, 0);
			}
		}

		// The data might not have been found in the database, in which case
		// we leave the chunk in it's default state (initialized to zero).
		if (compressedData)
		{
			mz_ulong uncomp_len = pChunk->getDataSizeInBytes();
			int status = uncompress((unsigned char*)pChunk->getData(), &uncomp_len, (const unsigned char*)compressedData, compressedLength);
			POLYVOX_THROW_IF(status != Z_OK, CompressionError, "Decompression failed with error message \'", mz_error(status), "\'");

			// Data on disk is stored in linear order because so far we have not been able to show that Morton order
			// has better compression. But data in memory has Morton order because it is (probably) faster to access.
			pChunk->changeLinearOrderingToMorton();
		}

		POLYVOX_LOG_TRACE("Paged chunk in in ", timer.elapsedTimeInMilliSeconds(), "ms");
	}

	template <typename VoxelType>
	void VoxelDatabase<VoxelType>::pageOut(const PolyVox::Region& region, typename PolyVox::PagedVolume<VoxelType>::Chunk* pChunk)
	{
		POLYVOX_ASSERT(pChunk, "Attempting to page out NULL chunk");

		PolyVox::Timer timer;

		POLYVOX_LOG_TRACE("Paging out data for ", region);

		// Data on disk is stored in linear order because so far we have not been able to show that Morton order
		// has better compression. But data in memory has Morton order because it is (probably) faster to access.
		pChunk->changeMortonOrderingToLinear();

		// Prepare for compression
		uLong srcLength = pChunk->getDataSizeInBytes();
		uLong compressedLength = compressBound(srcLength); // Gets update when compression happens
		if (mCompressedBuffer.size() != compressedLength)
		{
			// All chunks are the same size so should have the same upper bound. Therefore this should only happen once.
			POLYVOX_LOG_INFO("Resizing compressed data buffer to ", compressedLength, "bytes. This should only happen once");
			mCompressedBuffer.resize(compressedLength);
		}

		// Perform the compression, and update passed parameter with the new length.
		int status = compress(&(mCompressedBuffer[0]), &compressedLength, (const unsigned char *)pChunk->getData(), srcLength);
		POLYVOX_THROW_IF(status != Z_OK, CompressionError, "Compression failed with error message \'", mz_error(status), "\'");

		int64_t key = regionToKey(region);

		// Based on: http://stackoverflow.com/a/5308188
		sqlite3_reset(mInsertOrReplaceOverrideChunkStatement);
		sqlite3_bind_int64(mInsertOrReplaceOverrideChunkStatement, 1, key);
		sqlite3_bind_blob(mInsertOrReplaceOverrideChunkStatement, 2, static_cast<const void*>(&(mCompressedBuffer[0])), compressedLength, SQLITE_TRANSIENT);
		sqlite3_step(mInsertOrReplaceOverrideChunkStatement);

		POLYVOX_LOG_TRACE("Paged chunk out in ", timer.elapsedTimeInMilliSeconds(), "ms (", pChunk->getDataSizeInBytes(), "bytes of data)");
	}

	template <typename VoxelType>
	void VoxelDatabase<VoxelType>::acceptOverrideChunks(void)
	{
		EXECUTE_SQLITE_FUNC( sqlite3_exec(mDatabase, "INSERT OR REPLACE INTO Blocks (Region, Data) SELECT Region, Data from OverrideChunks;", 0, 0, 0) );

		// The override chunks have been copied accross so we
		// can now discard the contents of the override table.
		discardOverrideChunks();
	}

	template <typename VoxelType>
	void VoxelDatabase<VoxelType>::discardOverrideChunks(void)
	{
		EXECUTE_SQLITE_FUNC( sqlite3_exec(mDatabase, "DELETE FROM OverrideChunks;", 0, 0, 0) );
	}

	template <typename VoxelType>
	bool VoxelDatabase<VoxelType>::getProperty(const std::string& name, std::string& value)
	{
		EXECUTE_SQLITE_FUNC(sqlite3_reset(mSelectPropertyStatement));
		EXECUTE_SQLITE_FUNC(sqlite3_bind_text(mSelectPropertyStatement, 1, name.c_str(), -1, SQLITE_TRANSIENT));
		if (sqlite3_step(mSelectPropertyStatement) == SQLITE_ROW)
		{
			// I think the last index is zero because our select statement only returned one column.
			value = std::string(reinterpret_cast<const char*>(sqlite3_column_text(mSelectPropertyStatement, 0)));
			return true;
		}
		else
		{
			POLYVOX_LOG_WARNING("Property '", name, "' was not found. The default value will be used instead");
			return false;
		}
	}

	template <typename VoxelType>
	int32_t VoxelDatabase<VoxelType>::getPropertyAsInt(const std::string& name, int32_t defaultValue)
	{
		std::string value;
		if (getProperty(name, value))
		{
			return ::atol(value.c_str());
		}

		return defaultValue;
	}

	template <typename VoxelType>
	float VoxelDatabase<VoxelType>::getPropertyAsFloat(const std::string& name, float defaultValue)
	{
		std::string value;
		if (getProperty(name, value))
		{
			return ::atof(value.c_str());
		}

		return defaultValue;
	}

	template <typename VoxelType>
	std::string VoxelDatabase<VoxelType>::getPropertyAsString(const std::string& name, const std::string& defaultValue)
	{
		std::string value;
		if (getProperty(name, value))
		{
			return value;
		}

		return defaultValue;
	}

	template <typename VoxelType>
	void VoxelDatabase<VoxelType>::setProperty(const std::string& name, int value)
	{
		std::stringstream ss;
		ss << value;
		setProperty(name, ss.str());
	}

	template <typename VoxelType>
	void VoxelDatabase<VoxelType>::setProperty(const std::string& name, float value)
	{
		std::stringstream ss;
		ss << value;
		setProperty(name, ss.str());
	}

	template <typename VoxelType>
	void VoxelDatabase<VoxelType>::setProperty(const std::string& name, const std::string& value)
	{
		// Based on: http://stackoverflow.com/a/5308188
		EXECUTE_SQLITE_FUNC(sqlite3_reset(mInsertOrReplacePropertyStatement));
		EXECUTE_SQLITE_FUNC(sqlite3_bind_text(mInsertOrReplacePropertyStatement, 1, name.c_str(), -1, SQLITE_TRANSIENT));
		EXECUTE_SQLITE_FUNC(sqlite3_bind_text(mInsertOrReplacePropertyStatement, 2, value.c_str(), -1, SQLITE_TRANSIENT));
		sqlite3_step(mInsertOrReplacePropertyStatement); //Don't wrap this one as it isn't supposed to return SQLITE_OK?
	}
}

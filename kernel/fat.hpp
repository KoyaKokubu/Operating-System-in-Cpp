/*
* file collecting programs to manipulate FAT file system
*/

#pragma once

#include <cstdint>
#include <cstddef>

#include "error.hpp"
#include "file.hpp"

namespace fat {
    struct BPB {
        uint8_t jump_boot[3];
        char oem_name[8];
        uint16_t bytes_per_sector;
        uint8_t sectors_per_cluster;
        uint16_t reserved_sector_count;
        uint8_t num_fats;
        uint16_t root_entry_count;
        uint16_t total_sectors_16;
        uint8_t media;
        uint16_t fat_size_16;
        uint16_t sectors_per_track;
        uint16_t num_heads;
        uint32_t hidden_sectors;
        uint32_t total_sectors_32;
        uint32_t fat_size_32;
        uint16_t ext_flags;
        uint16_t fs_version;
        uint32_t root_cluster;
        uint16_t fs_info;
        uint16_t backup_boot_sector;
        uint8_t reserved[12];
        uint8_t drive_number;
        uint8_t reserved1;
        uint8_t boot_signature;
        uint32_t volume_id;
        char volume_label[11];
        char fs_type[8];
    } __attribute__((packed));

    enum class Attribute : uint8_t {
        kReadOnly   = 0x01,
        kHidden     = 0x02,
        kSystem     = 0x04,
        kVolumeID   = 0x08,
        kDirectory  = 0x10,
        kArchive    = 0x20,
        kLongName   = 0x0f,
    };

    struct DirectoryEntry {
        unsigned char name[11];
        Attribute attr;
        uint8_t ntres;
        uint8_t create_time_tenth;
        uint16_t create_time;
        uint16_t create_data;
        uint16_t last_access_date;
        uint16_t first_cluster_high;
        uint16_t write_time;
        uint16_t write_date;
        uint16_t first_cluster_low;
        uint32_t file_size;

        uint32_t FirstCluster() const {
            return first_cluster_low |
                (static_cast<uint32_t>(first_cluster_high) << 16);
        }
    } __attribute__((packed));

    extern BPB* boot_volume_image;
    extern unsigned long bytes_per_cluster;
    void Initialize(void* volume_image);

    /** @brief return memory address of the head sector of the given cluster.
    *
    *   @param cluster : cluster number (from 2) 
    *   @return address of the memory region where the head sector of the cluster is located. 
    */
    uintptr_t GetClusterAddr(unsigned long cluster);

    /** @brief return the memory region where the head sector of the given cluster is located. 
    *
    *   @param cluster : cluster number (from 2) 
    *   return pointer to the memory region where the head sector of the cluster is located.
    */
    template <class T>
    T* GetSectorByCluster(unsigned long cluster) {
        return reinterpret_cast<T*>(GetClusterAddr(cluster));
    }

    /** @brief get short name of the given directory entry by dividing into base name and file extention.
    *   space character (0x20) will be removed, and null-terminated
    *
    *   @param entry : the target directory entry to get the file name
    *   @param base : file name other than file extention (array with the size more than 9 bytes) 
    *   @param ext : file extention (array with the size more than 4 bytes)
    */
    void ReadName(const DirectoryEntry& entry, char* base, char* ext);

		/** @brief copy short name of directory entry into dest.
		*		If extention of short name does not exit, copy "<base>". Otherwise, copy "<base>.<ext>". 
		*
		*		@param enry : directory entry to get a file name
		*		@param dest : large enough array to store string combining base name and extention.
		*/
		void FormatName(const DirectoryEntry& entry, char* dest);

		static const unsigned long kEndOfClusterchain = 0x0ffffffflu;

    /** @brief return a number of next cluster of the given cluster
    *
    *   @param cluster : cluster number
    *   @return next cluster number (if no next cluster, kEndOfClusterchain)
    */
    unsigned long NextCluster(unsigned long cluster);

    /** @brief find the file from the given directory
    *
    *   @param name : 8+3 file name (not distinguish upper and lower character) 
    *   @param directory_cluster start cluster of a directory (Default: root directory) 
    *   return the pair of file or entry indicating directory, and flag indicating end "/".
		*		If can not find file or directory, nullptr. 
		*		If there is a slash right after entry, true.
		*		If there is a file in a path, quit finding and return the entry and true.
    */
		std::pair<DirectoryEntry*, bool>
    FindFile(const char* path, unsigned long directory_cluster = 0);

    bool NameIsEqual(const DirectoryEntry& entry, const char* name);

    /** @brief copy the given file into buffer
    *
    *   @param buf : copy into this buffer
    *   @param len : buffer length (per bytes) 
    *   @param entry : directory entry indicating a file
    *   @return length of bytes being read 
    */
    size_t LoadFile(void* buf, size_t len, DirectoryEntry& entry);

		bool IsEndOfClusterchain(unsigned long cluster);

		uint32_t* GetFAT();

		/**	@brief add given number of clusters into cluster chain
		*
		*		@param eoc_cluster : a cluster number which belongs to the extending cluster chain.
		*		@param n : the number of clusters to add
		*		@return cluster number of the end of extended chain
		*/
		unsigned long ExtendCluster(unsigned long eoc_cluster, size_t n);

		/**	@brief return one empty entry in the given directory.
		*		If there is no empty entry, allocate the empty entry by extending one.
		*
		*		@param dir_cluster : find an empty entry in this directory
		*		@return an empty entry
		*/
		DirectoryEntry* AllocateEntry(unsigned long dir_cluster);

		/** @brief set short file name to directory entry
		*
		*		@param entry : directory entry to be set a name
		*		@param name : file name integrating base name and entention with dot. 
		*/
		void SetFileName(DirectoryEntry& entry, const char* name);

		/** @brief create a file entry in the given path. 
		*
		*		@param path : file path
		*		@return newly created file entry 
		*/
		WithError<DirectoryEntry*> CreateFile(const char* path);

		/** @brief construct a chain with the number os clusters
		*
		*		@param n : the number of clusters
		*		@return : a number of the head cluster of the constructed chain
		*/
		unsigned long AllocateClusterChain(size_t n);

		class FileDescriptor : public ::FileDescriptor {
				public:
						explicit FileDescriptor(DirectoryEntry& fat_entry);
						size_t Read(void* buf, size_t len) override;
						size_t Write(const void* buf, size_t len) override;
						size_t Size() const override { return fat_entry_.file_size; }
						size_t Load(void* buf, size_t len, size_t offset) override;
				
				private:
						DirectoryEntry& fat_entry_;
						size_t rd_off_ = 0;
						unsigned long rd_cluster_ = 0;
						size_t rd_cluster_off_ = 0;
						size_t wr_off_ = 0;
						unsigned long wr_cluster_ = 0;
						size_t wr_cluster_off_ = 0;
		};

} // namespace fat
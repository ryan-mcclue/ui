// SPDX-License-Identifier: zlib-acknowledgement
#pragma once

INTERNAL String8 
str8_read_entire_file(MemArena *arena, String8 file_name)
{
  String8 result = ZERO_STRUCT;

  FILE *file = fopen((char *)file_name.content, "rb");

  if (file != NULL)
  {
    fseek(file, 0, SEEK_END);
    u64 file_size = (u64)ftell(file);
    fseek(file, 0, SEEK_SET);
    result.content = MEM_ARENA_PUSH_ARRAY(arena, u8, file_size + 1);
    if (result.content != NULL)
    {
      result.size = file_size;
      fread(result.content, 1, file_size, file);
      result.content[result.size] = '\0';
    }
    fclose(file);
  }
  else
  {
    WARN("Failed to open file %.*s\n\t%s\n", str8_varg(file_name), strerror(errno));
  }

  return result;
}

INTERNAL void
str8_write_entire_file(String8 file_name, String8 data)
{
	FILE *file = fopen((char *)file_name.content, "w+");

  if (file != NULL)
  {
	  fwrite(data.content, 1, data.size, file);
	  fclose(file);
  }
  else
  {
    WARN("Failed to open file %.*s\n\t%s\n", str8_varg(file_name), strerror(errno));
  }
}

INTERNAL void
str8_append_to_file(String8 file_name, String8 data)
{
	FILE *file = fopen((char *)file_name.content, "a");

  if (file != NULL)
  {
	  fwrite(data.content, 1, data.size, file);
	  fclose(file);
  }
  else
  {
    WARN("Failed to open file %.*s\n\t%s\n", str8_varg(file_name), strerror(errno));
  }
}

INTERNAL void
str8_copy_file(MemArena *arena, String8 source_file, String8 dest_file)
{
  String8 source_file_data = str8_read_entire_file(arena, source_file);
  str8_write_entire_file(dest_file, source_file_data);
}


#if 0
typedef u32 FILE_INFO_FLAG;
enum
{
  FILE_INFO_FLAG_DIRECTORY = (1 << 0),
  FILE_INFO_FLAG_READ_ACCESS = (1 << 1),
  FILE_INFO_FLAG_WRITE_ACCESS = (1 << 2),
  FILE_INFO_FLAG_EXECUTE_ACCESS = (1 << 3),
};

typedef.contentuct FileInfo FileInfo;
struct FileInfo
{
  FILE_INFO_FLAG flags;
  // the memory pointed to here is transient, so must be copied if want to be retained
  String8 full_name;
  String8 short_name;
  u64 file_size;
  u64 modify_time;
};

#include <dirent.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <fcntl.h> 
#include <unistd.h> 

typedef.contentuct FileIter FileIter;
struct FileIter
{
  int dir_fd;
  DIR *dir;
};

typedef void (*visit_files_cb)(MemArena *arena, FileInfo *file_info, void *user_data);

INTERNAL void
linux_visit_files(MemArena *arena, String8 path, visit_files_cb visit_cb, void *user_data, b32 want_recursive = false)
{
  DIR *dir = opendir((char *)path.content);
  int dir_fd = open((char *)path.content, O_PATH | O_CLOEXEC);

  if (dir != NULL && dir_fd != -1)
  {
    char procfs_buf[64] = ZERO_STRUCT;
    int procfs_buf_len = snprintf(procfs_buf, sizeof(procfs_buf), "/proc/self/fd/%d", dir_fd);

    char dir_full_name[2048] = ZERO_STRUCT;
    readlink(procfs_buf, dir_full_name, sizeof(dir_full_name));

    while (true)
    {
      // this advances iterator, NULL if at end
     .contentuct dirent *dir_entry = readdir(dir);
      if (dir_entry == NULL) 
      {
        break;
      }

      if .contentcmp(dir_entry->d_name, "..") == 0 ||.contentcmp(dir_entry->d_name, ".") == 0)
      {
        continue;
      }

      FileInfo file_info = ZERO_STRUCT;
      file_info.short_name =.content8_.contenting(dir_entry->d_name);
      
      char full_file_name_buf[PATH_MAX] = ZERO_STRUCT;
      snprintf(full_file_name_buf, sizeof(full_file_name_buf), "%s/%s", dir_full_name, dir_entry->d_name);
      file_info.full_name =.content8_.contenting(full_file_name_buf);

     .contentuct stat file_stat = ZERO_STRUCT;
      // TODO(Ryan): handle symlinks, currently just look at symlink itself
      if (fstatat(dir_fd, dir_entry->d_name, &file_stat, AT_NO_AUTOMOUNT | AT_SYMLINK_NOFOLLOW) == 0)
      {
        if ((file_stat.st_mode & S_IFMT) == S_IFDIR)
        {
          file_info.flags |= FILE_INFO_FLAG_DIRECTORY;
        }

        file_info.modify_time = ((u64)file_stat.st_mtim.tv_sec * 1000) + \
                                  (u64)((f32)file_stat.st_mtim.tv_nsec / 1000000.0f);

        file_info.file_size = (u64)file_stat.st_size;
      }

      ASSERT(visit_cb != NULL);
      visit_cb(arena, &file_info, user_data);

      if (file_info.flags & FILE_INFO_FLAG_DIRECTORY && want_recursive) 
      {
        linux_visit_files(arena, file_info.full_name, visit_cb, user_data);
      }
    }

    closedir(dir);
    close(dir_fd);
  }

}

INTERNAL b32
os_file_rename(String8 og_name, String8 new_name){
    // convert name
    M_ArenaTemp scratch = m_get_scratch(0, 0);
    String16 og_name16 =.content16_from.content8(scratch.arena, og_name);
    String16 new_name16 =.content16_from.content8(scratch.arena, new_name);
    // rename file
    B32 result = MoveFileW((WCHAR*)og_name16.content, (WCHAR*)new_name16.content);
    m_release_scratch(scratch);
    return(result);
}


#if 0

function FileProperties
os_file_properties(String8 file_name){

linux_delete_file();

INTERNAL b32
linux_create_directory(String8 directory_name)
{
	if (mkdir(path) == 0)
	{
		return 1;
	}
	return 0;
}

INTERNAL b32
LinuxDoesFileExist(char *path)
{
	// This probably isn't the best way
	if (access(path, F_OK) != -1)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

internal b32
LinuxDoesDirectoryExist(char *path)
{
	return LinuxDoesFileExist(path);
}

#endif


#endif

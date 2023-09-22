// SPDX-License-Identifier: zlib-acknowledgement
#pragma once

#include <ctype.h>
#include <stdarg.h>

typedef struct String8 String8;
struct String8
{
  u8 *content;
  memory_index size;
};

typedef u32 MATCH_FLAGS;
typedef u32 S8_MATCH_FLAGS;
enum
{
  MATCH_FLAG_FIND_LAST = (1 << 0),
};
enum
{
  S8_MATCH_FLAG_RIGHT_SIDE_LAZY = (1 << 4),
  S8_MATCH_FLAG_CASE_INSENSITIVE = (1 << 5),
};

typedef struct String8Node String8Node;
struct String8Node
{
  String8Node *next;
  String8 string;
};

typedef struct String8List String8List;
struct String8List
{
  String8Node *first;
  String8Node *last;
  u32 node_count;
  memory_index total_size;
};

typedef struct String8Join String8Join;
struct String8Join
{
  String8 pre;
  String8 mid;
  String8 post;
};

INTERNAL String8
str8(u8 *str, memory_index size)
{
  String8 result = ZERO_STRUCT;

  result.content = str;
  result.size = size;

  return result;
}

INTERNAL String8
str8_allocate(MemArena *arena, memory_index len)
{
  String8 result = ZERO_STRUCT;

  result.content = MEM_ARENA_PUSH_ARRAY_ZERO(arena, u8, len);
  result.size = len;

  return result;
}

INTERNAL char *
str8_to_cstr(MemArena *arena, String8 str)
{
  char *result = MEM_ARENA_PUSH_ARRAY_ZERO(arena, char, str.size + 1);

  MEMORY_COPY(result, str.content, str.size);
  result[str.size] = '\0';

  return result;
}

#define str8_lit(s) str8((u8 *)(s), sizeof(s) - 1)
#define str8_cstr(s) str8((u8 *)(s), strlen((char *)s))
// IMPORTANT(Ryan): When substringing will run into situations where not null terminated.
// So, use like: "%.*s", str8_varg(string)
#define str8_varg(s) (int)(s).size, (s).content

#define PRINT_STR8(var) printf(STRINGIFY(var) " = %.*s\n", str8_varg(var))

INTERNAL String8
str8_up_to(u8 *start, u8 *up_to)
{
  String8 string = ZERO_STRUCT;

  string.content = start;
  string.size = (memory_index)(up_to - start);;

  return string;
}

INTERNAL String8
str8_substring(String8 str, memory_index start, memory_index end)
{
  if (end > str.size)
  {
    end = str.size;
  }

  if (start > str.size)
  {
    start = str.size;
  }

  if (start > end)
  {
    SWAP(memory_index, start, end);
  }

  str.size = end - start;
  str.content += start;

  return str;
}

INTERNAL String8
str8_advance(String8 str, memory_index advance)
{
  return str8_substring(str, advance, str.size);
}

INTERNAL String8
str8_chop(String8 str, memory_index chop)
{
  return str8_substring(str, 0, str.size - chop);
}

INTERNAL String8
str8_prefix(String8 str, memory_index prefix)
{
  return str8_substring(str, 0, prefix);
}

INTERNAL String8
str8_suffix(String8 str, memory_index suffix)
{
  return str8_substring(str, str.size - suffix, str.size);
}

INTERNAL b32
str8_match(String8 a, String8 b, S8_MATCH_FLAGS flags)
{
  b32 result = false;

  if (a.size == b.size || flags & S8_MATCH_FLAG_RIGHT_SIDE_LAZY)
  {
    result = true;

    for (memory_index i = 0; i < a.size && i < b.size; i += 1)
    {
      b32 match = (a.content[i] == b.content[i]);

      if (flags & S8_MATCH_FLAG_CASE_INSENSITIVE)
      {
        match |= (tolower(a.content[i]) == tolower(b.content[i]));
      }

      if (!match)
      {
        result = false;
        break;
      }
    }
  }

  return result;
}

INTERNAL memory_index
str8_find_substring(String8 str, String8 substring, memory_index start_pos, MATCH_FLAGS flags)
{
  b32 found = false;
  memory_index found_idx = str.size;

  for (memory_index i = start_pos; i < str.size; i += 1)
  {
    if (i + substring.size <= str.size)
    {
      String8 substr_from_str = str8_substring(str, i, i + substring.size);

      if (str8_match(substr_from_str, substring, flags))
      {
        found_idx = i;
        found = true;

        if (!(flags & MATCH_FLAG_FIND_LAST))
        {
          break;
        }
      }
    }
  }

  return found_idx;
}

INTERNAL String8
str8_chop_by_delim(String8 *str, String8 delim)
{
  String8 result = *str;

  u32 delim_i = str8_find_substring(*str, delim, 0, 0); 
  if (delim_i == str->size) return result;

  result.size = delim_i;

  str->content += (delim_i + 1);
  str->size -= (delim_i + 1);

  return result;
}

INTERNAL String8
str8_copy(MemArena *arena, String8 string)
{
  String8 result = ZERO_STRUCT;

  result.size = string.size;
  result.content = MEM_ARENA_PUSH_ARRAY(arena, u8, string.size + 1);
  MEMORY_COPY(result.content, string.content, string.size);
  result.content[string.size] = '\0';

  return result;
}

INTERNAL String8
str8_fmt(MemArena *arena, const char *fmt, ...)
{
  // IMPORTANT(Ryan): va_list is incremented behind-the-scenes when used.
  // So, require a copy to maintain same value across function calls
  va_list args, args_copy;
  va_start(args, fmt);
  va_copy(args_copy, args);

  String8 result = ZERO_STRUCT;
  memory_index needed_bytes = (memory_index)vsnprintf(NULL, 0, fmt, args_copy) + 1;
  result.content = MEM_ARENA_PUSH_ARRAY(arena, u8, needed_bytes);
  result.size = needed_bytes - 1;
  result.content[needed_bytes - 1] = '\0';
  vsnprintf((char *)result.content, (size_t)needed_bytes, fmt, args);

  va_end(args);
  va_end(args_copy);

  return result;
}

INTERNAL void
str8_list_push(MemArena *arena, String8List *list, String8 string)
{
  String8Node *node = MEM_ARENA_PUSH_ARRAY_ZERO(arena, String8Node, 1);
  node->string = string;

  SLL_QUEUE_PUSH(list->first, list->last, node);

  list->node_count += 1;
  list->total_size += string.size;
}

INTERNAL void
str8_list_push_fmt(MemArena *arena, String8List *list, char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);

  String8 string = str8_fmt(arena, fmt, args);

  va_end(args);

  str8_list_push(arena, list, string);
}

INTERNAL void
str8_list_concat(String8List *list, String8List *to_push)
{
  if (to_push->first)
  {
    list->node_count += to_push->node_count;
    list->total_size += to_push->total_size;

    if (list->last == NULL)
    {
      *list = *to_push;
    }
    else
    {
      list->last->next = to_push->first;
      list->last = to_push->last;
    }
  }

  MEMORY_ZERO_STRUCT(to_push);
}

INTERNAL String8List
str8_split(MemArena *arena, String8 string, int splitter_count, String8 *splitters)
{
  String8List list = ZERO_STRUCT;

  memory_index split_start = 0;
  for(memory_index i = 0; i < string.size; i += 1)
  {
    b32 was_split = 0;
    for (int split_idx = 0; split_idx < splitter_count; split_idx += 1)
    {
      b32 match = 0;
      if (i + splitters[split_idx].size <= string.size)
      {
        match = 1;
        for (memory_index split_i = 0; split_i < splitters[split_idx].size && i + split_i < string.size; split_i += 1)
        {
          if (splitters[split_idx].content[split_i] != string.content[i + split_i])
          {
            match = 0;
            break;
          }
        }
      }
      if (match)
      {
        String8 split_string = str8(string.content + split_start, i - split_start);
        str8_list_push(arena, &list, split_string);
        split_start = i + splitters[split_idx].size;
        i += splitters[split_idx].size - 1;
        was_split = 1;
        break;
      }
    }

    if (was_split == 0 && i == string.size - 1)
    {
      String8 split_string = str8(string.content + split_start, i+1 - split_start);
      str8_list_push(arena, &list, split_string);
      break;
    }
  }

  return list;
}

INTERNAL String8
str8_list_join(MemArena *arena, String8List list, String8Join *join_ptr)
{
  // setup join parameters
  String8Join join = ZERO_STRUCT;
  if (join_ptr != NULL)
  {
    MEMORY_COPY(&join, join_ptr, sizeof(join));
  }

  // calculate size & allocate
  memory_index sep_count = 0;
  if (list.node_count > 1)
  {
    sep_count = list.node_count - 1;
  }

  String8 result = ZERO_STRUCT;
  result.size = (list.total_size + join.pre.size + sep_count*join.mid.size + join.post.size);
  result.content = MEM_ARENA_PUSH_ARRAY_ZERO(arena, u8, result.size);

  // fill
  u8 *ptr = result.content;
  MEMORY_COPY(ptr, join.pre.content, join.pre.size);
  ptr += join.pre.size;

  for (String8Node *node = list.first; node; node = node->next)
  {
    MEMORY_COPY(ptr, node->string.content, node->string.size);
    ptr += node->string.size;
    if (node != list.last)
    {
      MEMORY_COPY(ptr, join.mid.content, join.mid.size);
      ptr += join.mid.size;
    }
  }

  MEMORY_COPY(ptr, join.post.content, join.post.size);
  ptr += join.post.size;

  return(result);
}

typedef String8 U8Buf;
#define u8buf(buf, size) str8((u8 *)(buf), (size))

typedef struct RingBuf RingBuf;
struct RingBuf 
{
  union {
    U8Buf buf;
    ISO_EXTENSION struct 
    {
      u8 *content;
      memory_index size;
    };
  };

  memory_index write_pos; 
};

INTERNAL RingBuf
ring_buf_create(MemArena *arena, memory_index size)
{
  RingBuf result = ZERO_STRUCT;

  result.content = MEM_ARENA_PUSH_ARRAY_ZERO(arena, u8, size);
  result.size = size;
  result.write_pos = 0;

  return result;
}

INTERNAL memory_index
ring_buf_write(RingBuf *ring_buf, U8Buf buf)
{
  if (buf.size > ring_buf->size) return 0;

  U8Buf block1 = buf;
  memory_index block1_offset = ring_buf->write_pos % ring_buf->size;

  U8Buf block2 = ZERO_STRUCT;
  if (block1_offset + block1.size > ring_buf->size)
  {
    memory_index block2_advance = ring_buf->size - block1_offset;
    block2 = str8_advance(block1, block2_advance);
    block1.size = block2_advance;
  }

  if (block1.size != 0)
  {
    // IMPORTANT(Ryan): Confusion with stdlib bytes, elements, etc. Encouraging buffer overflows!
    MEMORY_COPY(ring_buf->content + block1_offset, block1.content, block1.size);
    ring_buf->write_pos += block1.size;
  }

  if (block2.size != 0)
  {
    MEMORY_COPY(ring_buf->content, block2.content, block2.size);
    ring_buf->write_pos = block2.size;
  }

  return buf.size;
}

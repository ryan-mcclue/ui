// SPDX-License-Identifier: zlib-acknowledgement

INTERNAL void
aoc1(void)
{
  MEM_ARENA_TEMP_BLOCK(temp, NULL, 0)
  {
    String8 input = str8_read_entire_file(temp.arena, str8_lit("assets/aoc1.txt"));

    String8 iter = input;
    while (iter.size > 0)
    {
      String8 line = str8_chop_by_delim(iter, str8_lit("\n"));
      u32 depth = str8_to_u64(line, 10);
      PRINT_U32(depth);
      iter = str8_advance(iter, line.size + 1);
    }
  }
}

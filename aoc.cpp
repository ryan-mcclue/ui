// SPDX-License-Identifier: zlib-acknowledgement
INTERNAL void
aoc1_1(void)
{
  MEM_ARENA_TEMP_BLOCK(temp, NULL, 0)
  {
    String8 input = str8_read_entire_file(temp.arena, str8_lit("assets/aoc1.txt"));

    String8 iter = input;
    s32 prev = -1;
    s32 amt = 0;
    while (iter.size > 0)
    {
      String8 line = str8_chop_by_delim(iter, str8_lit("\n"));
      s32 depth = str8_to_u64(line, 10);

      if (prev >= 0 && depth > prev) amt++;
      prev = depth;

      iter = str8_advance(iter, line.size + 1);
    }

    PRINT_S32(amt);
  }
}

INTERNAL String8List
populate_string_list(MemArena *arena, String8 input, b32 ilp)
{
  String8List res = ZERO_STRUCT;

  if (ilp)
  {
    u32 num_lines = 2000;
    u32 chars_per_line = 5;
    u32 approx_halfway = num_lines / 2 * chars_per_line;
    String8 approx_half = str8_advance(input, approx_halfway);
    String8 half = str8_advance_by_delim(approx_half, str8_lit("\n"));
    String8 second = str8_advance(half, 1);
    String8 first = str8_prefix(input, input.size - second.size);

    String8 iter_first = first;
    String8 iter_second = second;
    while (iter_first.size > 0 || iter_second.size > 0)
    {
      if (iter_first.size > 0)
      {
        String8 line = str8_chop_by_delim(iter_first, str8_lit("\n"));
        str8_list_push(arena, &res, line);
        iter_first = str8_advance(iter_first, line.size + 1);
      }
      if (iter_second.size > 0)
      {
        String8 line = str8_chop_by_delim(iter_second, str8_lit("\n"));
        str8_list_push(arena, &res, line);
        iter_second = str8_advance(iter_second, line.size + 1);
      }
    }
  }
  else
  {
    String8 iter = input;
    while (iter.size > 0)
    {
      String8 line = str8_chop_by_delim(iter, str8_lit("\n"));
      str8_list_push(arena, &res, line);
      iter = str8_advance(iter, line.size + 1);
    }
  }

  return res;
}

INTERNAL void
aoc1_2(void)
{
  PROFILE_FUNCTION()
  {

  MEM_ARENA_TEMP_BLOCK(temp, NULL, 0)
  {
    String8 input;
    PROFILE_BLOCK("read_file")
    {
    input = str8_read_entire_file(temp.arena, str8_lit("assets/aoc1.txt"));
    }

    String8List depths = ZERO_STRUCT;

    PROFILE_BANDWIDTH("populate_string_list", input.size)
    {
      depths = populate_string_list(temp.arena, input, true);
    }

    u32 *v = MEM_ARENA_PUSH_ARRAY(temp.arena, u32, depths.node_count);
    u32 i = 0;
    PROFILE_BLOCK("string_list_to_array")
    {
    // IMPORTANT(Ryan): Remember list sentinel condition to not skip last element!
    for (String8Node *n = depths.first; n != NULL; n = n->next)
    {
      v[i++] = str8_to_u64(n->string, 10);
    }
    }

    s32 prev_sum = -1;
    s32 amt = 0;
    PROFILE_BANDWIDTH("sum_computation", depths.node_count * sizeof(u32))
    {
    for (u32 d = 0; d < depths.node_count - 2; d += 1)
    {
      s32 sum = v[d] + v[d+1] + v[d+2];
      if (prev_sum >= 0 && sum > prev_sum) amt++;
      prev_sum = sum;
    }
    }

    PRINT_S32(amt);
  }

  }
}

struct Params
{
  String8 input;
  MemArena *arena;
};

INTERNAL void
repeat_populate_string_list(RepetitionTester *tester, Params *params)
{
  while (true)
  {
    String8 iter = params->input;
    String8List depths = ZERO_STRUCT;
    TIME_TEST(tester)
    {
      while (iter.size > 0)
      {
        String8 line = str8_chop_by_delim(iter, str8_lit("\n"));
        str8_list_push(params->arena, &depths, line);
        iter = str8_advance(iter, line.size + 1);
      }
    }
    mem_arena_pop(params->arena, depths.total_size);

    tester_count_bytes(tester, params->input.size);
    if (update_tester(tester) != TESTER_STATE_TESTING) break;
    //PRINT_U32(tester->state);
  }
}

INTERNAL void
test_run(void)
{
  MEM_ARENA_TEMP_BLOCK(temp, NULL, 0)
  {
    printf("\n--- %s ---\n", "populate string list");

    Params params = ZERO_STRUCT;
    params.input = str8_read_entire_file(temp.arena, str8_lit("assets/aoc1.txt"));
    params.arena = temp.arena;

    u64 cpu_timer_freq = linux_estimate_cpu_timer_freq();


    RepetitionTester tester = ZERO_STRUCT;
    tester.state = TESTER_STATE_TESTING;
    tester.cpu_timer_freq = cpu_timer_freq;
    tester.target_bytes_processed = params.input.size;
    tester.cpu_timer_freq = cpu_timer_freq;
    tester.min_time = U64_MAX;
    tester.repeat_time = 5*cpu_timer_freq;
    tester.start = read_cpu_timer();

    repeat_populate_string_list(&tester, &params);
  }
}

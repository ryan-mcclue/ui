// SPDX-License-Identifier: zlib-acknowledgement

// IMPORTANT(Ryan): lists + hash maps!
// map(node_ptr, <>)

// IMPORTANT(Ryan): f(offset field_off) -> MEMBER_FROM_OFF(struct, field_off)

// TODO(Ryan): Tree operations 
// 1. Creation (add onto parent). May require hashmap to find parent, e.g. directory
// 2. Printing (call on children). 
//    Order determined by actionable event in between children, level/in/post
// 3. Traversal; DFS and BFS

#if 0
INTERNAL Node *
dfs_get_next(Node *node)
{
  if (node == NULL) return NULL;

  Node *child = node->first_child;
  if (child != NULL) return child;

  Node *sibling = node->next;
  if (sibling != NULL) return sibling;

  for (Node *p = node->parent; p != NULL; p = p->parent)
  {
    Node *n = p->next;
    if (n != NULL) return n;
  }

  return NULL;
}

int 
main(int argc, char *argv[])
{
 String8List args_list = {0};
 for(U64 argument_idx = 1; argument_idx < argument_count; argument_idx += 1)
 {
  Str8ListPush(tctx.arenas[0], &args_list, Str8C(arguments[argument_idx]));
 }
 CmdLine cmdline = CmdLineFromStringList(tctx.arenas[0], args_list);

 main_func(&cmdline);

  return 0;
}
#endif

void obfuscate(void)
{
  String8 msg = str8_lit("hi there");

#define HIST_SIZE 256
  u32 msg_hist[HIST_SIZE] = ZERO_STRUCT;
  for (u32 i = 0; i < msg.size; i += 1)
  {
    msg_hist[(u32)msg.content[i]] += 1;
  }

#define TABLE_CAP 64
  char table[TABLE_CAP] = ZERO_STRUCT;
  u32 table_size = 0;
  for (u32 i = 0; i < HIST_SIZE; i += 1)
  {
    if (msg_hist[i] > 0) table[table_size++] = (char)i;
  }

  printf("Message: %.*s\nTable: %.*s\n", str8_varg(msg), table_size, table);
  // number represents table of characters
  // other number represents sequence of indices
  
  u32 indexes[TABLE_CAP] = ZERO_STRUCT;
  u32 indexes_size = 0;
  for (u32 i = 0; i < msg.size; i += 1)
  {
    for (u32 j = 0; j < table_size; j += 1)
    {
      if (msg.content[i] == table[j]) indexes[indexes_size++] = j;
    }
  }
  
  DEFER_LOOP(printf("{"), printf("}\n"))
  {
    for (u32 i = 0; i < indexes_size; i += 1)
    {
      if (i != indexes_size - 1) printf("%d, ", indexes[i]);
      else printf("%d", indexes[i]);
    }
  }

  const char *gen_table = " ehirt";
  u32 gen_indexes[] = {0, 2, 3, 0, 5, 2, 1, 4, 1};

  for (u32 i = 0; i < ARRAY_COUNT(gen_indexes); i += 1)
  {
    putchar(gen_table[gen_indexes[i]]);
  }
  printf("\n");

  // 6 byte table
  u64 table64 = *(u64 *)gen_table;
  PRINT_U64(table64);

  u64 hex_table = 0x7400747269686520;
  for (u32 i = 0; i < ARRAY_COUNT(gen_indexes); i += 1)
  {
    putchar((hex_table >> (8 * gen_indexes[i])) & 0xFF);
  }
  printf("\n");

  // see that each index can be represented by 3bits, 8 of them
  // pack indices from right to left as bit operations can only push/pop from the right 
  u32 index32 = 0;
  u32 num_indexes = ARRAY_COUNT(gen_indexes);
  for (u32 i = 0; i < num_indexes; i += 1)
  {
    index32 = (index32 << 3) | gen_indexes[num_indexes - i - 1];
  }
  PRINT_U32(index32);

  u32 gen_index = 0x18550d0;
  while (gen_index)
  {
    putchar((hex_table >> ((((gen_index >>= 3) & 7) << 3) & 0xFF)));
  }

}

#define ENUMERATE_TREEMAP_NODE(it, first) \
  struct {TreeMapNode *it; u32 i} e = {(first), 0}; (e.it != NULL); e.it = e.it->next, e.i++;

typedef struct Node Node;
struct Node
{
  // tree links
  Node *parent, *left, *right;
  u32 val;

  // iterative stack
  Node *stack_next;
};

INTERNAL void
visit_nodes(Node *p)
{
  if (p == NULL) return;

  printf("%d\n", p->val);
  visit_nodes(p->left);
  visit_nodes(p->right);
}

INTERNAL void
invert_tree(Node *p)
{
  if (p == NULL) return;

  Node *left = p->left;

  p->left = p->right;
  invert_tree(p->left);

  p->right = left;
  invert_tree(p->right);

  // recursion is used to create a stack of nodes
}

INTERNAL void
create_tree(MemArena *arena, Node *p, u32 height, u32 *i)
{
  if (height <= 0) return;

  u32 *counter = i;

  Node *left = MEM_ARENA_PUSH_STRUCT_ZERO(arena, Node);
  left->val = *counter;
  *counter = *counter + 1;
  p->left = left;
  create_tree(arena, left, height - 1, counter);
  
  Node *right = MEM_ARENA_PUSH_STRUCT_ZERO(arena, Node);
  right->val = *counter;
  *counter = *counter + 1;
  p->right = right;
  create_tree(arena, right, height - 1, counter);
}

INTERNAL void
dump_from_node(MemArena *arena, String8List *dump, Node *n, u32 indent, String8 indent_str)
{
  if (n == NULL) return;

#define PRINT_INDENT(_i) \
  for (u32 i = 0; i < (_i); i += 1) { str8_list_push(arena, dump, indent_str); }

  // NOTE(Ryan): Will print tree rotated left 90°, so swap sub-trees
  dump_from_node(arena, dump, n->right, indent + 1, indent_str);

  PRINT_INDENT(indent);
  String8 node_value = str8_fmt(arena, "%d\n", n->val);
  str8_list_push(arena, dump, node_value); 

  dump_from_node(arena, dump, n->left, indent + 1, indent_str);

#undef PRINT_INDENT
}

typedef struct NodeStack NodeStack;
struct NodeStack
{
  Node *node;
  u32 indent;
  Node *next;
};

INTERNAL void
print_node_it(Node *n)
{
#define PUSH(node) __SLL_STACK_PUSH(first, node, stack_next)
#define POP() __SLL_STACK_POP(first, stack_next)

  BP();
  // TODO(Ryan): To emulate the callstack, the stack element will be all the arguments in the recursive function
  Node *first = NULL; 
  PUSH(n); // equate to actionable?
  
  while (first != NULL)
  {
    // IMPORTANT(Ryan): This order matters!
    Node *node = first;
    POP();

    printf("%d\n", node->val);

    if (node->right != NULL) PUSH(node->right);
    if (node->left != NULL) PUSH(node->left);
  }

#undef POP
#undef PUSH
}

INTERNAL void
print_node(Node *n, u32 indent)
{
  MEM_ARENA_TEMP_BLOCK()
  {
    String8List dump = ZERO_STRUCT;

    dump_from_node(temp.arena, &dump, n, 0, str8_lit(" "));

    String8Join join = ZERO_STRUCT;
    String8 dump_str = str8_list_join(temp.arena, dump, &join);

    printf("%.*s", str8_varg(dump_str));
  }
}


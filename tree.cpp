// SPDX-License-Identifier: zlib-acknowledgement

// IMPORTANT(Ryan): lists + hash maps!
// map(node_ptr, <>)

// IMPORTANT(Ryan): f(offset field_off) -> MEMBER_FROM_OFF(struct, field_off)

// TODO(Ryan): Tree operations 
// 1. Creation (add onto parent). May require hashmap to find parent, e.g. directory
// 2. Printing (print on side).
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

#define ENUMERATE_TREEMAP_NODE(it, first) \
  struct {TreeMapNode *it; u32 i} e = {(first), 0}; (e.it != NULL); e.it = e.it->next, e.i++;

typedef struct Node Node;
struct Node
{
  Node *parent, *left, *right;
  u32 val;
};

INTERNAL void
create_tree(MemArena *arena, Node *p, u32 height)
{
  if (height <= 0) return;

  Node *left = MEM_ARENA_PUSH_STRUCT_ZERO(arena, Node);
  left->val = height;
  p->left = left;
  create_tree(arena, left, height - 1);
  
  Node *right = MEM_ARENA_PUSH_STRUCT_ZERO(arena, Node);
  right->val = height;
  p->right = right;
  create_tree(arena, right, height - 1);
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
  BP();
  String8 node_value = str8_fmt(arena, "%d\n", n->val);
  str8_list_push(arena, dump, node_value); 

  dump_from_node(arena, dump, n->left, indent + 1, indent_str);

#undef PRINT_INDENT
}

INTERNAL void
print_node(MemArena *arena, Node *n, u32 indent)
{
  MEM_ARENA_TEMP_BLOCK(arena, scratch_arena)
  {
    String8List dump = ZERO_STRUCT;

    dump_from_node(scratch_arena.arena, &dump, n, 0, str8_lit(" "));

    String8Join join = ZERO_STRUCT;
    String8 dump_str = str8_list_join(scratch_arena.arena, dump, &join);

    printf("%.*s", str8_varg(dump_str));
  }
}


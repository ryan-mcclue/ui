// SPDX-License-Identifier: zlib-acknowledgement

// IMPORTANT(Ryan): lists + hash maps!
// IMPORTANT(Ryan): f(offset field_off) -> MEMBER_FROM_OFF(struct, field_off)

// TODO(Ryan): include tree operations
//   DFS and BFS
//   creation
//   printing

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

INTERNAL void
print_tree(MemArena *arena, Node *root)
{
  MEM_ARENA_TEMP_BLOCK(arena, scratch_arena)
  {
    String8List list = ZERO_STRUCT;
    dump_from_node(scratch.arena, &list, root, 0, S8Lit(" "));
    // can now print to file etc.
    String8 string = S8ListJoin(scratch.arena, list, 0);
    printf("%.*s", s8_varg(string));
  }
}

INTERNAL void
dump_from_node(MemArena *arena, String8List *out, Node *node, u32 indent, String8 indent_string)
{
#define PRINT_INDENT(_i) \
  for (u32 i = 0; i < (_i); i += 1) { str8_list_push(arena, out, indent_string); }

  PRINT_INDENT(indent);
  String8 node_value = str8_fmt(arena, "%d", node_value);
  str8_list_push(arena, out, node_value); 
  PRINT_INDENT(indent);
  str8_list_push(arena, out, s8_lit("\n")); 

  for (Node *child = node->first; child != NULL; child = child->next)
  {
    PRINT_INDENT(indent);
    String8 node_value = str8_fmt(arena, "%d", node_value);
    str8_list_push(arena, out, node_value); 
    u32 indent_value = (indent + 1 + node_value.size);
  }

  str8_list_push(arena, out, s8_lit("L-"));
  dump_from_node(node->left); 

#undef PRINT_INDENT
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

// for (Node *n = root; n != NULL; n = dfs_get_next(n))

//Node *root = add_node(NULL);
//Node *n = root;
//for (u32 i = 0; i < pow(2, depth) - 1; i += 1)
//{
//  n = add_node(n); 
//}

INTERNAL Node *
add_node(Node *p)
{
  Node *added = MEM_ARENA_PUSH_STRUCT_ZERO(arena, Node);
  if (node == NULL) return added;

  if (node->left != NULL) node->left = added;
  else if (node->right != NULL) node->right = added;
  else
  {
    node->left->left = added;
    added->parent = node->left;
  }

  return added;
}

INTERNAL Node *
create_tree(MemArena *arena, u32 depth)
{
  if (depth == 0) return NULL;

  Node *result = create_node(arena, );

  result->value = depth;
  result->left = create_tree(arena, depth - 1);
  result->right = create_tree(arena, depth - 1);

  return result;
}


typedef struct Node Node;
struct Node
{
  Node *parent, *first_child, *last_child, *next, *prev;
};

 // hash links, i.e. point to next node in hash map
 UI_Box *hash_next;
 UI_Box *hash_prev;

INTERNAL Node *
create_node(Node *parent)
{
  Node *node = PUSH_MEM();
  DLL_INSERT(node_hash_first, node_hash_last, node);

  DLL_INSERT(parent->first, parent->last, node);
  node->parent = ;
}



root_function UI_Box *UI_BoxMakeFromKey(UI_BoxFlags flags, UI_Key key);
root_function UI_Box *UI_BoxMake(UI_BoxFlags flags, String8 string);
root_function UI_Box *UI_BoxMakeF(UI_BoxFlags flags, char *fmt, ...);


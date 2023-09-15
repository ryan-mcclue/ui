// SPDX-License-Identifier: zlib-acknowledgement
#pragma once

typedef struct Plug Plug;
struct Plug
{
  u32 version; // allows for migration? i.e. at runtime convert to new struct? manual would be to recompile?
  // migration requires versioning
  // so, we can change 'schema' of struct when we reload
  // in essence, run-time update of data version
  
  // in other words, put version on external data?
  /*
   if (state->version == 1) {
    // Handle Version 1 (e.g., migrate data or update fields)
    // Create a new StateV2 instance and copy values from the old state
    struct StateV2 newState;
    newState.version = 2;
    newState.val = state->val;
    newState.newField = 0; // Initialize new field

    // Update the pointer to the new structure
    // IMPORTANT(Ryan): Would require defining padding in struct to account for additions
    state = (struct State*)&newState;
    
    // Now, state points to the updated structure (Version 2)
}*
   */
};

typedef void (plug_init)(void)
// could add extra functions to say indicate plugin about to be reloaded 

// IMPORTANT(Ryan): Use x-macros when want list of source code, e.g. declarations etc.
// $(cpp) is c pre processor
#define LIST_OF_PLUGS \
  X(plug_init, void, Plug*) \
  X(plug_update, void, Plug*) \

#define X(name, return_type, ...) \
	typedef return_type (* name##_t)(__VA_ARGS__);
LIST_OF_PLUGS
#undef X

// else where
#define X(name) name##_t = NULL;
LIST_OF_PLUGS
#undef X

#define X(name, return_type, ...) \
  name = dlsym(libplug, STRINGIFY(name))
LIST_OF_PLUGS
#undef X

// in release build, statically link, e.g. -lplug, or could use * typedef trick

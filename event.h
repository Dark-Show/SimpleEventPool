// Title: Simple Event Pool
// Author: Gregory Michalik
// 
// Usage:
//   Modify struct pool_data with any needed event data.
//
//   Create/Init pool:
//     s_event_pool ev_pool;      // Define event pool
//     event_pool_init(&ev_pool); // Init event pool for usage
// 
//   Insert event for owner:
//     s_event ev;       // Event to be inserted
//     int ret;          // Return code
//     ev.data.xx = xx;  // Append additional data
//     ret = event_insert(&ev_pool, EVENT_OWNER_DEF, ev); // Insert event
//     if (ret == EVENT_ERROR) { // Event pool full
//       ...
//     }
//
//   Get latest event for owner (Removes from pool):
//     s_event ev; // Empty event
//     ev = event_get(&ev_pool, EVENT_OWNER_DEF); // Get latest event / removes from pool
//     if (ev.id != EVENT_FREE_ID) { // Have Events
//       ...
//     }

#define EVENT_OWNERS       1    // Def owner only
#define EVENT_OWNER_LIMIT  1024 // Max events per owner

#define EVENT_POOL_MAX     (EVENT_OWNER_LIMIT*EVENT_OWNERS) // Max events in pool
#define EVENT_ERROR        -1   // Error return code
#define EVENT_FREE_ID      -1   // ID marker for unused event
#define EVENT_OWNER_DEF    0    // Def counts as an owner
#define EVENT_OWNER_1      1
#define EVENT_OWNER_2      2
#define EVENT_OWNER_3      3
#define EVENT_OWNER_4      4
#define EVENT_OWNER_5      5
#define EVENT_OWNER_6      6
#define EVENT_OWNER_7      7
#define EVENT_OWNER_8      8

struct s_event_data { // EDIT ME
	int command;
	int value;
};
typedef struct s_event_data s_event_data;

struct s_event {
	int          id;    // Required, leave untouched.
	int          owner; // Required, read only if accessing raw.
	s_event_data data;  // Passed event data
};
typedef struct s_event s_event;

struct s_event_pool {
	s_event events[EVENT_POOL_MAX]; // Events
	int     count[EVENT_OWNERS];    // Owner event counts / read only if accessing raw.
	int     current[EVENT_OWNERS];  // Owner current variable, leave untouched.
};
typedef struct s_event_pool s_event_pool;

void event_pool_init(s_event_pool *ep);                   // Init event pool
int event_insert(s_event_pool *ep, int owner, s_event e); // Insert event for owner into pool
s_event event_get(s_event_pool *ep, int owner);           // Get event for owner in order
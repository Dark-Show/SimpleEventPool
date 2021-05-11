// Title: Simple Event Pool
// Author: Gregory Michalik
// Version: 0.2
// Date: 2020/03/17
// 
// Usage:
//   Modify struct sep_event to add any needed event data.
//
//   Create/Init pool:
//	   sep_pool *pool = NULL; // Define event pool
//	   evid = event_register_owner(&pool); // Add first owner / init
// 
//   Insert event for owner:
//     sep_event ev;	 // Event data to be inserted
//     int ret;			 // Return code
//     ev.command = 0;	 // Append additional data
//	   ev.value = '\0';
//     ret = event_send(pool, evid, evid, ev); // Insert event
//     if (!ret) { // Event pool full
//	       if (ret == SEP_ERROR_LIMIT) {
//			   ...
//		   } else if ( ret == SEP_ERROR_EXIST) {
//			   ...
//		   } else {
//			   ...Unexpected
//		   }
//     }
//
//   Get latest event for owner (Removes from pool):
//     sep_event ev; // Empty event
//     ev = event_get(&ev_pool, evid); // Get latest event / removes from pool
//	   if (ev.owner == SEP_ERROR) {
//       if (ev.owner == SEP_ERROR_EXIST) {
//         ...
//       } else if (ev.owner == SEP_ERROR_MEM) { 

#define SEP_LIMIT_OWNER     32      // Max owners
#define SEP_LIMIT_EVENT     1000    // Max events per owner
#define SEP_ERROR           -1      // Generic error
#define SEP_ERROR_LIMIT     -2      // Limit error
#define SEP_ERROR_EXIST     -3      // ID exist error
#define SEP_ERROR_MEM       -4      // ID exist error

struct sep_event {
    int     id;	        // Internal Event ID
    int     owner;      // Owner ID
    int     from;       // From Owner ID
    int     command;    // OUR DATA
	char    *value;     // OUR DATA
};
typedef struct sep_event sep_event;

struct sep_pool {
    sep_event **events;	// Events
    int     es;         // Event Space
    int     oc;         // Owner Count
    int     ec;         // Event Count
    int     oec[SEP_LIMIT_OWNER];	// Owner event counts
    int     c[SEP_LIMIT_OWNER];		// Owner current event id
    int     on[SEP_LIMIT_OWNER];	// Owner next id tracking
};
typedef struct sep_pool sep_pool;

int event_register_owner(sep_pool **pool); // Add event owner to pool, returns owner id
int event_deregister_owner(sep_pool *pool, int owner); // Remove owner from pool, returns 1 on success
int event_send(sep_pool *pool, int to, int from, sep_event event); // Send event to owner id
int event_owner_exist(sep_pool *pool,  int owner); // Check to see if an owner id exists
sep_event event_get(sep_pool *pool, int owner); // Get next event, Returns event.owner == SEP_ERROR on error, .id (SEP_ERROR_LIMIT | SEP_ERROR_EXIST)
void event_free(sep_pool *pool); // Free event pool memory

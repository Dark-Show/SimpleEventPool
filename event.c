#include <stdlib.h>
#include <string.h>
#include "event.h"

const int event_locate_free(sep_pool *pool);
const int event_locate(sep_pool *ep, int owner);

// Free event memory
void event_free(sep_pool *pool) {
    int loc;
    // Free all events
    for (loc = 0; loc < pool->es; loc++) {
        free(pool->events[loc]);
    }
    // Free pool
    free(pool->events);
    free(pool);
}

int event_deregister_owner(sep_pool *pool, int owner) {
    int loc;

    if(!event_owner_exist(pool, owner)) {
        return(SEP_ERROR_EXIST);
    }

    pool->oec[owner] = -1; // Mark free
    pool->on[owner] = -1;  // Mark free
    pool->c[owner] = -1;
    pool->oc--; // Subtract owner from count

    // Remove all events for owner
    for (loc = 0; loc < pool->es; loc++) {
        if (pool->events[loc]->owner == owner){
            pool->events[loc]->owner = -1; // Mark free
        }
    }
    return(1);
}

int event_register_owner(sep_pool **pool) {
    sep_pool *ep = *pool;
    int i;

    // Init check
    if (!ep) {
        // Pool
        ep = malloc(sizeof(*ep));
        *pool = ep;
        ep->ec = 0; // Set event count
        ep->es = 1; // Set event size
        ep->oc = 0; // Set owner count
        // Events
        ep->events = malloc(sizeof(*ep->events)); // Allocate 1 event pointer
        if (!ep->events) {
            return(SEP_ERROR_MEM); // Error: Out of mem
        }
        ep->events[0] = malloc(sizeof(*ep->events[0])); // Allocate 1 event
        if (!ep->events[0]) {
            return(SEP_ERROR_MEM); // Error: Out of mem
        }
        ep->events[0]->owner = -1; // Mark event space
        ep->events[0]->id = 0;
        ep->events[0]->from = 0;
        // Owners
        for (i = 1; i < SEP_LIMIT_OWNER; i++) {
            ep->oec[i] = -1; // Disable owner event count
            ep->c[i] = -1;   // Disable owner current event
            ep->on[i] = -1;  // Disable next id
        }
    }
    
    if (ep->oc >= SEP_LIMIT_OWNER){
        return(SEP_ERROR_LIMIT); // Error: Owner limit reached
    }

    // Register Owner
    ep->oec[ep->oc] = 0; // Reset owner event count
    ep->c[ep->oc] = 0;   // Resent current id tracker
    ep->on[ep->oc] = 0;  // Reset next id tracker
    ep->oc++;            // Increment owner count
    return ep->oc - 1;
}

// Find highest event id, starting from start, lower than max
const int event_locate_free(sep_pool *pool) {
    int loc;
    for(loc = 0; loc < pool->es; loc++) { // Find empty space in pool
        if (pool->events[loc]->owner == -1) {
            return loc; // Return found location
        }
    }
    return -1; // Return Error
}

// Send event to id, with sender and data
int event_send(sep_pool *pool, int to, int from, sep_event event) {
    int loc;

    // Error check
    if(!event_owner_exist(pool, to) || !event_owner_exist(pool, from)) {
        return(SEP_ERROR_EXIST); // Error: Owner(s) does not exist
    }
    if (pool->oec[to] >= SEP_LIMIT_EVENT){
        return(SEP_ERROR_LIMIT); // Error: Pool limit reached
    }

    pool->ec++;
    pool->oec[to]++; // Increment owner event count

    // Pool space check
    if (pool->ec >= pool->es) {
        pool->es *= 2; // Double pool size (so we dont enter this often)
        pool->events = realloc(pool->events, sizeof(*pool->events) * pool->es); // Reallocate size of event pointer pool
        if (!pool->events) {
            pool->ec--;
            pool->oec[to]--;
            return(SEP_ERROR_MEM); // Error: Out of mem
        }
        for (loc = pool->es - pool->ec; loc < pool->es; loc++) { // Allocate new event memory
            sep_event *ev = malloc(sizeof(*ev));
            if (!ev) {
                pool->ec--;
                pool->oec[to]--;
                return(SEP_ERROR_MEM); // Error: Out of mem
            }
            ev->owner = -1; // Mark Free
            ev->id = 0;
            ev->from = 0;
            pool->events[loc] = ev;
        }
    }

    if(pool->oec[to] > SEP_LIMIT_EVENT) {
        pool->oec[to] = 0; // Wrap
    }

    loc = event_locate_free(pool); // Find empty space

    if (loc >= 0) {
        event.id = pool->on[to];  // Set next event id
        pool->on[to]++;
        if(pool->on[to] >= SEP_LIMIT_EVENT) {
            pool->on[to] = 0; // Wrap
        }
        
        event.owner = to;     // Set to
        event.from = from;    // Set from
        if(!memcpy(pool->events[loc], &event, sizeof(sep_event))) { // Copy event into pool
            return(SEP_ERROR_MEM); // Error: Out of mem
        }
        return 1; // Success
    }
    return SEP_ERROR; // Error
}

int event_owner_exist(sep_pool *pool, int owner){
    if (owner < 0 || owner > SEP_LIMIT_OWNER || pool->oec[owner] == -1) { // Look at owner count
        return 0;
    }
    return 1;
}

// Locate pool position of next event
const int event_locate(sep_pool *ep, int owner) {
    int i, f = 0, ff = 0, loc = 0;
    int fid = SEP_LIMIT_EVENT + 1, ffid = SEP_LIMIT_EVENT + 1;
    
    if (ep->oec[owner] <= 0) { // Pool check
        return(-1); // Error: No events
    }
    
    // Look for lowest event id higher than or equal to current
    for (i = 0; i < ep->es; i++) { // Search entire buffer
        if (ep->events[i]->owner == owner) { // Check if we own and id is lower than current
            if (ep->events[i]->id >= ep->c[owner] && fid > ep->events[i]->id) { // ID smaller than current event
                fid = ep->events[i]->id; // Store lower ID
                loc = i; // Store location
                f = 1;   // Found something 
            } else if (!f && ep->events[i]->id < ep->c[owner] && ffid > ep->events[i]->id) {
                ffid = ep->events[i]->id; // Store lower ID
                loc = i; // Store location
                ff = 1;  // Found something
            }
        }
    }
    if(f || ff) {
        return loc; // Return location of event
    }
    return(-1); // Error: No event found
}

// Get next pooled event for owner
sep_event event_get(sep_pool *pool, int owner) {
    sep_event e;
    int loc;
    
    loc = event_locate(pool, owner); // Find location of next event
    
    if(loc == -1) {
        e.owner = SEP_ERROR;    // Set Error
        e.id = SEP_ERROR_EXIST; // Error: ID not found
        return(e);
    }

    pool->c[owner]++; // Increment current
    if(pool->c[owner] >= SEP_LIMIT_EVENT) { // Check Limit
        pool->c[owner] = 0; // Wrap
    }

    if(!memcpy(&e, pool->events[loc], sizeof(sep_event))) { // Copy event out of pool
        e.owner = SEP_ERROR;  // Set Error
        e.id = SEP_ERROR_MEM; // Error: Out of mem
        return(e);
    }
    pool->events[loc]->owner = -1; // Set Free
     
    pool->oec[owner]--; // Decrement Owner Event Count
    pool->ec--; // Decrement Event Count
    return(e);
}
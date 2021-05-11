#include "event.h"

// Initilize event pool
void event_pool_init(s_event_pool *ep) {
	s_event_pool p;
	int i;
	p = *ep;
	
	// Init events as free
	for (i = 0; i < EVENT_POOL_MAX; i++) {
		p.events[i].id = EVENT_FREE_ID; // Free event
	}
	
	// Init each owner
	for (i = 0; i < EVENT_OWNERS; i++){
		p.count[i] = 0;
		p.current[i] = 0;
	}
	*ep = p;
}

// Find free event location
int event_locate_free(s_event_pool ep) {
	int i;
	
	// Look for free event space
	for(i = 0; i < EVENT_POOL_MAX; i++){
		if(ep.events[i].id == EVENT_FREE_ID){ // Free event
			return(i); // Return location
		}
	}
	return(EVENT_ERROR); // Error: Pool full
}

// Find event location for owner using event id
int event_locate_id(s_event_pool ep, int owner, int id) {
	int i;
	
	// Look for event location
	for(i = 0; i < EVENT_POOL_MAX; i++){
		if(ep.events[i].owner == owner && // Match owner
		   ep.events[i].id == id          // Match ID
		  ){ // match id, owner
			return(i); // Return location
		}
	}
	return(EVENT_ERROR); // Error: Non-Existant
}

// Find highest event id, starting from start, lower than max
int event_id_high(s_event_pool ep, int owner, int low, int max) {
	int i;
	int id = EVENT_FREE_ID; // Default free
	
	// Look for highest event id, higher than or equal to low, or smaller than max
	for(i = 0; i < EVENT_POOL_MAX; i++){
		if(ep.events[i].id != EVENT_FREE_ID && ep.events[i].owner == owner) { // Free / Owner check
			if (ep.events[i].id > id &&             // ID larger than current
		   		ep.events[i].id >= low &&            // ID larger than low
		   		ep.events[i].id < max               // ID smaller than max
			){
				id = ep.events[i].id;
			}
		}
	}
	return(id);
}

// Insert event for owner
int event_insert(s_event_pool *ep, int owner, s_event e) {
	s_event_pool p;
	int loc;
	int id = 0;
	
	p = *ep;
	
	// Limit check
	if (p.count[owner] == EVENT_OWNER_LIMIT) {
		return(EVENT_ERROR); // Error: Pool limit reached
	}
	
	loc = event_locate_free(p); // Find free event slot
	if (loc == EVENT_ERROR){
		return(EVENT_ERROR); // Error: Pool full
	}

	// Find next id for owner
	id = event_id_high(p, owner, p.current[owner], EVENT_OWNER_LIMIT); // Get highest id from owner starting from current until max
	if (id + 1 < EVENT_OWNER_LIMIT){ // Not at limit 
		id++;
	}else{ // At limit (time to loop)
		id = event_id_high(p, owner, 0, p.current[owner]); // Get higest ID starting at 0 until p.current 
		id++; // Next ID from highest
	}
	
	e.id = id;
	
	// Insert event into pool
	p.events[loc] = e;
	p.count[owner]++;
	
	*ep = p;
	
	return(id);
}

// Remove event from owner using event id
void event_remove_location(s_event_pool *ep, int loc) {
	s_event_pool p;
	p = *ep;
	
	// This check helps if we enter an unknown error state.
	// Not removing free events from the pool count
	// will make the pool fill until we report -1 on insert,
	// rather than entering a race condition and thrashing memory.
	// if it is needed is a question.
	
	if (p.events[loc].id != EVENT_FREE_ID) { // Unstable Check
		p.events[loc].id = EVENT_FREE_ID; // Mark Free
		p.count[p.events[loc].owner]--;	  // Subtract from event counter
		*ep = p;						  // Store
	}
}

// Find lowest event id for owner, starting from
int event_id_low(s_event_pool ep, int owner, int low) {
	int i;
	int id = EVENT_FREE_ID; // Default Free
	
	// Look for lowest event id higher than low
	for(i = 0; i < EVENT_POOL_MAX; i++) {
		// Not free, Match owner, Higher than or equal to low
		if (ep.events[i].id != EVENT_FREE_ID && ep.events[i].owner == owner && ep.events[i].id >= low) {
			if(ep.events[i].id < id || id == EVENT_FREE_ID) { // ID smaller than current
				id = ep.events[i].id; // Update lowest ID
			}
		}
	}
	return(id); // Return lowest ID
}

// Get next pooled event for owner
s_event event_get(s_event_pool *ep, int owner) {
	s_event_pool p;
	s_event e;
	int id;
	int loc;
	p = *ep;
	
	e.id = EVENT_FREE_ID; // Define free event
	
	if (p.count[owner] == 0) { // Pool check
		return(e); // Error: No event
	}
	
	// find next id for owner from current
	id = event_id_low(p, owner, p.current[owner]); // Find lowest ID
	
	if (id == EVENT_FREE_ID) { // No ID Found
		p.current[owner] = 0; // Loop Event Pool
		id = event_id_low(p, owner, p.current[owner]); // Find lowest ID
	}else{ // ID Found
		p.current[owner]++; // Increment current
	}

	loc = event_locate_id(p, owner, id); // Locate ID in events
	if (loc == EVENT_ERROR) { // Error: Non-existant
		return(e); // Return free event
	}
	e = p.events[loc]; // Store located event
	
	if(p.current[owner] >= EVENT_OWNER_LIMIT) { // Reached pool limit
		p.current[owner] = 0; // Loop
	}
	
	*ep = p;
	event_remove_location(ep, loc);
	return(e);
}
#ifndef __ELEVATOR_ALG_H__
#define __ELEVATOR_ALG_H__

#include <stdbool.h>

#define	FALSE	0
#define	TRUE	1
#define MAX_LIFTS 3
#define MAX_REQUESTS 3
#define MAX_REQUESTS_PROC 9 
#define MAX_INTERNAL_INPUTS 30

// enumerator for showing the elevator current status
typedef enum status {
	ELV_SUCCESS = 0,
	ELV_START_ERROR = -1,
	ELV_UNKNOWN_ERROR = -2
} ELV_STATUS;

// enumerator for showing the elevator current status
typedef enum direction {
	DIR_IDLE = 0,
	DIR_UP,
	DIR_DOWN
} ELV_DIRECTION;

// a service request primitive containing current user level & 
typedef struct __service_req {
	int level;
	ELV_DIRECTION dir;
} SERVICE_REQ;

// elevator structure containing elevator status and the requests its servicing
typedef struct __elevator {
	int id;
	SERVICE_REQ current_status;
	SERVICE_REQ service[3];
	int serivce_cntr;
	bool user_destination[MAX_INTERNAL_INPUTS];
} ELEVATOR;

// structue which takes the external user input
typedef SERVICE_REQ EXT_INPUT;

ELV_STATUS dispatch_idle_elv();

// dummy prototypes
ELV_STATUS start_elv(int elv_id);
void stopTimer(void);
void sleep(int how_long);

// User input from outside the elevator is queued and we get it by using the queue functions
void pop_from_queue(EXT_INPUT *in);
void push_to_queue(EXT_INPUT in);
#endif


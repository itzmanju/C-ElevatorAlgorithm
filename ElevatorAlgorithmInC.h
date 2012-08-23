#ifndef __ELEVATOR_ALG_H__
#define __ELEVATOR_ALG_H__

// enumerator for showing the elevator current status
typedef enum direction {
	DIR_IDLE = 0,
	DIR_UP,
	DIR_DOWN
} ENUM_LIFT_DIRECTION;

// structue which takes the external user input
typedef struct __ext_input {
	int cur_user_level;
	ENUM_LIFT_DIRECTION user_input_direction;
} EXT_INPUT;

// a service request primitive containing current user level & 
typedef struct __service_req {
	int level;
	ENUM_LIFT_DIRECTION dir;
} SERVICE_REQ;

// elevator structure containing elevator status and the requests its servicing
typedef struct __elevator {
	SERVICE_REQ current_status;
	SERVICE_REQ service1;
	SERVICE_REQ service2;
	SERVICE_REQ service3;
	int serivce_cntr;
} ELEVATOR;

void Set_Idle_Lift_Levels();
void Decision_Algrthm_Func();

void Elevator1_Processing_Func();
void Elevator2_Processing_Func();
void Elevator3_Processing_Func();

#endif


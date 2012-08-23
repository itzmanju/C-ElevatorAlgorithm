#include <stdio.h>
#include <time.h>
#include <string.h>
#include "ElevatorAlgorithmInC.h"

ELEVATOR elevators[3];
bool sleep_flag = FALSE;

void Set_Idle_Lift_Levels();
void decision_algo(EXT_INPUT *current_proc_input);

void receive_user_internal_trigger(int i, int dest_level);
void elevator_input_proc_func(int i);

void swap_func(int *num1, int *num2);
void set_service_level_descending(int i);
void set_service_level_ascending(int i);

/******************************************************************************************************/
// this function dispatches an elevator to the specified level
// the MCU inside the lift will figure out which direction to take
ELV_STATUS dispatch_elv (int elv_id, int elv_level)
{
	// atomic operation start
	
	elevators[elv_id].current_status.level = elv_level;
	
	// atomic operation end
	if (!start_elv(elv_id))
	{
		return ELV_SUCCESS;
	}
	else
	{
		printf("Elevator start error\r\n");
		return ELV_START_ERROR;
	}
}

/******************************************************************************************************/
// to find the closest elevator to the specified level
// the dnu1 & dnu2 parameters specify which all elevators we can take into account while finding the closes elevator
// if an elevator is currently servicing 3 requests, we do not want to consider it which figuring out the closest elevator
int find_close_elevator(int level, int dnu1, int dnu2)
{
	int i, close_elevator, diff;

	// Finding lift nearest to level 5
	for (i = 0; i < MAX_LIFTS; i++)
	{
		if (dnu1 == i) continue;
		if (dnu2 == i) continue;
		close_elevator = i;
		diff = elevators[i].current_status.level - level;
		diff = (diff < 0) ? diff*(-1) : diff;
		if (diff < (elevators[close_elevator].current_status.level - level))
		{
			close_elevator = i;
		}
	}
	return close_elevator;
}

/******************************************************************************************************/
// function to dispatch elevators to their idle levels when there is no activity for x time units
ELV_STATUS dispatch_idle_elv()
{
	int hour = 0, min = 0, level1_close_elevator, level2_close_elevator, level3_close_elevator;
	time_t tm;
	struct tm *ltime;	
	ELV_STATUS ret;
	time( &tm );
	
	ltime = localtime( &tm );
	ltime->tm_mon++;
	ltime->tm_year += 1900;
	
	hour = ltime->tm_hour;
	min = ltime->tm_min;

	printf( "  %d , %d \n\n", hour, min );
		
	// AM Peak
	if ((hour >=7) && (hour <= 9))
	{

		level1_close_elevator = find_close_elevator(5, -1, -1);
		level2_close_elevator = find_close_elevator(16, level1_close_elevator, -1);
		level3_close_elevator = find_close_elevator(23, level1_close_elevator, level2_close_elevator);

		elevators[0].current_status.level = 5;
		elevators[1].current_status.level = 16;
		elevators[2].current_status.level = 26;
	}
	// PM Peak
	else if ((hour >=17) && (hour <= 20))
	{
		level1_close_elevator = find_close_elevator(0, -1, -1);
		level2_close_elevator = find_close_elevator(10, level1_close_elevator, -1);
		level3_close_elevator = find_close_elevator(20, level1_close_elevator, level2_close_elevator);

		elevators[level1_close_elevator].current_status.level = 0;
		elevators[level2_close_elevator].current_status.level = 10;
		elevators[level3_close_elevator].current_status.level = 20;
	}
	// Normal timings
	else
	{
		level1_close_elevator = find_close_elevator(0, -1, -1);
		level2_close_elevator = find_close_elevator(14, level1_close_elevator, -1);
		level3_close_elevator = find_close_elevator(23, level1_close_elevator, level2_close_elevator);

		elevators[level1_close_elevator].current_status.level = 0;
		elevators[level2_close_elevator].current_status.level = 14;
		elevators[level3_close_elevator].current_status.level = 23;
	}
	
	// dispatch the elevators. incase of error, return to main loop
	if ((ret = dispatch_elv(0, elevators[0].current_status.level)) != ELV_SUCCESS) return ret;
	if ((ret = dispatch_elv(1, elevators[1].current_status.level)) != ELV_SUCCESS)return ret;
	if ((ret = dispatch_elv(2, elevators[2].current_status.level)) != ELV_SUCCESS)return ret;
	
}

/******************************************************************************************************/
// check which all lifts are servicing full requests and update their DNU status
int check_and_set_dnu(int *dnu1, int* dnu2)
{
	int i;
	for (i = 0; i < MAX_LIFTS; i++)
	{
		if (elevators[i].serivce_cntr >= 3)
		{
			if (*dnu1 < 0)
			{
				*dnu1 = i;
			}
			else if (*dnu2 < 0)
			{
				*dnu2 = i;
			}
			else return -1;
		}
	}
	return 0;
}

/******************************************************************************************************/
void swap_func(int *num1, int *num2)
{
	*num1+= *num2;
	*num2= *num1 - *num2;
	*num1 -= *num2; 
}

/******************************************************************************************************/
void set_service_level_descending(int i)
{
	// Sorting should be invoked with the corresponding SR only if it is set
	if((elevators[i].service[0].dir != DIR_IDLE) &&  (elevators[i].service[1].dir != DIR_IDLE) && (elevators[i].service[2].dir != DIR_IDLE))
	{

	 	if(elevators[i].service[0].level > elevators[i].service[1].level)			
		{
			if (elevators[i].service[0].level > elevators[i].service[2].level)	
			{
				;	// SR1 is the highest, no swap for SR1, for clarity sake
			}
			//	SR3 is highest so swap 1 & 3
			else		
			{
				swap_func(& elevators[i].service[0].level,& elevators[i].service[2].level);
				// swap 2 and 3 coz current SR3 is > SR2
				swap_func(& elevators[i].service[1].level,& elevators[i].service[2].level);
			}
		}
		else if(elevators[i].service[1].level > elevators[i].service[2].level)		
		{
			// SR2 is highest
			swap_func(& elevators[i].service[0].level,& elevators[i].service[1].level);
			if(elevators[i].service[1].level > elevators[i].service[2].level)	
			{	
				; // no swap required, for clarity sake
			} 
			else
			{
				swap_func(& elevators[i].service[1].level,& elevators[i].service[2].level);
			}
		}
		//	SR3 is highest. SR1 will be the lowest
		else 
		{
			swap_func(& elevators[i].service[0].level,& elevators[i].service[2].level);
		}
	}
	else if((elevators[i].service[0].dir != DIR_IDLE) &&  (elevators[i].service[1].dir != DIR_IDLE) && (elevators[i].service[2].dir == DIR_IDLE))
	{
	 	if(elevators[i].service[0].level > elevators[i].service[1].level)			
		{
			;	// SR1 is the highest, no swap for SR1, for clarity sake
		}
		else		
		{
			swap_func(& elevators[i].service[0].level,& elevators[i].service[1].level);

		}
	}
	else
	{
			;// only SR1 is active. No need to swap, for clarity sake
	}
}

/******************************************************************************************************/
void set_service_level_ascending(int i)
{
	// Sorting should be invoked with the corresponding SR only if it is set
	if((elevators[i].service[0].dir != DIR_IDLE) &&  (elevators[i].service[1].dir != DIR_IDLE) && (elevators[i].service[2].dir != DIR_IDLE))
	{ 	
		if(elevators[i].service[0].level < elevators[i].service[1].level)			
		{
			if (elevators[i].service[0].level < elevators[i].service[2].level)	
			{
				;	// SR1 is the lowest, no swap for SR1
			}
			//	SR3 is lowest so swap 1 & 3
			else		
			{
				swap_func(& elevators[i].service[0].level,& elevators[i].service[2].level);
				// swap 2 and 3 coz current SR3 is < SR2
				swap_func(& elevators[i].service[1].level,& elevators[i].service[2].level);
			}
		}
		else if(elevators[i].service[1].level < elevators[i].service[2].level)		
		{
			// SR2 is lowest
			swap_func(& elevators[i].service[0].level,& elevators[i].service[1].level);
			if(elevators[i].service[1].level < elevators[i].service[2].level)	
			{	
				; // no swap required, for clarity sake
			} 
			else
			{
				swap_func(& elevators[i].service[1].level,& elevators[i].service[2].level);
			}
		}
		else //	SR3 is lowest. SR1 will be the highest
		{
			swap_func(& elevators[i].service[0].level,& elevators[i].service[2].level);
		}
	}
	else if((elevators[i].service[0].dir != DIR_IDLE) &&  (elevators[i].service[1].dir != DIR_IDLE) && (elevators[i].service[2].dir == DIR_IDLE))
	{
	 	if(elevators[i].service[0].level < elevators[i].service[1].level)			
		{
			;	// SR1 is the highest, no swap for SR1, for clarity sake
		}
		else		
		{
			swap_func(& elevators[i].service[0].level,& elevators[i].service[1].level);

		}
	}
	else
	{
			;// only SR1 is active. No need to swap, for clarity sake
	}
}

/******************************************************************************************************/
void add_service_request(int elv_id, EXT_INPUT *current_proc_input)
{
	int i;
	for (i = 0; i < MAX_REQUESTS; i++)
	{
		// find an unused service request
		if (elevators[elv_id].service[i].dir != DIR_IDLE) continue;
		else
		{
			// assign this service request
			elevators[elv_id].service[i].level = current_proc_input->level;
			elevators[elv_id].service[i].dir = current_proc_input->dir;
			
			// increment service counter
			elevators[elv_id].serivce_cntr++;
		
			if (elevators[i].service[i].dir == DIR_DOWN)
			{
				set_service_level_descending(i);		
			}
			else 
			{
				set_service_level_ascending(i);		
			}

		}
	}
	
}

/******************************************************************************************************/
// run through elevator list and dispatch if there is a pending service request
ELV_STATUS dispatch_pending_elv(void)
{
	ELV_STATUS ret;
	int i;
	for (i = 0; i < MAX_LIFTS; i++)
	{
		// if there is a service request, dispatch it
		if (elevators[i].serivce_cntr > 0)
		{
			// return with error code in case of failure
			if ((ret = dispatch_elv(i, elevators[i].current_status.level)) != ELV_SUCCESS) return ret;
		}
	}
}

/******************************************************************************************************/
// decides which lifts to service which all requests
void decision_algo(EXT_INPUT *current_proc_input)
{
	int i, dnu1 = -1, dnu2 = -1, close_elv;

	// check for lifts with full service requests
	if (!check_and_set_dnu(&dnu1, &dnu2))
	{
		// wait till the next cycle
		printf("All elevators busy\n");
		return;
	}

	// find a moving elevator which can service the current request
	// if no elevator is moving or if a moving elevator is not suitable, use an IDLE elevator
	for (i = 0; i < MAX_LIFTS; i++)
	{
	
	// find elevator closest to this requested floor
	close_elv = find_close_elevator(current_proc_input->level, dnu1, dnu2);
	
	// find an elevator intending to serve in the same direction as the current_proc_input
	if (elevators[close_elv].service[0].dir == current_proc_input->dir)
	{
			// if user input is UP, select this elevator only if
			// the current level of this elevator is less than current user level
			// and the destination is greater than or equal to user level
			if (current_proc_input->dir == DIR_UP)
			{
				// we are not considering '<='
				// a moving up elevator has its uppermost level in SR3 
				if (elevators[i].current_status.level < current_proc_input->level && elevators[i].service[2].level >= current_proc_input->level)
				{
					// add this request service to this lift
					add_service_request(elevators[close_elv].id, current_proc_input);
					break;
				}
			}
			// if user input is DOWN, select this elevator only if
			// the current level of this elevator is greater than current user level
			// and the destination is smaller (in number) than or equal to user level
			else if (current_proc_input->dir == DIR_DOWN)
			{
				if (elevators[close_elv].current_status.level > current_proc_input->level && elevators[i].service[0].level <= current_proc_input->level)
				{
					// add this request service to this lift
					add_service_request(elevators[close_elv].id, current_proc_input);
					break;
				}
			}
		}
		// If no elevators moving in same direction is recevied, we should check for idle elevators to assign request
		else if (elevators[close_elv].current_status.dir == DIR_IDLE)
		{
			if ((elevators[close_elv].service[0].dir == DIR_IDLE) ||  (elevators[close_elv].service[0].dir == current_proc_input->dir))
			{
				add_service_request(elevators[close_elv].id, current_proc_input);
				break;
			}
		}
	}

}

/******************************************************************************************************/
void elv_init()
{
	int i,j;

	for (i = 0; i < MAX_LIFTS; i++)
	{
		memset(&elevators[i], 0x00, sizeof(ELEVATOR));
		// set elevator ids
		elevators[i].id = 0;

		// set all elevator initial levels to 0
		if (elevators[i].current_status.level != 0)	
		{
			elevators[i].current_status.level = 0;
		}

		for(j = 0; j < MAX_INTERNAL_INPUTS; j++)
		{	
			elevators[i].user_destination[j] = FALSE;
		}
	}

	// If the elevators were not already in level 0, then they should go down
	// TODO: Error check
	dispatch_elv(0, 0);
	dispatch_elv(1, 0);
	dispatch_elv(2, 0);
}

/******************************************************************************************************/
int main (void)
{
	int i;
	int x_time_units = 600; //in seconds
	elv_init();
	EXT_INPUT current_proc_input;

	while(1)
	{
		// we reset the elevators to idle position if there was no input for the last x time units
		// if an event occured in between, we do not reset the elevators to idle condition
		if (!sleep_flag)
		{
			dispatch_idle_elv();
		}
		
		// main decision making part
		for (i = 0; i < MAX_REQUESTS_PROC; i++)
		{
			pop_from_queue(&current_proc_input);
			if (&current_proc_input != NULL)
			{
				decision_algo(&current_proc_input);
			}
			else
			{
				break;
			}
		}
		
		// decision complete, check for elevators with pending service requests and dispatch them
		dispatch_pending_elv();
		
		sleep_flag = TRUE;
		
		//Starts a timer and sleeps till either and external event occurs or sleep timer expires.
		sleep(x_time_units);
	}

	return 0;
}

/******************************************************************************************************/
//should be invoked as a separate thread after the dispatch function

void elevator_input_proc_func(int i)
{
	int j;

	// process until all inputs reach 0
	for(j = 0; j < MAX_INTERNAL_INPUTS; j++)
	{	
		if(elevators[i].user_destination[j] == TRUE);
		{
			// invoke control function to open door in the j-th level;			
		}
	}
}

/******************************************************************************************************/
//interrupts and external calls

// interrupt happens when there is an external event, like a user calling a lift to a floor
// we add this request to queue and exits this function
void event_interrupt(int level, int dir)
{
	EXT_INPUT in;
	
	in.level = level;
	in.dir = dir;
	
	// adds the request to queue
	push_to_queue(in);
}

/******************************************************************************************************/
// get invoked at timer expiry
void timer_expiry()
{
	sleep_flag = FALSE;
	// this call will signal to sleep function and restarts the main while loop
	stopTimer();
}

/******************************************************************************************************/
// when elevator finishes all its service
void elv_done_service (int elv_id)
{
	int temp_level;
	// TODO: atomic operations start
	
	// we have to store current level before clean up
	temp_level = elevators[elv_id].current_status.level;
	
	// clean up service requests & counters
	memset(&elevators[elv_id], 0x00, sizeof(ELEVATOR));
	
	elevators[elv_id].current_status.level = temp_level;
	// no need to set this as IDLE value is 0, but setting just for clarity
	elevators[elv_id].current_status.dir = DIR_IDLE;

	// TODO: atomic operations end
}

/******************************************************************************************************/
// when user selects a destination from inside the elevator
void receive_user_internal_trigger(int i, int dest_level)
{
	// dest_level should be in the range 0 <-> 29 
	if ((0 <= dest_level) && (dest_level <= MAX_INTERNAL_INPUTS))
	{
		elevators[i].user_destination[dest_level] = TRUE;
		// invoke MCU control function to set the trigger to open door in the dest_level			
	}
	else
	{
	
	}
}


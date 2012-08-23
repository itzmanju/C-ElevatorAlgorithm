#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include "ElevatorAlgorithmInC.h"
#include "queue.h"

#define MAX_LIFTS 3

ELEVATOR elevators[3];

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

void Set_Idle_Lift_Levels( )
{
	int hour = 0, min = 0, level1_close_elevator, level2_close_elevator, level3_close_elevator;
	time_t tm;
	struct tm *ltime;	
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
		elevators[0].current_status.level = 0;
		elevators[1].current_status.level = 10;
		elevators[2].current_status.level = 20;
	}
	// Normal timings
	else
	{
		elevators[0].current_status.level = 0;
		elevators[1].current_status.level = 14;
		elevators[2].current_status.level = 23;
	}

}


void Decision_Algrthm_Func()
{
	int i = 0;
	EXT_INPUT current_processing_input;

	current_processing_input = pop_queue();
}


int main (void)
{
	Init();

	while(1)
	{
		Set_Idle_Lift_Levels();
		Decision_Algrthm_Func();
		printf("\nPress any key to continue\n");
		getchar();
	}

	return 0;
}


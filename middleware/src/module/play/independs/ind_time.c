
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "app/Assertions.h"
#include "ind_tmr.h"

/*
    下面两个函数有linux内核函数修改而来，不能为了pclint而修改
 */

unsigned int ind_time_make(struct ind_time *tp)
{
	if (tp == NULL)
		ERR_OUT("ind_time is NULL\n");

	if (0 >= (int) (tp->mon -= 2)) {	/* 1..12 -> 11,12,1..10 */
		tp->mon += 12;		/* Puts Feb last since it has leap day */
		tp->year -= 1;
	}

	return (((
		(unsigned int) (tp->year/4 - tp->year/100 + tp->year/400 + 367*tp->mon/12 + tp->day) +
			tp->year*365 - 719499
			)*24 + tp->hour /* now have hours */
		)*60 + tp->min /* now have minutes */
	)*60 + tp->sec; /* finally seconds */
Err:
	return 0;
}

/* Nonzero if YEAR is a leap year (every 4 years,
   except every 100th isn't, and every 400th is).  */
# define __isleap(year)	\
  ((year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0))

static const unsigned short int __mon_yday[2][13] =
  {
    /* Normal years.  */
    { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
    /* Leap years.  */
    { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
  };

#define	SECS_PER_HOUR	(60 * 60)
#define	SECS_PER_DAY	(SECS_PER_HOUR * 24)

/* Compute the `struct tm' representation of *T,
	 offset OFFSET seconds east of UTC,
	 and store year, yday, mon, mday, wday, hour, min, sec into *TP.
	 Return nonzero if successful.	*/
int ind_time_local (unsigned int t, struct ind_time *tp)
{
	long int days, rem, y;
	const unsigned short int *ip;

	days = t / SECS_PER_DAY;
	rem = t % SECS_PER_DAY;

	tp->week = (days + 4) % 7;

	while (rem < 0)
		{
			rem += SECS_PER_DAY;
			--days;
		}
	while (rem >= SECS_PER_DAY)
		{
			rem -= SECS_PER_DAY;
			++days;
		}
	tp->hour = rem / SECS_PER_HOUR;
	rem %= SECS_PER_HOUR;
	tp->min = rem / 60;
	tp->sec = rem % 60;

	y = 1970;

#define DIV(a, b) ((a) / (b) - ((a) % (b) < 0))
#define LEAPS_THRU_END_OF(y) (DIV (y, 4) - DIV (y, 100) + DIV (y, 400))

	while (days < 0 || days >= (__isleap (y) ? 366 : 365))
		{
			/* Guess a corrected year, assuming 365 days per year.	*/
			long int yg = y + days / 365 - (days % 365 < 0);

			/* Adjust DAYS and Y to match the guessed year.	*/
			days -= ((yg - y) * 365
				 + LEAPS_THRU_END_OF (yg - 1)
				 - LEAPS_THRU_END_OF (y - 1));
			y = yg;
		}
	tp->year = y;
	ip = __mon_yday[__isleap(y)];
	for (y = 11; days < (long int) ip[y]; --y)
		continue;
	days -= ip[y];
	tp->mon = y + 1;
	tp->day = days + 1;

	return 1;
}


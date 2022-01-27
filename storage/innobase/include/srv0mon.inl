/*****************************************************************************

Copyright (c) 2010, 2013, Oracle and/or its affiliates. All Rights Reserved.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1335 USA

*****************************************************************************/

/*******************************************************************//**
@file include/srv0mon.ic
Server monitoring system

Created 1/20/2010	Jimmy Yang
************************************************************************/

/*************************************************************//**
This function is used to calculate the maximum counter value
since the start of monitor counter
@return max counter value since start. */
UNIV_INLINE
mon_type_t
srv_mon_calc_max_since_start(
/*=========================*/
	monitor_id_t	monitor)	/*!< in: monitor id */
{
	if (MONITOR_MAX_VALUE_START(monitor) == MAX_RESERVED) {

		/* MONITOR_MAX_VALUE_START has not yet been
		initialized, the max value since start is the
		max count in MONITOR_MAX_VALUE */
		MONITOR_MAX_VALUE_START(monitor) =
				MONITOR_MAX_VALUE(monitor);

	} else if (MONITOR_MAX_VALUE(monitor) != MAX_RESERVED
		   && (MONITOR_MAX_VALUE(monitor)
		       + MONITOR_VALUE_RESET(monitor)
		      > MONITOR_MAX_VALUE_START(monitor))) {

		/* If the max value since reset (as specified
		in MONITOR_MAX_VALUE) plus the reset value is
		larger than MONITOR_MAX_VALUE_START, reset
		MONITOR_MAX_VALUE_START to this new max value */
		MONITOR_MAX_VALUE_START(monitor) =
				MONITOR_MAX_VALUE(monitor)
				+ MONITOR_VALUE_RESET(monitor);
	}

	return(MONITOR_MAX_VALUE_START(monitor));
}

/*************************************************************//**
This function is used to calculate the minimum counter value
since the start of monitor counter
@return min counter value since start. */
UNIV_INLINE
mon_type_t
srv_mon_calc_min_since_start(
/*=========================*/
	monitor_id_t	monitor)	/*!< in: monitor id */
{
	if (MONITOR_MIN_VALUE_START(monitor) == MIN_RESERVED) {

		/* MONITOR_MIN_VALUE_START has not yet been
		initialized, the min value since start is the
		min count in MONITOR_MIN_VALUE */
		MONITOR_MIN_VALUE_START(monitor) =
				MONITOR_MIN_VALUE(monitor);

	} else if (MONITOR_MIN_VALUE(monitor) != MIN_RESERVED
		   && (MONITOR_MIN_VALUE(monitor)
		       + MONITOR_VALUE_RESET(monitor)
		       < MONITOR_MIN_VALUE_START(monitor))) {

		/* If the min value since reset (as specified
		in MONITOR_MIN_VALUE) plus the reset value is
		less than MONITOR_MIN_VALUE_START, reset
		MONITOR_MIN_VALUE_START to this new min value */
		MONITOR_MIN_VALUE_START(monitor) =
			MONITOR_MIN_VALUE(monitor)
                        + MONITOR_VALUE_RESET(monitor);
        }

	return(MONITOR_MIN_VALUE_START(monitor));
}

/*************************************************************//**
This function resets all values of a monitor counter */
UNIV_INLINE
void
srv_mon_reset_all(
/*==============*/
	monitor_id_t	monitor)	/*!< in: monitor id */
{
	/* Do not reset all counter values if monitor is still on. */
	if (MONITOR_IS_ON(monitor)) {
		fprintf(stderr, "InnoDB: Cannot reset all values for"
			" monitor counter %s while it is on. Please"
			" turn it off and retry.\n",
			srv_mon_get_name(monitor));
	} else {
		MONITOR_RESET_ALL(monitor);
	}
}
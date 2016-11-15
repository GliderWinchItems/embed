/*  proto.c */

/*
08-11-2011

General layout of operational POD program.

*/

void main (void)
{
	/* Basic initialization required for all modes */
	setup_clocks();
	initialize_ports();
	initialize_adc();

	/* Separate the type of reset: power on, or wake-up from (STANDBY or pushbutton) */
	if (RTC & BKP domain intact)
	{
		/* Determine type of wake-up */
		get_PA0_bit();		// Bit = 1 for Pushbutton wakeup (might coincide with RTC)
		normal_reset_run();
	}
	else
	{	
		cold_reset_run();	// Power off, then on, reset	
	}
	
	/* Never comes back to here */
}

void cold_reset_run(void)
{
	search_SD ();		// Get params, etc.
	normal_reset_run();	
	/* Never comes back to here */

}

void normal_reset_run(void)
{
	if (PA0_bit == 1)	// Was wake-up caused by the pushbutton?	
	{ // Here, yes.
		turn_on_RS232();	// Turn on RS232 and set timeout timer
		initialize_UART4();	// Ready for GPS rx
		initialize_USART1();	// Ready for PC tx/rx
		mode = active;		// Set active mode
	}
	switch (mode)
	{
		case active: 	
			/* The follow two take about 1/2 sec */
			get_SD_hw_ready();
			get_AD7799_ready();
			set_active_mode_timeout();	// 60 min(?)
			/* Escape from loop is via going into STANDBY */
			while(1==1)
			{
				BlipLED(ON);	// "Blip" led whilst reading tensions
				if (RS232_timer_ON == TRUE) // Each EOL resets timer
				{ // Each received line resets timer
					if (PC_rcv_line() == TRUE) do_pc_rcv();
					if (PC_gps_line() == TRUE) do_gps_rcv();				
				}
				if (active_timer_ON == TRUE) 
				{ 
					SD_card_write();	// Do whatever to write when a block is full
				}
				else
				{
					if (PC_command == READOUT)// Stay active as long as reading out
					{
						turn_AD7799_hw_off(); // Reduce power
						BlipLED(OFF);	// No longer reading tensions
					}
					else
					{
						mode = deepsleep;
						wait_for_SD_flag();
						gotostandby();	// 10 sec STANDBY
						/* Never comes back to here */
					}
				}
			}
		case deepsleep:	/* Objective: fix time and get back to STANDBY *fast* */
				while(time_temp_adj_flag == 0);	// Wait for RTC interrupt stuff to show done
				gotostandby();	// 10 sec STANDBY
				/* Never comes back to here */

	}
}

void do_gps_rcv(void)
{
	if (got_time_fix_line() == TRUE)
	{
		if (time_fix_counter < TIMEFIXLIMIT)
		{
			time_fix_counter += 1;
			SD_place_gps_time();
		}
	}

	return;
}

void do_pc_rcv(void)
{
	
}

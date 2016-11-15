/******************************************************************************
* File Name          : cmd_f.c
* Date First Issued  : 05/13/2014
* Board              : PC
* Description        : Display gps fix: lat lon ht
*******************************************************************************/
/*
This lifts & modifies some code from--
~/svn_sensor/sensor/co1_Olimex/trunk/p1_PC_monitor_can.c

*/

#include "cmd_f.h"


/******************************************************************************
 * void cmd_f_do_msg(struct CANRCVBUF* p);
 * @brief 	: Output current msg ID's ('n' command)
*******************************************************************************/
/*
*/

struct GPSFIX // Lifted from ../svn_sensor/sensor/co1_sensor_ublox/trunk/p1_gps_time_convert.h
{
	int	lat;	// Latitude  (+/-  540000000 (minutes * 100,000) minus = S, plus = N)
	int	lon;	// Longitude (+/- 1080000000 (minutes * 100,000) minus = E, plus = W)
	int	ht;	// Meters (+/- meters * 10)
	unsigned char fix;	// Fix type 0 = NF, 1 = G1, 2 = G3, 3 = G3
	unsigned char nsats;	// Number of satellites
};

static struct GPSFIX gfx;
static double dlat;
static double dlon;
static double dht;
static int linectr = 32;;

void cmd_f_do_msg(struct CANRCVBUF* p)
{
	int latd;	// Integer degrees: lat
	int lond;	// Integer degrees: lon
	double dlatd;	// Conversion to minutes lat
	double dlond;	// Conversion to minutes lon

//	if (p->id == (CAN_UNITID_CO_OLI | (CAN_DATAID_LAT_LONG << CAN_DATAID_SHIFT)))
	if (p->id == 0xb4000000)
	{
//printf ("%08X\n",(CAN_UNITID_CO_OLI | (CAN_DATAID_LAT_LONG << CAN_DATAID_SHIFT)));
		gfx.lat = p->cd.ui[0];
		gfx.lon = p->cd.ui[1];
	}

//	if (p->id == (CAN_UNITID_CO_OLI | (CAN_DATAID_HEIGHT   << CAN_DATAID_SHIFT)))
	if (p->id == 0xc4000000)
	{
//printf ("%08X\n",(CAN_UNITID_CO_OLI | (CAN_DATAID_HEIGHT   << CAN_DATAID_SHIFT)));
		gfx.ht = p->cd.ui[0]; gfx.fix = p->cd.uc[4]; gfx.nsats = p->cd.uc[5];
		linectr += 1;
		if (linectr > 32)
		{
			linectr = 0;
			printf ("  Lat(deg)    Lon(deg)     Height(m)  Lat(deg:min) Lon(deg:min) fix n-sats\n");
			
		}
		dlat = gfx.lat; dlon = gfx.lon; dht = gfx.ht;
		printf("%10.5f  %10.5f  %8.3f    -- ",dlat/60.0E5, dlon/60.0E5, dht/1E3);
		latd = dlat/60.0E5; dlatd = dlat/60.0E5; dlatd -= latd; dlatd *= 60.0;
		lond = dlon/60.0E5; dlond = dlon/60.0E5; dlond -= lond; dlond *= 60.0;
		printf("%d:%1.5f  %d:%1.5f  %2i %2i\n",latd, dlatd, lond, dlond, gfx.fix, gfx.nsats);
	}

	return;
}



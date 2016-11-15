/*
crc16TableGenerator.c  Compute/format table for tablelookup comutation of crc16
Companion to crc16.c
09/11/2007
*/

/* **************************************************************************************************** */
void compute_crc16_table(void)
{
        ushort temp, a;
	int i,j;
	union XX 
        {
		unsigned int zz;
		unsigned char c[2];
	}xx;

	sci0putstring("\nstatic unsigned int table[256] = {\n");
		
        for(i = 0; i < 256; i++)
	{
            temp = 0;
            a = (ushort)(i << 8);
            for(j = 0; j < 8; j++) 
            {
                if(((temp ^ a) & 0x8000) != 0) 
                {
                    temp = (ushort)((temp << 1) ^ poly);
                } 
                else 
                {
                    temp <<= 1;
                }
                a <<= 1;
            }
            xx.zz = temp;
	    sprintf (vv,"  0x");
    	    sci0putstring(vv);
            for(j = 12; j >= 0; j -= 4) 
		{
			sprintf (vv,"%1x",((xx.zz>>j)&0x000f));
	    	    	sci0putstring(vv);
		}
	    if ( (i & 0x0007) == 7) 
	    	sci0putstring(",\n");
	    else
	    	sci0putstring(",");
        }
	sci0putstring("\n};\n");
	
	return;		
}

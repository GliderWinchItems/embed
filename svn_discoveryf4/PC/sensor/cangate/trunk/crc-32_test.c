/* ************************************************************************
crc-32_test.c
 **************************************************************************/


/*  

https://my.st.com/public/STe2ecommunities/mcu/Lists/cortex_mx_stm32/Flat.aspx?RootFolder=https%3a%2f%2fmy.st.com%2fpublic%2fSTe2ecommunities%2fmcu%2fLists%2fcortex_mx_stm32%2fCC%2b%2b%20Example%20for%20CRC%20Calculation&FolderCTID=0x01200200770978C69A1141439FE559EB459D7580009C4E14902C3CDE46A77F0FFD06506F5B&currentviews=2282

gcc crc-32_test.c -o crc-32_test -lz && ./crc-32_test 
*/


unsigned int Crc32(unsigned int* pData, unsigned int intcount)
{
  	int i;
	int k;
	unsigned int Crc = 0xFFFFFFFF;
 
	for (k = 0; k < intcount; k += 1)
	{	
	 	Crc = Crc ^ *pData++;

  		for(i=0; i<32; i++)
   		if (Crc & 0x80000000)
      		Crc = (Crc << 1) ^ 0x04C11DB7; // Polynomial used in STM32
    		else
      		Crc = (Crc << 1);
	}

  	return(Crc);
}
 
 
unsigned int Crc32Fast(unsigned int* pData, unsigned int intcount)
{
  static const unsigned int CrcTable[16] = { // Nibble lookup table for 0x04C11DB7 polynomial
    0x00000000,0x04C11DB7,0x09823B6E,0x0D4326D9,0x130476DC,0x17C56B6B,0x1A864DB2,0x1E475005,
    0x2608EDB8,0x22C9F00F,0x2F8AD6D6,0x2B4BCB61,0x350C9B64,0x31CD86D3,0x3C8EA00A,0x384FBDBD };

	int k;
	unsigned int Crc = 0xFFFFFFFF;

 	for (k = 0; k < intcount; k += 1)
	{	
		Crc = Crc ^ *pData++; // Apply all 32-bits
 
  		// Process 32-bits, 4 at a time, or 8 rounds
 
	  Crc = (Crc << 4) ^ CrcTable[Crc >> 28]; // Assumes 32-bit reg, masking index to 4-bits
	  Crc = (Crc << 4) ^ CrcTable[Crc >> 28]; //  0x04C11DB7 Polynomial used in STM32
	  Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
	  Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
	  Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
	  Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
	  Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
	  Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
 	}
  	return(Crc);
}
void main (void)
{
unsigned int z[4] = { 0x12345678, 0x87654321, 0xabcdef12, 0x1};
  printf("%08X\n\n",Crc32(z,4)); // 
 
  printf("%08X\n\n",Crc32Fast(z,4)); // 
}


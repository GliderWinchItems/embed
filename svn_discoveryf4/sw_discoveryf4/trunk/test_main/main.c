static volatile long foo;

main()
{
	int i;
	
	for(i=0; i<1000; i+=1)
		foo += 1;
}

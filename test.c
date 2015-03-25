#include <stdio.h>
#include <limits.h>

main()
{
	printf("sizeof int = %d\n", sizeof(int));
	printf("sizeof long = %d\n", sizeof(long));
	printf("%d\n", INT_MAX);
	printf("%d\n", INT_MIN);
	printf("%ld\n", LONG_MAX);
	printf("%ld\n", LONG_MIN);
}


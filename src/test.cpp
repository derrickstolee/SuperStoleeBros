#include <stdio.h>

template <class T>
T divide(T a, T b)
{
	int result = 0;
	try
{
	result = a/b;
}
	catch(...)
{
	throw 0;
}

return result;
}


int main(void)
{
	int a = 1;	
	int b = 0;


	try
	{
		divide<int>(a,b);
	}
	catch(int i)
	{
		printf("threw %d\n",i);
	}
	catch(...)
	{
		printf("caught somethign\n");
	}
return 0;
}

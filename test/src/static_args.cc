#include <iostream>
using namespace std;
int main()
{
	int n = 0;
	while(true)
	{
		static int a = 0;
		if(a < ++n) a = n;
		if(a < 10) cout<<"a = "<<a<<std::endl;
	}
	return 0;
}
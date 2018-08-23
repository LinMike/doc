#include <iostream>
#include <cstdlib>
#include <float.h>
#include <cmath>
using namespace std;

int main()
{
	float vx, vy, x, y;
	float shoot_target_x = 21.5;
	float shoot_target_y = 0;
	while (true)
	{
		cin >> x >> y >> vx >> vy;
		float shoot_distance = sqrt((x-shoot_target_x)*(x-shoot_target_x) + (y-shoot_target_y)*(y-shoot_target_y));
		float alpha = atan2(y-shoot_target_y, x-shoot_target_x);
		bool has_result = false;
		// theta = [16, 26], v0 = [3, 20]
		float result_theta = 0, result_v0 = 0, dis = FLT_MAX;
		for(int j=160;j<=260;j+=5) //theta
		{
			for(int i=30;i<=200;i+=5)
			{
				float theta = j*1.0/10, v0 = i * 1.0 / 10;
				float h = v0*v0*sin(theta/180.0*M_PI)*sin(theta/180.0*M_PI)/(2*9.8);
				float sx = v0*v0*sin(2.0*theta/180.0*M_PI)/9.8 * cos(alpha) + vx* 2*v0*sin(theta/180.0*M_PI)/9.8;
				float sy = v0*v0*sin(2.0*theta/180.0*M_PI)/9.8 * sin(alpha) + vx* 2*v0*sin(theta/180.0*M_PI)/9.8;
				if(h >= 1.5 && h <= 3.0 && fabs(sqrt(sx*sx + sy*sy) - shoot_distance) < dis
						/*&& fabs(fabs(sx)-fabs(shoot_target_x-x))<0.1 && fabs(fabs(sy)-fabs(shoot_target_y-y))<0.1*/)
				{
					result_theta = theta;
					result_v0 = v0;
					has_result = true;
					dis = fabs(sqrt(sx*sx + sy*sy) - shoot_distance);
					cout<<"h = "<<h<<", s = "<<sqrt(sx*sx + sy*sy)<<", v0 = "<<v0<<", theta = "<<theta<<std::endl;
				}
			}
		}
 		if(has_result == false) cout<<"has_result = "<<has_result<<std::endl;
 		else std::cout<<"result theta = "<<result_theta<<", result v0 = "<<result_v0<<std::endl;
	}
	return 0;
}

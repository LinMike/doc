#include <opencv2/opencv.hpp>
#include <opencv2/gpu/gpu.hpp>
#include <boost/filesystem.hpp>
class bgs_gmg
{
public:
	bgs_gmg(cv::Size image_size)
	{
		gmg_.numInitializationFrames = 30;
		gmg_.decisionThreshold = 0.98;
		gmg_.initialize(image_size);
		fg_.create(image_size, CV_8U);
	}
	cv::gpu::GpuMat Apply(cv::gpu::GpuMat src)
	{
		gmg_(src, fg_, 0.005);
		return fg_;
	}
	cv::KeyPoint BuildDownsideCircleFromEllipse(const cv::RotatedRect &ellipse) const {
		assert(ellipse.angle>=0 && ellipse.angle<=180);
		const float d = (ellipse.size.height - ellipse.size.width) * 0.5;
		assert(d >= 0);
		float theta = ellipse.angle + (ellipse.angle>=90 ? -90 : 90);
		theta = theta / 180. * M_PI;
		cv::Point2f delta(d*cos(theta), d*sin(theta));
		if (fabs(ellipse.angle-90) <= 10)
			delta = cv::Point2f(0, 0);

		cv::KeyPoint circle;
		circle.size = ellipse.size.width * 0.5;
		circle.pt = ellipse.center + delta;
		return circle;
	}
	double FitEllipse(const std::vector<cv::Point> &contour, cv::RotatedRect &ellipse) const {
		double dist = 0;
		std::vector<cv::Point> contourEllipse;

		ellipse = cv::fitEllipse(contour);

		cv::Size axes(ellipse.size.width/2, ellipse.size.height/2);
		cv::ellipse2Poly(ellipse.center, axes, ellipse.angle, 0, 360, 1, contourEllipse);

		for (size_t i=0; i<contour.size(); i++)
			dist += fabs(cv::pointPolygonTest(contourEllipse, contour[i], true));

		return dist/contour.size();
	}
	float Eccentricity(const cv::RotatedRect &ellipse) const {
		float a = std::max(ellipse.size.width, ellipse.size.height) * 0.5;
		float b = std::min(ellipse.size.width, ellipse.size.height) * 0.5;
		return sqrt(1 - (b*b)/(a*a));
	}
	void DetectBall(cv::Mat &fg, cv::Mat &raw_img, std::vector<cv::KeyPoint> &balls)
	{
		std::vector<std::vector<cv::Point> > contours;
		cv::findContours(fg, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

		float eccenty;
		double ellipseness, contourArea, ratio;
		cv::RotatedRect ellipse;
		cv::KeyPoint ballItem;
		for (uint i=0; i<contours.size(); i++) {
			if (contours[i].size() < 5)
				continue;

			ellipseness =  FitEllipse(contours[i], ellipse);

			contourArea = cv::contourArea(contours[i]);
			eccenty = Eccentricity(ellipse);
			ratio = ellipseness/sqrt(contourArea);

			if (contourArea < 4.5 || eccenty > 0.86 || ratio > 0.21) {

				continue;
			}

			ballItem = BuildDownsideCircleFromEllipse(ellipse);
			balls.push_back(ballItem);

			cv::ellipse(raw_img, ellipse, cv::Scalar(0, 255, 0));
			cv::circle(raw_img, ballItem.pt, ballItem.size, cv::Scalar(0, 0, 255));
		}
	}
private:
	cv::gpu::GpuMat fg_;
	cv::gpu::GMG_GPU gmg_;
};
int main(int argc, char *argv[])
{
	if(argc != 2) {
		std::cout<<"input image path"<<std::endl;
		return 0;
	}
	std::string img_path(argv[1]);
	if(img_path.empty() || boost::filesystem::exists(img_path) == false)
	{
		std::cout<<"empty image path"<<std::endl;
		return 0;
	}
	boost::filesystem::directory_iterator it_end;
	bgs_gmg bgs(cv::Size(1280, 1024));
	std::vector<std::string> file_names;
	for(boost::filesystem::directory_iterator it(img_path); it != it_end; ++it)
	{
		std::string file_name = it->path().string();
		file_names.push_back(file_name);
	}
	std::sort(file_names.begin(), file_names.end());
	for(std::vector<std::string>::iterator it=file_names.begin();it!=file_names.end();it++)
	{
		std::cout<<"current process file name = "<<*it<<std::endl;
		cv::Mat img = cv::imread(*it);
		cv::gpu::GpuMat gpu_mat, fg_mat;
		cv::Mat fg_img;
		gpu_mat.upload(img);
		fg_mat = bgs.Apply(gpu_mat);
		fg_mat.download(fg_img);
		cv::erode(fg_img, fg_img, cv::Mat(), cv::Point(-1, -1), 2);
		cv::dilate(fg_img, fg_img, cv::Mat(), cv::Point(-1, -1), 2);

		std::cout<<"apply fg done"<<std::endl;
		std::vector<cv::KeyPoint> balls;
		bgs.DetectBall(fg_img, img, balls);
//		cv::imshow("fg img", fg_img);
//		cv::imshow("img", img);
//		cv::waitKey(-1);

		//TODO: set roi range in raw image to get new image 20x20 with ball
		for(std::vector<cv::KeyPoint>::iterator it = balls.begin(); it != balls.end(); it++) {
			float lt_x = it->pt.x - 20 <= 1279 ? (it->pt.x-20 > 0 ? it->pt.x-20 : 0) : 1279;
			float lt_y = it->pt.y - 20 <= 1023 ? (it->pt.y-20 > 0 ? it->pt.y-20 : 0) : 1023;
			float rb_x = it->pt.x + 20 <= 1279 ? (it->pt.x+20 > 0 ? it->pt.x+20 : 0) : 1279;
			float rb_y = it->pt.y + 20 <= 1023 ? (it->pt.y+20 > 0 ? it->pt.y+20 : 0) : 1023;
			std::cout<<cv::Point(lt_x, lt_y)<<", "<< cv::Point(rb_x, rb_y)<<"from ["<<img.rows<<", "<<img.cols<<"]"<<std::endl;
			cv::Mat roi_img(img, cv::Rect(cv::Point(lt_x, lt_y), cv::Point(rb_x, rb_y)));
			cv::imshow("roi img", roi_img);
			cv::waitKey(-1);
		}

	}
	return 0;
}

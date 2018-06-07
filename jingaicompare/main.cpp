#include "process_method.h"
#include<cstdlib>

int main() 
{
	/********************************get current pic****************************************/
	VideoCapture *capture;		//视频流
	printf("read video 10.10.150.101\n");
	capture = new VideoCapture("rtsp://admin:sutpc654321@10.10.150.101:554/h264/ch1/sub/av_stream");
	
	//capture = new VideoCapture("test.mp4");
	printf("------VideoCapture------------\n");
	if (!capture->isOpened())
	{
		std::cerr << "Cannot initialize video!" << std::endl;
	}
	/********************************get current pic****************************************/

	move_target_detect mtd;
	int flag;
	Mat current_pic;
	
	while (1) 
	{
/********************************get current pic****************************************/
		capture->read(current_pic);
		//current_pic=imread("1.jpg");
/********************************get current pic****************************************/
		// printf("----loop_process----\n");
		
		flag = mtd.loop_process(current_pic);
		if (flag==1)
			cout << "save alert image" << endl;
			// mtd.Return_saved_file();
		if (flag==2)
			cout << "save alert disappear image" << endl;
			// mtd.Return_saved_file();
		/***********test code*************/
		if (cvWaitKey(10) == 'q')
			break;
		/***********test code*************/
	}
	return 0;
}

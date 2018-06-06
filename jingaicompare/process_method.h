#pragma once
#include <iostream>
// #include "SuBSENSE.h"
// #include "IBGS.h"
// #include "bgslibrary.h"
#include "socket_client.h"
#include <time.h> 
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
using namespace cv;
using namespace std;

#define resizedHeight       113     
#define resizedWidth        200     
#define FrameHistory        5      //设置历史帧500

class move_target_detect 
{
public:
	move_target_detect();
	~move_target_detect();
	int loop_process(Mat current_pic);			//放入循环代码中，每次执行后如果保存了图片则返回1
	cv::Mat get_ROI(cv::Mat srcImage);	//ROI get
	cv::MatND calculate_hist(cv::Mat img); //calculate histgram of an image
	int compare(cv::Mat temp_img, cv::Mat curr_im); //compare the histgram of temp and current image
	int send_img_socket(char status);
	double getPSNR(cv::Mat temp_img, cv::Mat curr_img);
	Scalar getMSSIM( const Mat& i1, const Mat& i2);
	char* Return_saved_file();
private:
	
	// IBGS *bgs;							//subsense方法
	cv::Mat img_input;				//当前输入帧
	cv::Mat img_temp;				//template frame
	int frame_count;					//帧计数
	int alert_count;					//alert count
	int disappear_count;					//disappear count
	int start_time;						//执行背景提取的时间点
	int save_img_time;				//执行图片保存的时间点
	int temp_update_time;			//执行temp update的时间点
	int iDiffNum;
	char Stored_file_name[200];
	my_client *client_to_DLServer;
	char ip_addr[20];
	char port[20];
};
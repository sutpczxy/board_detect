#include "process_method.h"
#include <unistd.h>
#define DIFF_T  15
move_target_detect::move_target_detect()
{
	std::cout << "Using OpenCV " << CV_MAJOR_VERSION << "." << CV_MINOR_VERSION << "." << CV_SUBMINOR_VERSION << std::endl;


	//bgs = new SuBSENSE();		/* Background Subtraction Methods */
	frame_count = 0;
	alert_count = 0;
	disappear_count=0;
	start_time = clock();
	save_img_time = clock();
	temp_update_time = clock();
	// client_to_DLServer = new my_client();

	// sprintf(ip_addr, "10.10.2.167");
	// sprintf(port, "9999");
}


int move_target_detect::loop_process(cv::Mat current_pic)
{
	current_pic.copyTo(img_input);
	if (img_input.empty())
	{
		//cvWaitKey(10);
		printf("------img_input.empty------\n");
		usleep(100*1000);
		return 0;
	}
	if(img_temp.empty())
	{
		current_pic.copyTo(img_temp);
	}

	Mat operateImg;
	//cv::resize(img_input, operateImg, cv::Size(resizedWidth, resizedHeight));

	cv::Mat img_mask;
	cv::Mat img_bkgmodel;
	int end = clock();
	float dt1 = float(float(end - start_time) / CLOCKS_PER_SEC);
	float dt2 = float(float(end - save_img_time) / CLOCKS_PER_SEC);
	float dt3 = float(float(end - temp_update_time) / CLOCKS_PER_SEC);
	int save_flag = 0;			//保存图片flag

	if (dt1 > 1) 
	{
		//printf("---------bgs->process------- \n");
		//bgs->process(operateImg, img_mask, img_bkgmodel);		 // by default, it shows automatically the foreground mask image		
		//frame_count++;
		int diff_flag=compare(img_temp,img_input);
		diff_flag+=getPSNR(img_temp,img_input);
		if(diff_flag)
		{
			if(alert_count<10&&dt3>10)
			{
				img_input.copyTo(img_temp);
				temp_update_time = clock();	
			}
			if(alert_count>=10)
			{
				disappear_count++;
				if(disappear_count==10)
				{
					img_input.copyTo(img_temp);
					temp_update_time = clock();	
					send_img_socket(0x02);
					save_flag=2;
					disappear_count=0;
					alert_count=0;
				}
			}
		}
		else{
			alert_count++;
			if(alert_count>600)
			{
				alert_count=0;
				img_input.copyTo(img_temp);
			}
		}
		printf("alert_count:%d \n",alert_count);

				
		// if (frame_count <= FrameHistory)
		// {
		// 	//cvWaitKey(10);
		// 	usleep(10*1000);
		// 	return 0;
		// }
		//if (!img_mask.empty())
		if (alert_count==10)		//alert
		{
			// cv::Mat th;
			// cv::erode(img_mask, th, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)), Point(-1, -1), 1);
			// cv::dilate(th, th, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)), Point(-1, -1), 1);
			// //cv::dilate(th, th, getStructuringElement(MORPH_RECT, Size(5, 5)), Point(-1, -1), 1);

			// vector<vector<Point>> contours;
			// vector<Vec4i> hierarchy;
			// float area;
			// int bkg_flag = 1;			//判定是否可以作为背景

			// findContours(th, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
			// for (int i = 0; i < contours.size(); i++)
			// {
			// 	area = contourArea(contours[i]);
			// 	if (area >= 100)
			// 		bkg_flag = 0;
			// }
			
			

			//保存背景图片供识别
			//if (dt2 > 60 && bkg_flag==1)
			send_img_socket(0x01);
			save_flag=1;
			alert_count++;
		}
		start_time = clock();
	}
	//cvWaitKey(10);
	usleep(10*1000);
	return save_flag;
}

cv::Mat move_target_detect::get_ROI(cv::Mat srcImage)
{
	int w1=int(srcImage.cols*0.425);
	int w2=int(srcImage.rows*0.352);
	int w3=int(srcImage.cols*0.039);
	int w4=int(srcImage.rows*0.052);
	cv::Mat ImageROI = srcImage(cv::Rect(w1, w2, w3, w4));
	//imshow("img", ImageROI);
	//cvWaitKey(0);
	//cvDestroyWindow("img");
	return ImageROI;
}

cv::MatND move_target_detect::calculate_hist(cv::Mat img)
{
	cv::Mat hsv_base;
	cv::cvtColor(img, hsv_base, CV_BGR2HSV);
	
	cv::MatND srcHist;
	//H¡¢SÍ¨µÀ  
	int channels[] = { 0, 1 };
	int histSize[] = { 50, 60 };
	float HRanges[] = { 0, 256 };
	float SRanges[] = { 0, 180 };
	const float *ranges[] = { HRanges, SRanges };

	//½øÐÐÖ±·½Í¼µÄ¼ÆËã  
	calcHist(&hsv_base, 1, channels, cv::Mat(), srcHist, 2, histSize, ranges, true, false);
	normalize(srcHist, srcHist, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());

	return srcHist;
}

int move_target_detect::compare(cv::Mat temp_img, cv::Mat curr_img)
{
	cv::Mat tmp_ROI, cur_ROI, matDst1, matDst2;

	cv::MatND  tmp_hist, cur_hist;
	cv::Mat current_image = curr_img;
	
	cv::cvtColor(temp_img, temp_img, CV_BGR2GRAY);
	cv::cvtColor(curr_img, curr_img, CV_BGR2GRAY);
	
	//equalizeHist(temp_img, temp_img);
	//equalizeHist(curr_img, curr_img);

	tmp_ROI = get_ROI(temp_img);
	cur_ROI = get_ROI(curr_img);
	// imshow("tmp", tmp_ROI);
	// imshow("cur", cur_ROI);


	cv::resize(tmp_ROI, matDst1, cv::Size(8, 8), 0, 0, cv::INTER_CUBIC);
	cv::resize(cur_ROI, matDst2, cv::Size(8, 8), 0, 0, cv::INTER_CUBIC);


	int iAvg1 = 0, iAvg2 = 0;
	int arr1[64], arr2[64];

	for (int i = 0; i < 8; i++)
	{
		uchar* data1 = matDst1.ptr<uchar>(i);
		uchar* data2 = matDst2.ptr<uchar>(i);

		int tmp = i * 8;

		for (int j = 0; j < 8; j++)
		{
			int tmp1 = tmp + j;

			arr1[tmp1] = data1[j] / 4 * 4;
			arr2[tmp1] = data2[j] / 4 * 4;

			iAvg1 += arr1[tmp1];
			iAvg2 += arr2[tmp1];
		}
	}

	iAvg1 /= 64;
	iAvg2 /= 64;

	for (int i = 0; i < 64; i++)
	{
		arr1[i] = (arr1[i] >= iAvg1) ? 1 : 0;
		arr2[i] = (arr2[i] >= iAvg2) ? 1 : 0;
	}

	iDiffNum = 0;

	for (int i = 0; i < 64; i++)
		if (arr1[i] != arr2[i])
			++iDiffNum;

	cout << "iDiffNum = " << iDiffNum << endl;

	if (iDiffNum <= DIFF_T)				
	{	
		return 1;
	}
	else
		return 0;
}

int move_target_detect::send_img_socket(char status)
{
	printf("---------save image------- \n");
	const time_t t = time(NULL);
	struct tm* current_time = localtime(&t);			
	sprintf(Stored_file_name, "B_10000001_%04d%02d%02d%02d%02d%02d.jpg",
		current_time->tm_year + 1900, current_time->tm_mon + 1, current_time->tm_mday, current_time->tm_hour, current_time->tm_min, current_time->tm_sec);			
	Mat img_output;
	if(status==0x01)
	{
		int w1=int(img_input.cols*0.425);
		int w2=int(img_input.rows*0.352);
		int w3=int(img_input.cols*0.039);
		int w4=int(img_input.rows*0.052);
		rectangle(img_input, cv::Rect(w1, w2, w3, w4), CV_RGB(0, 255, 30), 3, 1, 0);
	}
	imwrite(Stored_file_name, img_input);

	/****************tftp operation*******************************/
	// char Tftpcmd[200] = {'\0'};
	// sprintf(Tftpcmd, "/home/tftpc 10.10.2.167 -o -p  %s   \n", Save_file_name);
	// system(Tftpcmd);
	// sprintf(Tftpcmd, "rm %s \n", Save_file_name);
	// system(Tftpcmd);
	/****************tftp operation*******************************/
	/****************send data frame by socket********************/
	// char sendmsg[40];
	// char head[] = { 0xAA,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1B,0x07 };
	// char lamp_ID[] = { "10000001" };
	// for (int i = 1; i < 9; i++)
	// {
	// 	head[i] = lamp_ID[i - 1];
	// }
	// char end[] = { 0x06,0xBB };
	// char type[] = { 0x01 };
	// char sendFileName[100] = { '\0' };
	// sprintf(sendFileName, "B_10000001_%04d%02d%02d%02d%02d%02d",
	// 	current_time->tm_year + 1900, current_time->tm_mon + 1, current_time->tm_mday, current_time->tm_hour, current_time->tm_min, current_time->tm_sec);

	// for (int i = 0; i < 11; i++)
	// {
	// 	sendmsg[i] = head[i];
	// }
	// for (int i = 11; i < 36; i++)
	// {
	// 	sendmsg[i] = sendFileName[i - 11];
	// }
	// sendmsg[36] = type[0];
	// sendmsg[37] = status;
	// for (int i = 38; i < 40; i++)
	// {
	// 	sendmsg[i] = end[i - 38];
	// }
	// client_to_DLServer->init_socket(ip_addr,port);
	// client_to_DLServer->send_msg(sendmsg);
	// client_to_DLServer->close_socket();
	/****************send data frame by socket********************/
	save_img_time = clock();
	return 0;
}


double move_target_detect::getPSNR(cv::Mat temp_img, cv::Mat curr_img)
{
	cv::Mat tmp_ROI, cur_ROI, matDst1, matDst2;

	cv::MatND  tmp_hist, cur_hist;
	cv::Mat current_image = curr_img;
	
	cv::cvtColor(temp_img, temp_img, CV_BGR2GRAY);
	cv::cvtColor(curr_img, curr_img, CV_BGR2GRAY);
	//equalizeHist(temp_img, temp_img);
	//equalizeHist(curr_img, curr_img);

	tmp_ROI = get_ROI(temp_img);
	cur_ROI = get_ROI(curr_img);
	// imshow("tmp", tmp_ROI);
	// imshow("cur", cur_ROI);

    Mat s1; 
    absdiff(tmp_ROI, cur_ROI, s1);       // |I1 - I2|
    s1.convertTo(s1, CV_32F);  // cannot make a square on 8 bits
    s1 = s1.mul(s1);           // |I1 - I2|^2

    Scalar s = sum(s1);         // sum elements per channel

    double sse = s.val[0] + s.val[1] + s.val[2]; // sum channels

    double  mse =sse /(double)(tmp_ROI.channels() * tmp_ROI.total());
    double psnr = 10.0*log10((255*255)/mse);

	cout << "psnr = " << psnr << endl;

	if (psnr >= 20)				
	{
		return 1;
	}
	else
		return 0;

}


Scalar move_target_detect::getMSSIM( const Mat& i1, const Mat& i2)
{ 
    const double C1 = 6.5025, C2 = 58.5225;
    /***************************** INITS **********************************/
    int d     = CV_32F;

    Mat I1, I2; 
    i1.convertTo(I1, d);           // cannot calculate on one byte large values
    i2.convertTo(I2, d); 

    Mat I2_2   = I2.mul(I2);        // I2^2
    Mat I1_2   = I1.mul(I1);        // I1^2
    Mat I1_I2  = I1.mul(I2);        // I1 * I2

    /*************************** END INITS **********************************/

    Mat mu1, mu2;   // PRELIMINARY COMPUTING
    GaussianBlur(I1, mu1, Size(11, 11), 1.5);
    GaussianBlur(I2, mu2, Size(11, 11), 1.5);

    Mat mu1_2   =   mu1.mul(mu1);    
    Mat mu2_2   =   mu2.mul(mu2); 
    Mat mu1_mu2 =   mu1.mul(mu2);

    Mat sigma1_2, sigma2_2, sigma12; 

    GaussianBlur(I1_2, sigma1_2, Size(11, 11), 1.5);
    sigma1_2 -= mu1_2;

    GaussianBlur(I2_2, sigma2_2, Size(11, 11), 1.5);
    sigma2_2 -= mu2_2;

    GaussianBlur(I1_I2, sigma12, Size(11, 11), 1.5);
    sigma12 -= mu1_mu2;

    ///////////////////////////////// FORMULA ////////////////////////////////
    Mat t1, t2, t3; 

    t1 = 2 * mu1_mu2 + C1; 
    t2 = 2 * sigma12 + C2; 
    t3 = t1.mul(t2);              // t3 = ((2*mu1_mu2 + C1).*(2*sigma12 + C2))

    t1 = mu1_2 + mu2_2 + C1; 
    t2 = sigma1_2 + sigma2_2 + C2;     
    t1 = t1.mul(t2);               // t1 =((mu1_2 + mu2_2 + C1).*(sigma1_2 + sigma2_2 + C2))

    Mat ssim_map;
    divide(t3, t1, ssim_map);      // ssim_map =  t3./t1;

    Scalar mssim = mean( ssim_map ); // mssim = average of ssim map
    return mssim; 
}

char* move_target_detect::Return_saved_file()
{
	return Stored_file_name;
}
move_target_detect::~move_target_detect()
{
	printf("delete target");
	// delete bgs;
	// capture->release();
	cvDestroyAllWindows();
}
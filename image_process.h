//*******************//
//This file's funcation is:
//1.Get image num pitch from iamge
//2.Classify the image and get num
//3.Get the camera motion result
//Author:TomRen
//E-mail��jiejiren@sjtu.edu.cn
//******************//

#ifndef IMAGE_PROCESS_H
#define IMAGE_PROCESS_H

#include <opencv2\highgui\highgui.hpp>
#include <opencv2\opencv.hpp> 
#include <iostream>

using namespace std;
using namespace cv;

//---DownCamera------//


//-1- read image into cache  from AirSim API


//-2- processs distort with warpperspective
void undistort_img(Mat &input_img, Mat &output_img) {
	Mat cameraMatrix = Mat::eye(3, 3, CV_64F);
	cameraMatrix.at<double>(0, 0) = 246.33203;
	cameraMatrix.at<double>(0, 1) = 0;
	cameraMatrix.at<double>(0, 2) = 289.82891;
	cameraMatrix.at<double>(1, 1) = 246.65002;
	cameraMatrix.at<double>(1, 2) = 253.71157;

	Mat distCoeffs = Mat::zeros(5, 1, CV_64F);
	distCoeffs.at<double>(0, 0) = -0.04114;
	distCoeffs.at<double>(1, 0) = 0.04878;
	distCoeffs.at<double>(2, 0) = -0.02292;
	distCoeffs.at<double>(3, 0) = 0.01071;
	distCoeffs.at<double>(4, 0) = 0;

	//Mat view, rview, map1, map2;
	//Size imageSize;
	//imageSize = input_img.size();
	//cvInitUndistortMap(cameraMatrix, distCoeffs, map1, map2);
	//remap(input_img, output_img, map1, map2, INTER_LINEAR);
	undistort(input_img, output_img, cameraMatrix, distCoeffs);
}







//-3- find rectangle num pitch in down image
//���ȷɸ߶�λ���е�����ͣ��ƺ
//ÿ�ε�������Ҫclear vector �����ͻ   >>c.clear(); 
//�˴���Ҫ����Ѳ������������ȷ��һ���߶�Ȼ����ֵ���Σ�����������ֲ�ȷ��һ����С��Ѳ���߶����ڶ�λ��ȷ����
//TODO: ��������ֵ��Ӧʵ�ʾ���
//Output: loading tarmac global locations  (from picture to GPS)   Ѳ����õ�ȫ��ͣ��ƺ��λ��
void downFindAllRectangle(Mat &input_img, vector<RotatedRect> &all_location) {
	namedWindow("down_view");
	imshow("down_view", input_img);
	waitKey();



	//deliate in the region
	Mat element = getStructuringElement(MORPH_RECT, Size(15, 15));
	Mat dilate_img;

	dilate(input_img, dilate_img, element);

	imshow("down_view", dilate_img);
	waitKey();


	//binary process
	Mat output_bin_img;
	threshold(dilate_img, output_bin_img, 200, 255, CV_THRESH_TOZERO);    //BRIGHT REGION
	imshow("down_view", output_bin_img);
	waitKey();


	//find contour and rect it
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;

	findContours(output_bin_img, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);



	//draw and display�� return location
	RotatedRect location;
	Point2f vertex[4];
	location.points(vertex);
	for (int j = 0; j < contours.size(); j++) {
		location = minAreaRect(contours[j]);
		location.points(vertex);
		all_location.push_back(location);

		for (int i = 0; i < 4; i++) {
			line(input_img, vertex[i], vertex[(i + 1) % 4], cv::Scalar(255));
		}
	}
	imshow("down_view", input_img);
	waitKey();
	//TODO: REDLEASE ALL VARIABLES

}

//-4-
//Find the red circle top seview location in top view
//Ѱ�����еĺ�ɫȦ��λ�ã�ͬ��Ҫȷ��Ѳ���߶�*hight0*��������ֵ
//Output: red circles global locations    (from picture to GPS)  Ѳ����õ�ȫ�ֺ�ɫȦ��λ��
cv::Rect downFindAllRedCircle(Mat &input_img) {

	//use hue to find the red region  from   Hue, Saturation, Value  --A. R. Smith1978//
	Mat img_hsv;
	Mat red_out(640, 480, CV_8U);
	cvtColor(input_img, img_hsv, CV_BGR2HSV);    //change color into hsv

	int iLowH = 0;
	int iHighH = 10;

	int iLowS = 42;
	int iHighS = 255;

	int iLowV = 43;
	int iHighV = 255;

	//�ղ��� (����һЩ��ͨ��)
	inRange(img_hsv, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), red_out); //Threshold the image

	Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));
	erode(red_out, red_out, element);
	element = getStructuringElement(MORPH_RECT, Size(15, 15));
	dilate(red_out, red_out, element);
	//imshow("Thresholded Image", red_out); //show the thresholded image
	//imshow("Original", input_img); //show the original image
	//waitKey();


	//find max_contours
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;

	// ������������Ӧ��ͨ��
	findContours(red_out, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	//problems: https://blog.csdn.net/fightingforcv/article/details/78423866

	// Ѱ�������ͨ��
	double maxArea = 0;
	vector<cv::Point> maxContour;
	for (size_t i = 0; i < contours.size(); i++)
	{
		double area = cv::contourArea(contours[i]);
		if (area > maxArea)
		{
			maxArea = area;
			maxContour = contours[i];
		}
	}
	//�����Ҫ����ȫ��������push vector���ؼ���


	// ������תΪ���ο�
	cv::Rect maxRect = cv::boundingRect(maxContour);
	/*
	cout << maxRect << endl;
	// ��ʾ��ͨ��
	cv::Mat result1, result2;

	input_img.copyTo(result1);
	input_img.copyTo(result2);

	for (size_t i = 0; i < contours.size(); i++)
	{
	cv::Rect r = cv::boundingRect(contours[i]);
	cv::rectangle(result1, r, cv::Scalar(255));
	}
	cv::imshow("all regions", result1);
	cv::waitKey();

	cv::rectangle(result2, maxRect, cv::Scalar(255));
	cv::imshow("largest region", result2);

	waitKey();
	*/

	return maxRect;
}


//-5- 
//����ȫ�����꣬�ɵ���Ӧͣ��ƺ�Ϸ��̶��߶�*hight1*���ɼ�ͼ��
//����ͼ�񣬿�ȡ��Ӧ��ͼ������rect��resize����ΪSVM/DNN������
//Output: number rect region
void downFindRectangle(Mat &input_img, Mat &output_img) {

	// canny ��� ��Ե
	Mat grey;
	Mat edges;
	int lowThresh = 50; int lowThrestHigh = 100; int apatureszie = 3;

	cvtColor(input_img, grey, COLOR_BGR2GRAY);

	//denoise
	//blur(grey, grey, Size(3, 3));
	Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));
	//erode(grey, grey, element);
	morphologyEx(grey, grey, MORPH_OPEN, element);
	blur(grey, grey, Size(2, 2));

	//imshow("down", grey);
	Canny(grey, edges, lowThresh, lowThrestHigh, apatureszie);
	//imshow("edges",edges);

	//Ѱ�Ұ���
	vector<vector<Point>> contours;
	//threshold(edges, edges, 128, 255, THRESH_BINARY);
	findContours(edges, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	double maxare = 0; int max_i = 0;
	for (size_t i = 0; i < contours.size(); i++) {  //size_t special typedef
		double area = contourArea(contours[i]);
		if (area > maxare) {
			maxare = area;
			max_i = i;
		}
		//cout << contours.size()<<"   "<< max_i << endl;

	}


	//���������
	Rect rect = boundingRect(contours[max_i]);
	Mat rect_roi;
	input_img(rect).copyTo(rect_roi);
	//���ؾ���Ƭ Mat
	output_img = rect_roi;


}

//-6- find center
//If the drone in front of the circle, we could use the houghcircle to detect the cycle and center//if not work use the frontFindRectangle mass center
//����ȫ�����꣬ �ɵ���Ӧ��Ȧǰ�棬�̶��߶�*hight2*��ǰ�����*distance*
//Output: circle center  �õ���ӦȦ������
void frontFindCircle(Mat &input_img, vector<Vec3f> &circles) {
	Mat grey;
	cvtColor(input_img, grey, COLOR_BGR2GRAY);
	GaussianBlur(grey, grey, Size(9, 9), 2, 2);
	HoughCircles(grey, circles, HOUGH_GRADIENT, 1.5, 10, 200, 50, 100, 200);

	//TODO: Find the biggest one argmax(circles.r)
	/*
	for (size_t i = 0; i < circles.size(); i++)
	{
	Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
	int radius = cvRound(circles[i][2]);
	// circle center
	circle(input_img, center, 3, Scalar(0, 255, 0), -1, 8, 0);
	// circle outline
	circle(input_img, center, radius, Scalar(0, 0, 255), 3, 8, 0);
	}
	imshow("hough", input_img);
	waitKey();
	*/
}




//-7- find rectangle num pitch in font image
//����ȫ�����꣬ �ɵ���Ӧ��Ȧǰ�棬�̶��߶�*hight2*��ǰ�����*distance*
//����ͼ�񣬿�ȡ��Ӧ��ͼ������rect��resize����ΪSVM/DNN������
//Output: number rect region
Rect frontFindRectangle(Mat &input_img) {


	//find read region
	int iLowH = 0; int iHighH = 7; int iLowS = 43; int iHighS = 255; int iLowV = 43; int iHighV = 255;


	Mat img_hsv;
	Mat red_out(640, 480, CV_8U);
	cvtColor(input_img, img_hsv, CV_BGR2HSV);    //change color into hsv

	inRange(img_hsv, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), red_out); //Threshold the image

																							//imshow("origin", input_img);
																							//imshow("hsv", red_out);



																							//if find circle in the sunny side, hough is enough!
	Mat element1 = getStructuringElement(MORPH_RECT, Size(3, 3));
	Mat element2 = getStructuringElement(MORPH_RECT, Size(30, 30));
	erode(red_out, red_out, element1);
	dilate(red_out, red_out, element2);
	imshow("dilate", red_out);

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	Mat circle_font = input_img.clone();

	// ������������Ӧ��ͨ��
	findContours(red_out, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	//problems: https://blog.csdn.net/fightingforcv/article/details/78423866

	// Ѱ�������ͨ��
	double maxArea = 0;
	vector<cv::Point> maxContour;
	for (size_t i = 0; i < contours.size(); i++)
	{
		double area = cv::contourArea(contours[i]);
		if (area > maxArea)
		{
			maxArea = area;
			maxContour = contours[i];
		}
	}

	// ������תΪ���ο�
	cv::Rect maxRect = cv::boundingRect(maxContour);
	maxRect.height = maxRect.height * 2;
	cv::rectangle(circle_font, maxRect, cv::Scalar(255));
	//cv::imshow("largest region", circle_font);
	//waitKey();
	//cout << maxRect << endl;
	return maxRect;		//return the corase  X,Y  location of circle
						//���������С�ƶϳ��������ֵ�ƬԪ����ͬ�Ŀ����¸߶ȱ�ɶ���������ƬԪ�����а������֡�

}



//������ת��Ϊʵ�ʾ���,Ŀǰ�ڲ�Ϊ269.5mm,��������ص�Ԫ��СΪ:
float pixelToLength(int pixel_num, float distance) {
	float pixel_scale = 0.000111;		//pixel scale in meter  TODO:calibration
	float focus_lenght = 0.2695;		//fx=fy=269.5mm
	float real_length = (pixel_num*pixel_scale)*distance / focus_lenght;  //nx/f=X/z  from camera model

	return real_length;
}

#endif // !IMAGE_PROCESS_H








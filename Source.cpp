#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <Windows.h>
#include <iostream>
#include <vector>
#include <string> 

using namespace std;
using namespace cv;

const int h = 280;
const int w = 1235;
int height = h, width = w, srcheight= h, srcwidth= w;

Mat hwnd2mat(HWND hwnd)	//capture frame
{
	HDC hwindowDC, hwindowCompatibleDC;

	HBITMAP hbwindow;
	Mat src,img;
	BITMAPINFOHEADER  bi;

	hwindowDC = GetDC(NULL);
	hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
	SetStretchBltMode(hwindowCompatibleDC, COLORONCOLOR);	

	src.create(height, width, CV_8UC4);

	// create a bitmap
	hbwindow = CreateCompatibleBitmap(hwindowDC, width, height);
	bi.biSize = sizeof(BITMAPINFOHEADER); 
	bi.biWidth = width;
	bi.biHeight = -height;  //this is the line that makes it draw upside down or not
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	// use the previously created device context with the bitmap
	SelectObject(hwindowCompatibleDC, hbwindow);
	// copy from the window device context to the bitmap device context
	StretchBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, 0, 0, srcwidth, srcheight, SRCCOPY); //change SRCCOPY to NOTSRCCOPY for wacky colors !
	GetDIBits(hwindowCompatibleDC, hbwindow, 0, height, src.data, (BITMAPINFO *)&bi, DIB_RGB_COLORS);  //copy from hwindowCompatibleDC to hbwindow

																									   // avoid memory leak
	DeleteObject(hbwindow);
	DeleteDC(hwindowCompatibleDC);
	ReleaseDC(hwnd, hwindowDC);

	img = src(Rect(685, 100, 550, 180)); //only game
	
	return img;
}

void pressUpper() {
	INPUT upper = { 0 };
	upper.type = INPUT_KEYBOARD;
	upper.ki.wVk = VK_UP;
	SendInput(1, &upper, sizeof(INPUT)); // Send KeyDown
	upper.ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(1, &upper, sizeof(INPUT)); // Send KeyUp
}

void pressDown() {
	INPUT upper = { 0 };
	upper.type = INPUT_KEYBOARD;
	upper.ki.wVk = VK_DOWN;
	SendInput(1, &upper, sizeof(INPUT)); // Send KeyDown	
}

void relasseDown() {
	INPUT upper = { 0 };
	upper.type = INPUT_KEYBOARD;
	upper.ki.wVk = VK_DOWN;
	upper.ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(1, &upper, sizeof(INPUT)); // Send KeyUp
}

int main()
{
	HWND hwndDesktop = GetDesktopWindow();
	Mat img2 = hwnd2mat(hwndDesktop);

	uchar lastColor = 247;
	bool press = false, fast = false, bird = false;

	Mat gray1, gray2;
	Mat absOut, trOut, blurOut, noGrass,finalIMG,screen;
	Mat kernel(20, 20, CV_8U);
	Mat blackStrip(60, 550, CV_8U, Scalar(0, 0, 0));

	int value1 = 110;
	int value2 = 90;
	//createTrackbar("y", "original",&value1, 255);
	//createTrackbar("x", "original", &value2, 255);

	namedWindow("original", WINDOW_AUTOSIZE);
	namedWindow("final", WINDOW_AUTOSIZE);
	namedWindow("finalimg", WINDOW_AUTOSIZE);
	namedWindow("test", WINDOW_AUTOSIZE);

	while (true)
	{			
		Mat img1 = hwnd2mat(hwndDesktop);
		imshow("test", img1);

		int tvalue1 = value1;
		int tvalue2 = value2;		

		cvtColor(img1, gray1, COLOR_BGR2GRAY);
		cvtColor(img2, gray2, COLOR_BGR2GRAY);

		absdiff(gray1, gray2, absOut);
		threshold(absOut, trOut, 140, 255, THRESH_BINARY);

		noGrass = trOut(Rect(0, 0, 550, 145));
		blackStrip.copyTo(noGrass(Rect(0, 0, blackStrip.cols, blackStrip.rows)));

		morphologyEx(noGrass, noGrass, MORPH_CLOSE, kernel);
		medianBlur(noGrass, finalIMG, 5);

		Mat con = finalIMG.clone();
		vector<vector<Point>> contours;
		findContours(con, contours, RETR_TREE, CHAIN_APPROX_NONE);
		vector<vector<Point> > contours_poly(contours.size());//
		vector<Rect> boundRect(contours.size());//	

		screen = img1.clone();

		Vec3b color = img1.at<Vec3b>(Point(300, 170));	//check if isn't night, if is jump		

		//cout << color << endl;

		if (color.val[0] < 200 && color.val[0] > 40) {
			pressUpper();
			lastColor = color.val[0];
		}
			


		if (contours.size() > 0) {		//tuto je problem

			for (int i = 0; i < contours.size(); i++)
			{
				approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true);//
				boundRect[i] = boundingRect(Mat(contours_poly[i]));//
			}


			for (int i = 0; i < contours.size(); i++)
			{
				if (boundRect[i].tl().y > 100) {
					boundRect[i].height = 30;
				}			

				boundRect[i].x = boundRect[i].tl().x - 5;
				boundRect[i].width = boundRect[i].width - 10;

				rectangle(screen, boundRect[i].tl(), boundRect[i].br(), Scalar(255, 0, 255), 2, 8, 0);
				rectangle(finalIMG, boundRect[i].tl(), boundRect[i].br(), Scalar(255, 0, 255), 2, 8, 0);
			}

		}
		
		if (boundRect.size() > 0)	//check nearby enemy
		{
			rectangle(screen, boundRect[boundRect.size() - 1].tl(), boundRect[boundRect.size() - 1].br(), Scalar(255, 0, 0), 2, 8, 0); //firs enemy change collor

			//for enemy which are less than 110p

			for (int i = 0; i < boundRect.size(); i++) {	
				if (boundRect[i].tl().x < value2 && boundRect[i].tl().x > -4 && boundRect[i].tl().y > tvalue1) {
					pressUpper();
				}
			}

			//for enemy which are more than 110p

			for (int i = 0; i < boundRect.size(); i++) {
				if (boundRect[i].tl().x < value2  && boundRect[i].tl().x > 20 && boundRect[i].tl().y < tvalue1) {					
					pressDown();
					press = true;
					bird = true;
				}
			}

			if (press == true && boundRect[boundRect.size() - 1].br().x + 60 < value2) {
				relasseDown();
				press = false;
				bird = false;
			}
			
			//cout << boundRect[boundRect.size() - 1].area() << endl;

			//fast down
			if (bird == false) {

				if (boundRect[boundRect.size() - 1].area() < 450 && boundRect[boundRect.size() - 1].tl().x < 80 && boundRect[boundRect.size() - 1].area() > 270 && fast == false) {
					pressDown();
					fast = true;
				}

				else if (boundRect[boundRect.size() - 1].area() > 1000 && fast == true && boundRect[boundRect.size() - 1].tl().x < 80 ) {
					relasseDown();
					fast = false;
				}
				
			}
		}
		
		imshow("original", screen);
		imshow("final", noGrass);
		imshow("finalimg", finalIMG);
		waitKey(1);	
		
		img2 = img1;
	}
}
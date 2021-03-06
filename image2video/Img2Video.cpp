// Img2Video.cpp : read a sequence of images, output a video with dirty lens marked (in green)
// Author: Zhiyuan Wang
// zywang@u.northwestern.edu
// Feb, 2, 2014
//#include <string.h>
#include <iostream>
#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp>
using namespace cv;
using namespace std;

/* calculate gradient image */
Mat gradient(Mat src, int color);
/* get dirty lens mask from first few images in a sequence*/
Mat Mask(char*, int, int);
/* the following 3 function still need debug and revision*/
/* an implementation of algorithm in J. Gu et al. */
Mat SmoothBG();
/* obtain median value of a Mat */
uchar getMedian(Mat src);
/* get pixelwise max - min image for a sequence of images */
void Maxmin();

/******************************************************************************************
 * Note: input images can only be named by sequential number, as "%d.jpg".
 * no input: Run with default setting. Read image sequence under "C://sample_drive//sample_drive//cam_2//", 
             begin from 393408606.jpg, end with 393413167.jpg
 * one input: directory changed as the input
 * three input: directory path, begin image number, end image number
 */
int main(int argc, char* argv[])
{
	// test for J. Gu et al. method
	/*
	Mat est = SmoothBG();
	imshow("", est);
	waitKey(0);
	return 0;
	*/
	int begin = 393408606;
	int end = 393413167;
	int i;
	int len = 4500;
	char filepath[100];
	char outpath[100];
	char* dirpath = "C://sample_drive//sample_drive//cam_5//";
	
	if(argc == 2)
	{
		dirpath = argv[1];
		//cout << argv[1] << endl;
	}
	else if(argc == 4)
	{
		dirpath = argv[1];
		begin = atoi(argv[2]);
		end = atoi(argv[3]);
	}
	else if(argc == 5)
	{
		dirpath = argv[1];
		begin = atoi(argv[2]);
		end = atoi(argv[3]);
		len = atoi(argv[4]);
	}
	// get the mask from first few images in the sequence
	Mat mask = Mask(dirpath, begin, len);
	cout << "succ" << endl;

	sprintf_s(filepath, "%s%d.jpg", dirpath, begin);
	Mat frame = imread(filepath);
	sprintf_s(outpath, "%scam_grad.avi", dirpath);
	VideoWriter Output(outpath, CV_FOURCC('D', 'I', 'V', 'X'), 5, frame.size());
	namedWindow("frame", WINDOW_NORMAL);
    for(i = begin; i <= end; i++)
    {
		sprintf_s(filepath, "%s%d.jpg", dirpath, i);
		//cout << filepath;
		frame = imread(filepath);
		if(frame.empty())
		{
			continue;
		}
		frame.setTo(Scalar(0, 255, 0), mask);

		Output << frame;
		imshow("frame", frame);
		//imwrite("result.jpg", frame);
        if(waitKey(1)==27)
            exit(0);
    }
	destroyWindow("frame");
    return 0;
}

Mat Mask(char* dirpath, int begin, int len)
{
	//int begin = 393408606;
	char filepath[100];
	int n;
	sprintf_s(filepath, "%s%d.jpg", dirpath, begin);
	Mat frame = imread(filepath);
	//Mat grad;
	Mat frame32f;
	//int len = 4500;
	Mat mask = Mat::zeros(frame.rows,frame.cols,CV_32FC1);
	for(n = 0; n < len; n++)
	{
		sprintf_s(filepath, "%s%d.jpg", dirpath, n + begin);
		frame = imread(filepath,CV_LOAD_IMAGE_COLOR);
		if(frame.empty())
		{
			continue;
		}
		frame = gradient(frame, 1);
        frame.convertTo(frame32f,CV_32FC1);
		mask +=frame32f;
		//frame.release();
	}
	mask *= (1.0/len);
	mask.convertTo(mask, CV_8UC1);
	// this is a trick
	// when normalization, do not consider the possible noisy point with 0 intensity
	Mat mask_mask = Mat::ones(mask.rows, mask.cols, CV_8UC1);
	uchar* fline = mask_mask.ptr<uchar>(0);
	uchar* lline = mask_mask.ptr<uchar>(mask_mask.rows-1);
	for(int j = 0; j < mask_mask.cols; j++)
	{
		fline[j] = lline[j] = 0;
	}
	for(int j = 0; j < mask_mask.rows; j++)
	{
		uchar* line = mask_mask.ptr<uchar>(j);
		line[0] = line[mask_mask.cols-1] = 0;
	}
	for(int i = 0; i < mask.rows; i++)
	{
		uchar* mline = mask.ptr<uchar>(i);
		uchar* mmline = mask_mask.ptr<uchar>(i);
		for(int j = 0; j < mask.cols; j++)
		{
			uchar* mp = &mline[j];
			uchar* mmp = &mmline[j];
			if (*mp == 0)
				*mmp = 0;
		}
	}
	normalize(mask,mask,0,255,CV_MINMAX, -1, mask_mask);
	namedWindow("average", WINDOW_NORMAL);
	imshow("average", mask);
	imwrite("average.jpg", mask);
	waitKey(0);
	Mat element = getStructuringElement(MORPH_RECT, Size(3,3));
	// do morphology opening 4 iteration
	morphologyEx(mask, mask, MORPH_OPEN, element, Point(-1,-1), 4);
	
	normalize(mask,mask,0,255,CV_MINMAX, -1, mask_mask);
	/*
	double maxval, minval, factor;
	minMaxLoc(mask, &minval, &maxval, NULL, NULL, mask_mask);
	mask -= minval;
	factor = (maxval - minval)/255;
	cout << minval << '\t' << maxval << '\t' << factor << endl;
	mask /= factor;
	*/
	namedWindow("opening", WINDOW_NORMAL);
	imshow("opening", mask);
	imwrite("opening.jpg", mask);
	waitKey(0);
	threshold(mask, mask, 10, 255, THRESH_BINARY_INV);
	
	namedWindow("frame", WINDOW_NORMAL);
	imwrite("Mask.jpg", mask);
	imshow("frame", mask);
	waitKey(0);
	return mask;
}

Mat gradient(Mat src, int color)
{
	 int scale = 1;
     int delta = 0;
     int ddepth = CV_16S;
	 Mat src_gray, grad;
	 GaussianBlur( src, src, Size(3,3), 0, 0, BORDER_DEFAULT );
	 /// Convert it to gray
	 if(src.channels() > 1)
		cvtColor( src, src_gray, CV_RGB2GRAY );
	 else
		 src_gray = src;
	 Mat grad_x, grad_y;
	 Mat abs_grad_x, abs_grad_y;

	 /// Gradient X
     //Scharr( src_gray, grad_x, ddepth, 1, 0, scale, delta, BORDER_DEFAULT );
     Sobel( src_gray, grad_x, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT );
     convertScaleAbs( grad_x, abs_grad_x );

    /// Gradient Y
    //Scharr( src_gray, grad_y, ddepth, 0, 1, scale, delta, BORDER_DEFAULT );
    Sobel( src_gray, grad_y, ddepth, 0, 1, 3, scale, delta, BORDER_DEFAULT );
    convertScaleAbs( grad_y, abs_grad_y );

    /// Total Gradient (approximate)
    addWeighted( abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad );

	//threshold(grad, grad, 10, 255, THRESH_BINARY);	

	return grad;
}
/*
Mat SmoothBG()
{
	Mat src = Mask();
	uchar median = getMedian(src);
	int i, j, k, ii, ji;
	Mat inlier_mask = Mat::zeros(src.rows, src.cols, CV_8UC1);
	Mat est = src.clone();
	int inlier_count, outlier_count;
	do{
		inlier_count = 0;
		outlier_count = 0;
		char* x_mat = new char[inlier_count*9];
		char* y_mat = new char[inlier_count];
		Mat X(inlier_count, 9, CV_8UC1, x_mat, 9*sizeof(char));
		Mat Y(inlier_count, 1, CV_8UC1, y_mat, sizeof(char));
		int flag = 1;
		for(i = 1; i < src.rows - 1; i++)
		{
			uchar* Mi = src.ptr<uchar>(i);
			uchar* maskl = inlier_mask.ptr<uchar>(i);
			for(j = 1; j< src.cols - 1; j++)
			{
				uchar* pixel = &Mi[j];
				uchar* maskp = &maskl[j];
				if ((*pixel > median && flag) || *maskp > 0)
				{
					inlier_count++;
					*maskp = 255;
					k = 0;
					// TO DO Local Normalize
					// build a line in X for one inliear point
					for(ii = -1; ii <= 1; ii++)
					{
						for(ji = -1; ji <= 1; ji++)
						{
							uchar* temp = src.ptr<uchar>(i + ii);
							uchar* pix_t = &temp[j + ji];
							x_mat[(inlier_count - 1) * 9 + k] = *pix_t;
							k++;
						}
					}
					y_mat[inlier_count - 1] = *pixel;
				}
			}
		}
		// least square
		Mat X_T, a;
		cv::transpose(X, X_T);
		a = X.mul(X_T).inv().mul(X_T).mul(Y);
		uchar* mat_a = a.data; 
		// calculate estimated image and diff
		for(i = 1; i < src.rows - 1; i++)
		{
			uchar* Mi = src.ptr<uchar>(i);
			uchar* maskl = inlier_mask.ptr<uchar>(i);
			uchar* estl = est.ptr<uchar>(i);
			for(j = 1; j< src.cols - 1; j++)
			{
				uchar* pixel = &Mi[j];
				uchar* maskp = &maskl[j];
				uchar* estp = &estl[j];
					
					k = 0;
					// set pixel in estimate image
					for(ii = -1; ii <= 1; ii++)
					{
						for(ji = -1; ji <= 1; ji++)
						{
							uchar* temp = src.ptr<uchar>(i + ii);
							uchar* pix_t = &temp[j + ji];
							*estp += *pix_t * mat_a[k];
							k++;
						}
					}
					if( *pixel - *estp > 0.1 * *pixel || *estp - *pixel > 0.1 * *pixel)
					{
						*maskl = 0;
						outlier_count++;
					}
			}
		}
		flag = 0;
	}while (outlier_count > 0);
	
	return est;
}
*/
uchar getMedian(Mat src)
{
	Mat hist;
	int channels[] = {0};
	int histsize[] = {256};
	float range[] = {0, 255};
	const float* ranges[] = {range};
	calcHist( &src, 1, channels, Mat(), hist, 1, histsize, ranges, true, false);

	uchar * h = hist.data;    
	int total = 0, sum = 0, bin = 0;
	for(bin = 0; bin < 256; bin++)
	{
		total += h[bin];
	}
	bin = 0;
	while((sum < total / 2) && (bin <= 255))
	{
		sum += h[bin];
		bin++;
	}
	return bin;
}

void Maxmin()
{
	int begin = 393408606;
	char filepath[100];
	int n, i, j;
	sprintf_s(filepath, "C://sample_drive//sample_drive//cam_3//%d.jpg", begin);
	Mat frame = imread(filepath, CV_LOAD_IMAGE_GRAYSCALE);
	int len = 400;
	Mat max_im = Mat::zeros(frame.rows,frame.cols,CV_8UC1);
	Mat min_im(frame.rows, frame.cols, CV_8UC1, Scalar::all(255));
	Mat mask = Mat::zeros(frame.rows,frame.cols,CV_8UC1);
	uchar min = 255;
	uchar max = 0;
	for(n = 0; n < len; n++)
	{
		sprintf_s(filepath, "C://sample_drive//sample_drive//cam_3//%d.jpg", n + begin);
		frame = imread(filepath,CV_LOAD_IMAGE_GRAYSCALE);
		for(i = 0; i < frame.rows; i++)
		{
			uchar* Mi = frame.ptr<uchar>(i);
			uchar* Maxi = max_im.ptr<uchar>(i);
			uchar* Mini = min_im.ptr<uchar>(i);
			for(j = 0; j< frame.cols; j++)
			{
				uchar* pixel = &Mi[j];
				uchar* max_pixel = &Maxi[j];
				uchar* min_pixel = &Mini[j];
				if(*pixel > *max_pixel)
				{
					*max_pixel = *pixel;
				}
				if(*pixel < *min_pixel)
				{
					*min_pixel = *pixel;
				}
			}
		}
	}

	subtract(max_im, min_im, mask);
	threshold(mask, mask, 10, 255, THRESH_BINARY);

	namedWindow("frame", WINDOW_NORMAL);
	//imwrite("Mask.jpg", mask);
	imshow("frame", mask);
	waitKey(0);
	return;
}



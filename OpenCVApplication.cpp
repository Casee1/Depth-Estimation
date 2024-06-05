#include "stdafx.h"
#include "common.h"
#include <opencv2/core/utils/logger.hpp>
#include <iostream>

wchar_t* projectPath;
using namespace std;
using namespace cv;

bool inside(Mat src, int i, int j)
{
    return (i >= 0 && i < src.rows && j >= 0 && j < src.cols);
}

Mat_<uchar> censusTransform(Mat_<uchar> src, int w)
{
    int height = src.rows;
    int width = src.cols;
    Mat_<uchar> dst(height, width);

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int census_code = 0;

            for (int u = -w / 2; u <= w / 2; u++)
            {
                for (int v = -w / 2; v <= w / 2; v++)
                {
                    if (u == 0 && v == 0)
                    {
                        continue;
                    }

                    if (inside(src, i + u, j + v))
                    {
                        census_code *= 2;
                        if (src(i, j) < src(i + u, j + v))
                        {
                            census_code += 1;
                        }
                    }
                }
            }
            dst(i, j) = census_code;
        }
    }
    return dst;
}

Mat_<int> censusTransform1(Mat_<uchar> src, int w)
{
    int height = src.rows;
    int width = src.cols;
    //int** dst = (int*)malloc(height * sizeof(int));

   /* for (int i = 0; i < height; i++)
    {
        dst[i] = (int*)malloc(width * sizeof(int));
    }*/

    Mat_<int> dst(height, width);

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int census_code = 0;

            for (int u = -w / 2; u <= w / 2; u++)
            {
                for (int v = -w / 2; v <= w / 2; v++)
                {
                    if ((i + u) == 0 && (j + v) == 0)
                    {
                        continue;
                    }
                    else
                    {
                        if (inside(src, i + u, j + v))
                        {
                            census_code *= 2;
                            if (src(i, j) < src(i + u, j + v))
                            {
                                census_code += 1;
                            }
                        }
                    }

                }
            }
            dst(i, j) = census_code;
        }
    }
    return dst;
}

int hammingDistanceWegner(int a, int b)
{
    int differences = 0;
    int val = a ^ b;
    while (val > 0)
    {
        val &= (val - 1);
        differences++;
    }
    return differences;
}


Mat_<uchar> disparity(Mat_<int> left_census, Mat_<int> right_census, int height, int width, int w) {
    Mat_<uchar> dst(height, width);
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int min_hamming = INT_MAX;
            int best_disparity = 0;

            for (int d = 0; d < 50; d++)
            {
                if (j - d >= 0)
                {
                    int hamming_distance = 0;
                    for (int u = -w / 2; u <= w / 2; u++)
                    {
                        for (int v = -w / 2; v <= w / 2; v++)
                        {
                            if (inside(left_census, i + u, j + v) && inside(right_census, i + u, j + v - d))
                            {
                                hamming_distance += hammingDistanceWegner(left_census(i + u, j + v), right_census(i + u, j + v - d));
                            }
                        }
                    }
                    if (hamming_distance < min_hamming)
                    {
                        min_hamming = hamming_distance;
                        best_disparity = d;
                    }
                }
            }

            dst(i, j) = best_disparity * 4;
        }
    }
    return dst;
}

void calc_error(Mat_<uchar> disp, Mat_<uchar> disp1, int th)
{
    int height = disp.rows;
    int width = disp.cols;
    int differences = 0;
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            if (abs(disp1(i, j) - disp(i, j)) > th)
            {
                differences++;
            }
        }
    }

    float error = ((float)differences / (height * width)) * 100;

    cout << error << " ";
}

int main() {
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_FATAL);

    //Mat_<uchar> left = imread("Images/proiect/cones/im2.png", IMREAD_GRAYSCALE);
    //Mat_<uchar> right = imread("Images/proiect/cones/im6.png", IMREAD_GRAYSCALE);

    Mat_<uchar> left = imread("Images/proiect/teddy2.png", IMREAD_GRAYSCALE);
    Mat_<uchar> right = imread("Images/proiect/teddy1.png", IMREAD_GRAYSCALE);

    Mat_<int> left_census = censusTransform1(left, 5);
    Mat_<int> right_census = censusTransform1(right, 5);

    Mat_<uchar> dst = disparity(left_census, right_census, left_census.rows, left_census.cols, 3);
    Mat_<uchar> disp = imread("Images/proiect/teddy/disp2.png", IMREAD_GRAYSCALE);
    //Mat_<uchar> disp = imread("Images/proiect/cones/disp2.png", IMREAD_GRAYSCALE);

    imshow("left", left);
    imshow("right", right);
    
    calc_error(disp, dst, 3);
    for (int i = 0; i < dst.rows; i++)
    {
        for (int j = 0; j < dst.cols; j++)
        {
            dst(i, j) = dst(i, j) * 255 / 50;
        }
    }
    imshow("dst", dst);
    waitKey(0);
    waitKey(0);
    waitKey(0);

    return 0;
}
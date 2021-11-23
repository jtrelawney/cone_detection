#include <iostream>
#include "opencv2/opencv.hpp"
using namespace std;
using namespace cv;
using namespace cv::ml;

void ex1(int K=1){
	Ptr<ml::KNearest>  knn(ml::KNearest::create());
	Mat_<float> trainFeatures(6,4);
	trainFeatures << 2,2,2,2,
		         3,3,3,3,
		         4,4,4,4,
		         5,5,5,5,
		         6,6,6,6,
		         7,7,7,7;

	Mat_<int> trainLabels(1,6);
	trainLabels << 2,3,4,5,6,7;

	cout << "train data" << endl;
	cout << trainFeatures << endl;

	cout << "label data" << endl;
	cout << trainLabels << endl;

	knn->train(trainFeatures, ml::ROW_SAMPLE, trainLabels);

	Mat_<float> testFeature(1,4);
	testFeature<< 3,3,3,3;

	cout << "test data" << endl;
	cout << testFeature << endl;

	Mat response, dist;
	knn->findNearest(testFeature, K, noArray(), response, dist);

	cout << "prediction" << endl;
	cout << response << endl;

	cout << "dist" << endl;
	cout << dist << endl;
};

int main(){
	ex1();
  return 0;
};

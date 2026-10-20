//---------------------------------------------------------------------------

#pragma hdrstop

#include <tchar.h>
//---------------------------------------------------------------------------

#pragma argsused
#include "opencv2/opencv_modules.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/ml/ml.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/opencv.hpp"
#include <windows.h>
#include <stdio.h>

using namespace std;
using namespace cv;
using namespace cv::ml;
//----------------------------------------------------------------------
// Получаем список файлов из директории
// Применение:
// string ImagesDir=tmp+"C:\\Images\\*.jpg";
// vector<string> files=listFilesInDirectory(ImagesDir);
//----------------------------------------------------------------------
vector<String> listFilesInDirectory(String directoryName){
  WIN32_FIND_DATA FindFileData;
  HANDLE hFind = FindFirstFile(directoryName.c_str(), &FindFileData);
  vector<String> listFileNames;
  listFileNames.push_back(FindFileData.cFileName);

  while (FindNextFile(hFind, &FindFileData))
	listFileNames.push_back(FindFileData.cFileName);

  return listFileNames;
}
//----------------------------------------------------------------------
inline TermCriteria TC(int iters, double eps)
{
	return TermCriteria(TermCriteria::MAX_ITER + (eps > 0 ? TermCriteria::EPS : 0), iters, eps);
}

int main(){
// Высота и ширина символа
int digit_w=11;
int digit_h=14;
// Размерность вектора обучающей выборки
const int image_area = digit_w*digit_h;

// Директория в которой лежат поддиректории с картинками
String DataDir="..\\data\\";

// Список названий поддиректорий с примерами
vector<String> DirNames;
DirNames.push_back("0");
DirNames.push_back("1");
DirNames.push_back("2");
DirNames.push_back("3");

// Формируем список путей к изображениям и заполняем метки классов
int num_img=0;
vector<String> files_plain_list;
vector<int> files_label_list;
for (int i=0;i<DirNames.size();++i) {
  DirNames[i]=DataDir+DirNames[i]+"\\";
  vector<String> files_tmp(listFilesInDirectory(DirNames[i]+"*.png"));
  num_img+=files_tmp.size();
  files_plain_list.insert(files_plain_list.end(), files_tmp.begin(), files_tmp.end());

  for(int j=0;j<files_tmp.size();++j)
    files_label_list.push_back(i);
}

// Матрица обучающих векторов
cv::Mat training_mat(num_img, image_area, CV_32FC1);    //train_data
// Матрица меток классов
cv::Mat labels;
labels=Mat::zeros(num_img, DirNames.size(), CV_32FC1);  //train_responses
// Временная матрица для преобразований
cv::Mat tmp_img;

int count = 0;
String Dir;
vector<String>::const_iterator i;
for (i = files_plain_list.begin(); i != files_plain_list.end(); ++i) {
 // Узнали метку класса
 int class_label=files_label_list[i-files_plain_list.begin()];
 // Узнали директорию, где лежат векторы этого класса
 Dir=DirNames[class_label];
 // Номер метки для каждого входного вектора
 labels.at< float >(count, class_label) = 1;
 // Выведем что получилось (имена файлов с директориями)
 String rrr=Dir + *i;
 cout << rrr.c_str() << endl;
 // Считаем изображение
 tmp_img = imread( Dir +*i, 0 );
 // Ресайзим под заданный размер входного вектора
 resize( tmp_img, tmp_img, Size(digit_w,digit_h) );
 // Вытягиваем в одну строку
 tmp_img = tmp_img.reshape( 1, 1 );
 // Переводим в тип float
 tmp_img.convertTo( training_mat.row(count), CV_32FC1 );
 // Следующий ...
 ++count;
}

// Архитектура сети. Создадим трехслойную сеть.
Mat layer_sizes1( 1, 3, CV_32FC1 );
// По размеру входного вектора
layer_sizes1.at<int>(0) = image_area;
// Ну, так мне захотелось
layer_sizes1.at<int>(1) = image_area/10.0;
// По количеству классов
layer_sizes1.at<int>(2) = DirNames.size();

// Все векторы равнозначны, ущербных данных нет
int layer_sz[] = { training_mat.cols, 100, 100, DirNames.size() };
int nlayers = (int)(sizeof(layer_sz)/sizeof(layer_sz[0]));
Mat weights( 1, nlayers, CV_32S, layer_sz);// Scalar::all(1) );
// Создаем и обучаем сеть
#if 1
		int method = ANN_MLP::BACKPROP;
		double method_param = 0.001;
		int max_iter = 300;
#else
		int method = ANN_MLP::RPROP;
		double method_param = 0.1;
		int max_iter = 1000;
#endif
Ptr<TrainData> tdata = TrainData::create(training_mat, ROW_SAMPLE, labels);

Ptr<ANN_MLP> ann;
ann = ANN_MLP::create();
ann->setLayerSizes(weights);
ann->setActivationFunction(ANN_MLP::SIGMOID_SYM, 1, 1);
ann->setTermCriteria(TC(max_iter,0));
ann->setTrainMethod(method, method_param);
ann->train(tdata);
// Сохраняем сеть
ann->save("recog.xml");

// Тестируем сеть
Mat outputs;
Mat image;
image = imread(DataDir+"x.png", 0 );
resize( image, image, Size(digit_w,digit_h) );
image = image.reshape( 1, 1 );
image.convertTo(image, CV_32FC1 );
//imshow("Result", image);

//ann->predict(training_mat, outputs);
ann->predict(image, outputs);

float * p = (float *) outputs.data;

char ccc[50];
int j;
for( int i=0; i<outputs.rows*outputs.cols; i+=outputs.cols) {
  ccc[0]=0;
  for( j=0; j<outputs.cols; j++) {
	cout << p[i+j];
	cout << "\t";
  }
  cout << endl;
  if ((i/4)%2 > 0 ) cout << endl;
}
// Ждем кнопку ...
waitKey(0);

return 1;
}


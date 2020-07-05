#include <cmath>
#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/framework/formats/rect.pb.h"
#include <fstream>
#include <vector>
#include "mediapipe/framework/packet.h"
#include "mediapipe/framework/port/status.h"
#include "mediapipe/util/resource_util.h"
#include <sstream>
#include "mediapipe/util/android/file/base/file.h"
#include "mediapipe/util/android/file/base/helpers.h"
#include <stdio.h>
#include <numeric>      // std::iota
#include <algorithm>    // std::sort, std::stable_sort
#include "mediapipe/framework/formats/gesture.pb.h"

namespace mediapipe
{

namespace
{
constexpr char normRectTag[] = "NORM_RECT";
constexpr char normalizedLandmarkListTag[] = "NORM_LANDMARKS";
constexpr char recognizedHandGestureTag[] = "RECOGNIZED_HAND_GESTURE";
} // namespace

// struct
typedef struct{
    float x;
    float y;
}point;

typedef struct{
    std::vector<point> points;
}keypoints;

typedef struct{
    std::vector<keypoints> samples;
}sampleset;

class HandGestureRecognitionCalculator : public CalculatorBase
{
public:
    static ::mediapipe::Status GetContract(CalculatorContract *cc);

    ::mediapipe::Status Open(CalculatorContext *cc) override;
    
    ::mediapipe::Status Process(CalculatorContext *cc) override;
    
    float distance_between_point(point* p1, point* p2);
    
    float distance_between_keypoint(keypoints* k1, keypoints* k2);
    
    void calculate_distance(keypoints* k1);
    
    void load_dataset();
    
    int query(keypoints* k1);
    
    std::vector<size_t> sort_indexes(const std::vector<float> &v);

private:
    int top_k = 3;
    
    sampleset datas; // dataset
    

    float floatdatas[30][21][2];
    
    std::vector<int> class_id;
    
    std::string s;//variable used to help debug
    
    std::vector<float> _distance;
};

// register custom calculator to framework
REGISTER_CALCULATOR(HandGestureRecognitionCalculator);

// check input and output
::mediapipe::Status HandGestureRecognitionCalculator::GetContract(
    CalculatorContract *cc)
{
    RET_CHECK(cc->Inputs().HasTag(normalizedLandmarkListTag));
    cc->Inputs().Tag(normalizedLandmarkListTag).Set<mediapipe::NormalizedLandmarkList>();

    RET_CHECK(cc->Inputs().HasTag(normRectTag));
    cc->Inputs().Tag(normRectTag).Set<NormalizedRect>();

    RET_CHECK(cc->Outputs().HasTag(recognizedHandGestureTag));
    // cc->Outputs().Tag(recognizedHandGestureTag).Set<std::string>();
    cc->Outputs().Tag(recognizedHandGestureTag).Set<Gesture>();
    // cc->Outputs().Tag(recognizedHandGestureTag).Set<std::vector<std::string>>();
    return ::mediapipe::OkStatus();
}

::mediapipe::Status HandGestureRecognitionCalculator::Open(
    CalculatorContext *cc)
{
    cc->SetOffset(TimestampDiff(0));
    //load from binary file
    std::string string_path = "mediapipe/models/kpts.bin"; //file path
    ASSIGN_OR_RETURN(string_path,
                     PathToResourceAsFile(string_path));
    // hard code
    //TODO: use mediapipe buildin function to read binary file.
    // char path[] = "/data/user/0/com.google.mediapipe.apps.handtrackinggpu/cache/mediapipe_asset_cache/kpts.bin";
    char* path = (char*)string_path.c_str();
    FILE* f = fopen(path, "rb");
    if (f){
        fread(floatdatas,1,/*number of byte*/30*21*2*4,f);
        fclose(f);
    }

    //convert dataset to vector format
    keypoints kpt;
    point single_sample;
    for (int i = 0; i < 30; i++){
        for (int j = 0; j < 21; j++){
            single_sample.x = floatdatas[i][j][0];
            single_sample.y = floatdatas[i][j][1];
            kpt.points.push_back(single_sample);
        }
        datas.samples.push_back(kpt);
        kpt.points.clear();
    }
    //init class id
    for (int i = 0; i < 30; i++){
        if (i < 10){
            class_id.push_back(0); //paper
        }
        else if (i < 20){
            class_id.push_back(1); //rock
        }
        else{
            class_id.push_back(2); //Scissors
        }
    }
    return ::mediapipe::OkStatus();
}

::mediapipe::Status HandGestureRecognitionCalculator::Process(
    CalculatorContext *cc)
{   // clear _distance vector
    _distance.clear();
    std::string *recognized_hand_gesture;

    // hand rectangle
    const auto rect = &(cc->Inputs().Tag(normRectTag).Get<NormalizedRect>());
    float width = rect->width();
    float height = rect->height();

    //default
    Gesture *gesture = new Gesture;
    recognized_hand_gesture = new std::string("___");
    // std::vector<std::string>* result = new std::vector<std::string>;
    // result->push_back("test");
    //check if exist hand or not
    if (width < 0.01 || height < 0.01)
    {
        // LOG(INFO) << "No Hand Detected";
        recognized_hand_gesture = new std::string("___");
        gesture->set_ges("___");
        // result->push_back("___");
        cc->Outputs()
            .Tag(recognizedHandGestureTag)
            .Add(gesture, cc->InputTimestamp());
        return ::mediapipe::OkStatus();
    }

    const auto &landmarkList = cc->Inputs()
                                   .Tag(normalizedLandmarkListTag)
                                   .Get<mediapipe::NormalizedLandmarkList>();
    RET_CHECK_GT(landmarkList.landmark_size(), 0) << "Input landmark vector is empty.";

    //TODO. Recognition with threshold
    float threshold = 0.01;

    //convert input landmarklist to keypoints struct
    keypoints kpt;
    point pt;
    float xx;
    float yy;
    float xmin = 999999.0;
    float ymin = 999999.0;
    //Minimum coordinate search. Transform coordinate system
    for(int point_id=0; point_id<21;point_id++){
        if(landmarkList.landmark(point_id).x()<xmin){
            xmin = landmarkList.landmark(point_id).x();
        }
        if(landmarkList.landmark(point_id).y()<ymin){
            ymin = landmarkList.landmark(point_id).y();
        }
    }
    for (int point_id=0; point_id<21;point_id++){
        xx = landmarkList.landmark(point_id).x() - xmin;
        yy = landmarkList.landmark(point_id).y() - ymin;
        pt.x = xx;
        pt.y = yy;
        kpt.points.push_back(pt);
    }
    int sample_id = query(&kpt);
    if (sample_id==0){
        recognized_hand_gesture = new std::string("paper");
        gesture->set_ges("paper");
        // result->push_back("paper");
    }
    if (sample_id==1){
        recognized_hand_gesture = new std::string("rock");
        gesture->set_ges("rock");
        // result->push_back("rock");
    }
    if (sample_id==2){
        recognized_hand_gesture = new std::string("Scissors");
        gesture->set_ges("Scissors");
        // result->push_back("scissor");
    }

    cc->Outputs()
        .Tag(recognizedHandGestureTag)
        .Add(gesture, cc->InputTimestamp());
    // cc->Outputs()
    //     .Tag(recognizedHandGestureTag)
    //     .Add(result, cc->InputTimestamp());
    return ::mediapipe::OkStatus();
} // namespace mediapipe

float HandGestureRecognitionCalculator::distance_between_point(point* p1, point* p2){
    float _d = std::pow(p1->x - p2->x, 2) + std::pow(p1->y - p2->y, 2);
    return std::sqrt(_d);
}

float HandGestureRecognitionCalculator::distance_between_keypoint(keypoints* k1,keypoints* k2){
    keypoints* _k1 = k1;
    keypoints* _k2 = k2;
    point* p1;
    point* p2;
    float _d = 0;
    int n = k1->points.size();
    for (int i=0; i< n;i++){
        p1 = &(_k1->points[i]); //point i of keypoints1
        p2 = &(_k2->points[i]); //point i of keypoints2
        _d += distance_between_point(p1, p2);
    }
    return _d;
}

std::vector<size_t> HandGestureRecognitionCalculator::sort_indexes(const std::vector<float> &v) {

  // initialize original index locations
  std::vector<size_t> idx(v.size());
  std::iota(idx.begin(), idx.end(), 0);
  std::stable_sort(idx.begin(), idx.end(),
       [&v](size_t i1, size_t i2) {return v[i1] < v[i2];});
  return idx;
}

void HandGestureRecognitionCalculator::calculate_distance(keypoints* k1){
    for(int i=0;i<30;i++){
        _distance.push_back(distance_between_keypoint(k1,&datas.samples[i]));
    }
}

int HandGestureRecognitionCalculator::query(keypoints* k1){
    //calculate disatances between this keypoint and all sample keypoints.
    calculate_distance(k1); 
    std::vector<size_t> index = sort_indexes(_distance); //argsort distances
    //select topk and vote
    int count0 = 0;
    int count1 = 0;
    int count2 = 0;
    int sample_id = -1;
    for (int i=0; i<top_k; i++){
        if(class_id[(int)index[i]] == 0){
            count0 += 1;
        }
        if(class_id[(int)index[i]] == 1){
            count1 += 1;
        }
        if(class_id[(int)index[i]] == 2){
            count2 += 1;
        }
    }
    if (count0 > count1 && count0 > count2){
        sample_id = 0;
    }
    if (count1 > count0 && count1 > count2){
        sample_id = 1;
    }
    if (count2 > count0 && count2 > count1){
        sample_id = 2;
    }
    // int sample_id = class_id[(int)index[0]];
    return sample_id;
}

} // namespace mediapipe
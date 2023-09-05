#include <jni.h>
#include <vector>
#include "IMU_process.h"
extern "C"
JNIEXPORT jdouble JNICALL
Java_com_example_myfirstapp_MainActivity_IMU_1cal(JNIEnv *env, jobject thiz,
                                                  jdoubleArray oneDArray,
                                                  jobjectArray twoDArray) {

    // Convert jdoubleArray to vector<double>
    jsize len1D = env->GetArrayLength(oneDArray);
    jdouble *body1D = env->GetDoubleArrayElements(oneDArray, 0);
    std::vector<double> vec1D(body1D, body1D + len1D);
    env->ReleaseDoubleArrayElements(oneDArray, body1D, 0);

// Convert jobjectArray (which is an array of jdoubleArray) to vector<vector<double>>
    std::vector<std::vector<double>> vec2D;
    jsize lenOuter = env->GetArrayLength(twoDArray);
    for (jsize i = 0; i < lenOuter; i++) {
        jdoubleArray innerArray = (jdoubleArray) env->GetObjectArrayElement(twoDArray, i);
        jsize lenInner = env->GetArrayLength(innerArray);
        jdouble *bodyInner = env->GetDoubleArrayElements(innerArray, 0);

        std::vector<double> vecInner(bodyInner, bodyInner + lenInner);
        vec2D.push_back(vecInner);

        env->ReleaseDoubleArrayElements(innerArray, bodyInner, 0);
        env->DeleteLocalRef(innerArray);
    }
    double dis;
    IMU_cal JJD;
    JJD.displacement_inmeter(vec1D,vec2D,&dis);
    return dis;
}
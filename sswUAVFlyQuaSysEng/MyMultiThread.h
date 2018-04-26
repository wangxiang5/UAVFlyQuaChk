#ifndef _my_multithread_h
#define _my_multithread_h
#pragma once




//影像与POS匹配线程函数
void WINAPI MultiThreadImgPosMatch(LPVOID p);
//相机畸变改正线程函数
void WINAPI MultiThreadCmrDistortionCorrect(LPVOID p);
//排列航带
void WINAPI MultiThreadReOrderStrips(LPVOID p);
//设置影像有效性
void WINAPI MultiThreadSetImgValidState(LPVOID p);
//空三处理
void WINAPI MultiThreadFlyQuaChkFun(LPVOID p);
//生成影像纹理
void WINAPI MultiThreadCreateImgTexture(LPVOID p);
//生成影像投影取VBO
//void WINAPI MultiThreadCreateCqProjectVbo(LPVOID p);


#endif
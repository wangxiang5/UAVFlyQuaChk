#ifndef _My_base_type_view_h
#define _My_base_type_view_h
#pragma once
#include "MyDataBaseType.h"

template <class DataType>
class Point2_
{
public:
	typedef Point2_<DataType> MiType;
	Point2_(){x=InvalidValue;y=InvalidValue;}
//		: Point2_(DataType(InvalidValue), DataType(InvalidValue)){}
	Point2_(const DataType _x, const DataType _y)
		: x(_x), y(_y){}
	Point2_(const MiType &p)
		: x(p.x), y(p.y){}
	~Point2_(){}

private:

public:
	DataType x;
	DataType y;

public:
	// define MiType operators

	MiType operator+(MiType pt){
		return Point2_(x + pt.x, y + pt.y);
	}
	MiType operator-(MiType pt){
		return Point2_(x - pt.x, y - pt.y);
	}
	MiType operator-(){
		return Point2_(-x, -y);
	}
	MiType operator*(MiType pt){
		return Point2_(x *= pt.x, y *= pt.y);
	}
	MiType operator=(MiType pt){
		return Point2_(x = pt.x, y = pt.y);
	}
	bool operator!=(MiType pt){
		return (x != pt.x|| y != pt.y);
	}
	bool operator==(MiType pt){
		return (x == pt.x && y == pt.y);
	}
	MiType operator+=(MiType pt){
		return (*this = *this + pt);
	}
	MiType operator-=(MiType pt){
		return (*this = *this - pt);
	}
	MiType operator*(DataType k){
		return Point2_(x *= k, y *= k);
	}
	MiType operator/(DataType k){
		if (k != 0) return Point2_(x /= k, y /= k);
		else return Point2_(x, y);
	}
};

template <class DataType>
class Point3_
{
public:
	typedef Point3_<DataType> MiType;

	Point3_(){}
//		: Point3_(DataType(InvalidValue), DataType(InvalidValue), DataType(InvalidValue)){}
	Point3_(const DataType _x, const DataType _y, const DataType _z)
		: X(_x), Y(_y), Z(_z){}
	~Point3_(){}


private:

public:
	DataType X;
	DataType Y;
	DataType Z;

public:

	MiType operator+(MiType gt){
		return Point3_(X + gt.X, Y + gt.Y, Z + gt.Z);
	}
	MiType operator-(MiType gt){
		return Point3_(X - gt.X, Y - gt.Y, Z - gt.Z);
	}
	MiType operator-(){
		return Point3_(-X, -Y, -Z);
	}
	MiType operator*(MiType gt){
		return Point3_(X *= gt.X, Y *= gt.Y, Z *= gt.Z);
	}
	MiType operator/(DataType a){
		if (a != DataType(0))
			return Point3_(X /= a, Y /= a, Z /= a);
		else return Point3_(999999999999999, 999999999999999, 9999999999999999);
	}
	MiType operator=(MiType gt){
		return Point3_(X = gt.X, Y = gt.Y, Z = gt.Z);
	}
	bool operator==(MiType gt){
		return (X == gt.X && Y == gt.Y && Z = gt.Z);
	}
	MiType operator+=(MiType gt){
		return (*this = *this + gt);
	}
	MiType operator-=(MiType gt){
		return (*this = *this - gt);
	}
	MiType operator/=(DataType a){
		if (a != DataType(0))
			return (*this = *this / a);
		else return Point3_(999999999999999, 999999999999999, 9999999999999999);
	}

};

template <class DataType>
class CRect_
{
public:  //构造初始化
	typedef CRect_<DataType> MiType;
	CRect_(){}
//		: CRect_(DataType(InvalidValue), DataType(InvalidValue), DataType(InvalidValue), DataType(InvalidValue)){}
	CRect_(const DataType _l, const DataType _t, const DataType _r, const DataType _b)
		: left(_l), top(_t), right(_r), bottom(_b){}
	CRect_(Point2_<DataType> p1, Point2_<DataType> p2)
		: left(p1.x), top(p1.y), right(p2.x), bottom(p2.y){}
	CRect_(const MiType &p)
		: left(p.left), top(p.top), right(p.right), bottom(p.bottom){}
public:
	DataType left;
	DataType right;
	DataType top;
	DataType bottom;

	Point2_<DataType> RightTop(){ return Point2_<DataType>(right, top); };
	Point2_<DataType> LeftBottom(){ return Point2_<DataType>(left, bottom); };
	Point2_<DataType> LeftTop(){ return Point2_<DataType>(left, top); };
	Point2_<DataType> RightBottom(){ return Point2_<DataType>(right, bottom); };
	Point2_<DataType> CenterPoint(){ return Point2_<DataType>((right + left) / 2, (top + bottom) / 2); };

	DataType Width(){ return fabs(right - left); };
	DataType Height(){ return fabs(top - bottom); };
	DataType Area(){ return Width()*Height(); };

	bool operator==(MiType drect){ //矩形赋值
		return (left == drect.left&& top == drect.top&& right == drect.right&& bottom == drect.bottom);
	}
	MiType operator=(MiType drect){ //矩形赋值
		return CRect_(left = drect.left, top = drect.top, right = drect.right, bottom = drect.bottom);
	}

	MiType operator+(MiType drect){//矩形求并
		return CRect_(left = min(left, drect.left), top = max(top, drect.top),
			right = max(right, drect.right), bottom = min(bottom, drect.bottom));
	}

public:
	bool Intersect_Rect(MiType dr){
		if (left > dr.right || right < dr.left ||
			top<dr.bottom || bottom>dr.top)
			return false;
		return true;
	}
	bool Intersect_Pt(Point2_<DataType> pt){
		if ((pt.x - left)*(pt.x - right) < 0 && (pt.y - top)*(pt.y - bottom) < 0)
			return true;
		return false;
	}

private:

};

typedef Point2_<double> Point2D;
typedef Point3_<double> Point3D;
typedef CRect_<double> CRectD;
inline bool FunIntersectCRectD(CRectD rd1, CRectD rd2, CRectD &rd1_2)
{
	double l, t, r, b;
	l = max(rd1.left, rd2.left);
	r = min(rd1.right, rd2.right);
	if (r < l) return false;
	if (rd1.top >= rd1.bottom)
	{
		t = min(rd1.top, rd2.top);
		b = max(rd1.bottom, rd2.bottom);
		if (t < b) return false;
	}
	if (rd1.top < rd1.bottom)
	{
		t = max(rd1.top, rd2.top);
		b = min(rd1.bottom, rd2.bottom);
		if (t > b) return false;
	}
	rd1_2 = CRectD(l, t, r, b);
	return true;
}
inline bool FunCombineCRectD(CRectD rd1, CRectD rd2, CRectD &rd1_2)
{
	if (rd1.Width()*rd1.Height() == 0 && rd2.Width()*rd2.Height() != 0)
	{
		rd1_2 = rd2; return true;
	}
	if (rd1.Width()*rd1.Height() != 0 && rd2.Width()*rd2.Height() == 0)
	{
		rd1_2 = rd1; return true;
	}
	double l, t, r, b;
	l = min(rd1.left, rd2.left);
	r = max(rd1.right, rd2.right);
	if (rd1.top > rd1.bottom)
	{
		t = max(rd1.top, rd2.top);
		b = min(rd1.bottom, rd2.bottom);
	}
	else
	{
		t = min(rd1.top, rd2.top);
		b = max(rd1.bottom, rd2.bottom);
	}
	rd1_2 = CRectD(l, t, r, b); return true;
}

inline double FunDistandPoint2D(Point2D pt1, Point2D pt2)
{
	return sqrt(pow(pt1.x-pt2.x,2)+pow(pt1.y-pt2.y,2));
}
#endif
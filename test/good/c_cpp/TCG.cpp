/*
Template for Computation Geometry
Author: sths
Team: StillAlive(sths, lcch, Pluto)
*/
#include <iostream>
#include <stdio.h>
#include <string>
#include <string.h>
#include <math.h>
#include <algorithm>
using namespace std;

const int maxNch=1000+10; // 凸包边长
const double inf=1e7;
const double eps=1e-7; // 精度
const double pi=acos(-1.0); // PIaslkdj
const int maxNpol=10+5; // 多边形边长
const int MaxInt=2147483647;
typedef double crdnt;

// ABS
template <class T>
T ABS(T x) {
	if (x>eps) return x;
	if (x<-eps) return -x;
	return 0;
}

struct tdot // 二维点
{
	crdnt x, y;
	tdot() {}
	tdot(crdnt x_, crdnt y_):x(x_), y(y_) {}
	void init() 
	{ 
		scanf("%lf%lf", &x, &y); 
		// scanf("%d%d", &x, &y);
	}
	void print() 
	{ 
		printf("(%.3lf, %.3lf)\n", x, y); 
		// printf("(%d, %d)\n", x, y); 
	}
	double length()
	{
		return sqrt(x*x+y*y);
	}
};
struct hdot // 三维点
{
	crdnt x, y, z;
	hdot() {}
	hdot(crdnt x_, crdnt y_, crdnt z_):x(x_), y(y_), z(z_) {}
};
struct tpol // 二维多边形
{
	int n;
	tdot dot[maxNpol];
};
struct tseg
{
	tdot s, t;
	tseg() {}
	tseg(tdot s_, tdot t_):s(s_), t(t_) {}
	tseg(crdnt x1, crdnt y1, crdnt x2, crdnt y2):s(x1, y1), t(x2, y2) {}
	crdnt operator () (crdnt x) { return s.y+(t.y-s.y)/(t.x-s.x)*(x-s.x); }
	tdot operator [] (crdnt x) { return tdot(x, s.y+(t.y-s.y)/(t.x-s.x)*(x-s.x)); }
	void init() 
	{ 
		scanf("%lf%lf%lf%lf", &s.x, &s.y, &t.x, &t.y); 
	}
};
// 二维直线ax+by+c=0 或 半平面 ax+by+c>=0 为统一方程，固定a=1,a=0的情况下固定b=1
struct tline
{
	crdnt a, b, c; // 直线方程
	double f; // 法向量的极角，注意这个法向量指向半平面内部
	tline() {}
	tline(crdnt a_, crdnt b_, crdnt c_):a(a_), b(b_), c(c_)
	{
		f=atan2(b, a);
		if (ABS(a)>eps) b/=ABS(a), c/=ABS(a), a/=ABS(a);
		else c/=ABS(b), b/=ABS(b);
	}
	tline(tseg seg) // 将线段转化成直线,
	{
		a=seg.s.y-seg.t.y;
		b=seg.t.x-seg.s.x;
		c=-a*seg.s.x-b*seg.s.y;
		f=atan2(b, a);
		if (ABS(a)>eps) b/=ABS(a), c/=ABS(a), a/=ABS(a);
		else c/=ABS(b), b/=ABS(b);
	}
	tline (tdot s, tdot t)
	{
		a=s.y-t.y;
		b=t.x-s.x;
		c=-a*s.x-b*s.y;
		f=atan2(b, a);
		if (ABS(a)>eps) b/=ABS(a), c/=ABS(a), a/=ABS(a);
		else c/=ABS(b), b/=ABS(b);
	}
	crdnt func(tdot o)
	{
		return a*o.x+b*o.y+c;
	}
	crdnt func(crdnt x, crdnt y)
	{
		return a*x+b*y+c;
	}
	void print()
	{
		printf("%lf %lf %lf %lf\n", a, b, c, f);
	}
};
// Value
int gcd(int a, int b) { if (b) return gcd(b, a%b); else return a; }
crdnt max(crdnt a, crdnt b) { if (a>b) return a; return b; }
crdnt min(crdnt a, crdnt b) { if (a<b) return a; return b; }
int sign(int a) { if (a>0) return 1; if (a<0) return -1; return 0; }
int sign(double a) { if (a>eps) return 1; if (a<-eps) return -1; return 0; }

// ******************** TDOT ********************

// 加法
tdot operator + (tdot a, tdot b) 
{ 
	return tdot(a.x+b.x, a.y+b.y); 
}
// 减法
tdot operator - (tdot a, tdot b) 
{ 
	return tdot(a.x-b.x, a.y-b.y); 
} 
// 叉积
crdnt operator * (tdot a, tdot b) 
{	
	return a.x*b.y-a.y*b.x; 
} 
// 点积
crdnt operator % (tdot a, tdot b) 
{ 
	return a.x*b.x+a.y*b.y; 
} 
// 求a, b, c三点的叉积
crdnt Cross(tdot a, tdot b, tdot c)
{ 
	/*printf("Cross\n"); a.print(); b.print(); c.print(); */
	return (b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x); 
}
// 求三角形(a, b, c)的面积
double Area(tdot a, tdot b, tdot c) 
{ 
	return 0.5*ABS((b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x));
} 
// 求距离
double Len(tdot a, tdot b) 
{ 
	return sqrt(eps+(a.x-b.x)*(a.x-b.x)+(a.y-b.y)*(a.y-b.y)); 
} 
double Len(tdot a) { return sqrt(eps+a.x*a.x+a.y*a.y); } // 求长度
// 水平序
int cmpxy(tdot a, tdot b)
{ 
	return a.y<b.y || (a.y==b.y && a.x<b.x);
}
// 极角序
int cmptline(tline a, tline b)
{
	if (sign(a.f-b.f)==0) return a.c<b.c-eps;
	else return a.f<b.f-eps;
}
// 求a与b的夹角
double Angle(tdot a, tdot b) 
{
	double temp=acos(a%b/Len(a)/Len(b));
	if (sign(a*b)==1) return temp;
	else return -temp;
}
// 将向量a逆时针旋转angle
tdot Rotate(tdot a, double angle)
{
	return tdot(a.x*cos(angle)-a.y*sin(angle), a.x*sin(angle)+a.y*cos(angle));
}
// 将向量a长度调整为length
tdot Lengthlize(tdot a, double len)
{
	if (a.length()<eps) return a; // 如果a为0向量，返回0向量
	return tdot(a.x*len/a.length(), a.y*len/a.length());
}
int operator == (tdot a, tdot b) { return sign(a*b)==0 && sign(a%b)>0; } // 判断两个向量是否相等
int operator != (tdot a, tdot b) { return sign(a*b)!=0 || sign(a%b)<=0; } // 判断两个向量是否不同
// HDOT
hdot operator + (hdot a, hdot b) { return hdot(a.x+b.x, a.y+b.y, a.z+b.z); } // 加法
hdot operator - (hdot a, hdot b) { return hdot(a.x-b.x, a.y-b.y, a.z-b.z); } // 减法
hdot operator * (hdot a, hdot b) { return hdot(a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x); } // 叉积
crdnt operator % (hdot a, hdot b) { return a.x*b.x+a.y*b.y+a.z*b.z; } // 点积
crdnt Mix(hdot a, hdot b, hdot c, hdot d) { return (b-a)*(c-a)%(d-a); } // 混合积
crdnt Vol(hdot a, hdot b, hdot c, hdot d) { return ABS((b-a)*(c-a)%(d-a))/6; } // 四面体体积
// TPOL
// TLIN

// ******************** TSEG ********************

// 求两线段交点
tdot its(tseg a, tseg b)
{
	double s1=Area(b.s, b.t, a.s), s2=Area(b.s, b.t, a.t);
	return tdot((a.s.x*s2+a.t.x*s1)/(s1+s2), (a.s.y*s2+a.t.y*s1)/(s1+s2));
}
tdot its(tline l1, tline l2)
{
	return tdot((l2.c*l1.b-l1.c*l2.b)/(l1.a*l2.b-l2.a*l1.b), (l2.c*l1.a-l1.c*l2.a)/(l1.b*l2.a-l2.b*l1.a));
}
// 询问两条线段是否有交点
bool sct(tline l1, tline l2) {
	return ABS(l1.a*l2.b-l2.a*l1.b)>eps;
}

bool sct(tdot a1, tdot a2, tdot b1, tdot b2) 
{
	if (sign(Cross(a1, b1, b2))==0 && sign(Cross(a2, b1, b2))==0) // 两条线段斜率一样
	{
		if (ABS(ABS(Len(a1, b1)-Len(a2, b2))-(Len(a1, a2)+Len(b1, b2)))<eps) return false;
		if (ABS(ABS(Len(a1, b2)-Len(a2, b1))-(Len(a1, a2)+Len(b1, b2)))<eps) return false;
		return true;
	}
	if (sign(Cross(a1, b1, b2)*Cross(a2, b1, b2))>0) return false;
	if (sign(Cross(b1, a1, a2)*Cross(b2, a1, a2))>0) return false;
	return true;
}
bool sct(tseg a, tseg b) 
{
	return sct(a.s, a.t, b.s, b.t);
}

// ******************** AGORITHM ********************

// 二维凸包 a[0..n-1], b[0..top-1]
int CHstk[maxNch];
void ConvexHull(tdot a[], int n, tdot b[], int &top)
{
	sort(a, a+n, cmpxy);
	top=2; CHstk[1]=0; CHstk[2]=1;
	for (int i=2; i<n; CHstk[++top]=i++)
		while (top>1 && sign(Cross(a[CHstk[top-1]], a[CHstk[top]], a[i]))<0) top--; // <0为所有点，<=0为去掉多余的点
	for (int i=n-1; i>=0; CHstk[++top]=i--)
		while (top>1 && sign(Cross(a[CHstk[top-1]], a[CHstk[top]], a[i]))<0) top--; // <0为所有点, <=0为去掉多余的点
	for (int i=1; i<=--top; i++) b[i-1]=a[CHstk[i]]; 
}

// 半平面交 a[0..n-1], b[l..r]
int needtopop(tline c, tline a, tline b)
{
	tdot o=its(a, b);
	if (c.func(o)<=0) return true;
	else return false;
}
int HalfPlane(tline a[], int n, tline b[], int &l, int &r)
{
	a[n++]=tline(1, 0, inf);
	a[n++]=tline(-1, 0, inf);
	a[n++]=tline(0, 1, inf);
	a[n++]=tline(0, -1, inf);
	sort(a, a+n, cmptline);
	int m=1;
	for (int i=1; i<n; i++) if (sign(a[i].f-a[i-1].f)!=0) a[m++]=a[i];
	n=m;
	l=0; r=1;
	b[0]=a[0]; b[1]=a[1];
	for (int i=2; i<n; i++)
	{
		while (l<r && needtopop(a[i], b[r], b[r-1])) r--;
		while (l<r && needtopop(a[i], b[l], b[l+1])) l++;
		b[++r]=a[i];
	}
	while (l<r && needtopop(b[l], b[r], b[r-1])) r--;
	while (l<r && needtopop(b[r], b[l], b[l+1])) l++;
	return r-l>=2;
}

const int MaxN=20000+10;
const int n = 10000;

tdot a[MaxN], b[MaxN];

int main() {
	//freopen("output.txt", "w", stdout);
	srand(time(NULL));
	for (int i = 0; i < n; i++) a[i] = tdot(rand() % 10000, rand() % 10000);
	//for (int i = 0; i < n; i++) a[i].print();
	int m = 0;
	ConvexHull(a, n, b, m);
	//for (int i = 0; i < m; i++) b[i].print();
	return 0;
}

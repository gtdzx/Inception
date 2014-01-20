#include<iostream>
using namespace std;
int f(int n) {
    if(n <= 2)
        return 1;
    else 
        return f(n-1) + f(n-2);
}
int n;
int main()
{
    cin >> n;
    cout << f(n);
    return 0;
}


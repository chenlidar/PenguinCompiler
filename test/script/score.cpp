#include <iostream>
#include <string>
using namespace std;

#if 0
char m[200];
char n[200];
int main()
{
	string s;
	while (cin >> s) {
		int a, b, c, d;
		scanf("%s %s\nTOTAL: %dH-%dM-%dS-%dus", m, n, &a, &b, &c, &d);
		printf("%d.%06d\n", a*3600+b*60+c, d);
	}
}
#endif
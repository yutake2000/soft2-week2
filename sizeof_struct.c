#include <stdio.h>

struct point1
{
  int x;
  int y;
};

struct point2
{
  int x;
  int y;
  char c;
};

struct point3
{
  int x;
  int y;
  char c[5];
};

struct point4
{
  int x;
  int y;
  short s[3];
};

int main()
{
   printf ( "%zu\n", sizeof(struct point1));
   printf ( "%zu\n", sizeof(struct point2));
   printf ( "%zu\n", sizeof(struct point3));
   printf ( "%zu\n", sizeof(struct point4));
}

#include <stdio.h>

struct student
{
  int id;
  char name[100];
  int age; // +4 (doubleに合わせるため)
  double height;
  double weight;
}; //124 + 4 = 128

struct tagged_student1
{
  int id;
  char name[100];
  int age; // +4 (doubleに合わせるため)
  double height;
  double weight;
  char tag; // +7 (doubleに合わせるため(配列になったときに必要))
}; //125 + 11 = 136

struct tagged_student2
{
  char tag; // +3 (intに合わせるため)
  int id;
  char name[100];
  int age;
  double height;
  double weight;
}; //125 + 3 = 128

void print_student(const struct student *s) {
  printf("%p %p %p %p %p\n", &s->id, &s->name, &s->age, &s->height, &s->weight);
}

void print_tagged_student1(const struct tagged_student1 *s) {
  printf("%p %p %p %p %p %p\n", &s->id, &s->name, &s->age, &s->height, &s->weight, &s->tag);
}

void print_tagged_student2(const struct tagged_student2 *s) {
  printf("%p %p %p %p %p %p\n", &s->tag, &s->id, &s->name, &s->age, &s->height, &s->weight);
}

int main (int argc, char**argv)
{
  struct student s_begin;
  struct student s1;
  struct tagged_student1 s2;
  struct tagged_student2 s3;
  struct tagged_student2 s_end;

  /* 以下に各構造体の中身のポインタを表示するプログラムを書く */
  /* printf で %p フォーマットを指定する*/
  /* 逆順に表示（send, s3, s2, s1, sbegin の順）*/

  printf("%p %p %p %p %p\n", &s_end, &s3, &s2, &s1, &s_begin);

  /* 以下には各構造体のサイズをsizeof演算子で計算し、表示する */
  /* printf 表示には%ld を用いる*/

  printf("%ld\n", sizeof(struct student));
  printf("%ld\n", sizeof(struct tagged_student1));
  printf("%ld\n", sizeof(struct tagged_student2));

  print_tagged_student2(&s_end);
  print_tagged_student2(&s3);
  print_tagged_student1(&s2);
  print_student(&s1);
  print_student(&s_begin);
  
  return 0;
}

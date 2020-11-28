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

void print_student(char var[], const struct student *s) {
  printf("%8s: %p %p %p %p %p\n", var, &s->id, &s->name, &s->age, &s->height, &s->weight);
}

void print_tagged_student1(char var[], const struct tagged_student1 *s) {
  printf("%8s: %p %p %p %p %p %p\n", var, &s->id, &s->name, &s->age, &s->height, &s->weight, &s->tag);
}

void print_tagged_student2(char var[], const struct tagged_student2 *s) {
  printf("%8s: %p %p %p %p %p %p\n", var, &s->tag, &s->id, &s->name, &s->age, &s->height, &s->weight);
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

  printf("address\n%18s %18s %18s %18s %18s\n", "send", "s3", "s2", "s1", "sbegin");
  printf("%18p %18p %18p %18p %18p\n\n", &s_end, &s3, &s2, &s1, &s_begin);

  /* 以下には各構造体のサイズをsizeof演算子で計算し、表示する */
  /* printf 表示には%ld を用いる*/

  printf("sizeof\n%16s %16s %16s\n", "student", "tagged_student1", "tagged_student2");
  printf("%16ld %16ld %16ld\n\n", sizeof(struct student), sizeof(struct tagged_student1), sizeof(struct tagged_student2));

  print_tagged_student2("s_end", &s_end);
  print_tagged_student2("s3", &s3);
  print_tagged_student1("s2", &s2);
  print_student("s1", &s1);
  print_student("s_begin", &s_begin);
  
  return 0;
}

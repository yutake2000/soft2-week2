/*
  太陽系の惑星シミュレーション

  表示範囲の制限で地球型惑星のみ。
  Wikipediaの各惑星の情報をもとに、初期パラメータに遠日点の距離と速度を用いた。
  (速度は面積速度一定の法則から平均公転距離*平均公転速度/遠日点距離で求めた。)

  現在の惑星の位置は調べてもわからなかったので、一直線上に並べて、それらが全て遠日点と仮定しシミュレーションしている。
  よって惑星同士の位置関係は本来と異なるが、それぞれの惑星の軌道はおおよそ再現できていると思われる。
  公転周期も実際のものとほぼ一致している(特に、365日で地球が一周している)。
  ただし、公転周期の小さい水星は誤差が大きくなる。

  初期位置と速度を現在のものに変更できれば未来の惑星位置が予測できる。

  0.1auが高さ1マス、幅2マス分になっている。
  (幅が2倍になっているのは、軌道が縦長に見えてしまうのを防ぐため。)

  コマンドライン引数に日数も指定できる
  実行例:
    水星の公転周期
    ./a.out 5 data4_sun.dat 87.97
    金星の公転周期
    ./a.out 5 data4_sun.dat 224.70
    地球の公転周期(引数を省略すると365になる)
    ./a.out 5 data4_sun.dat
    火星の公転周期
    ./a.out 5 data4_sun.dat 686.98
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <termios.h>
#include <fcntl.h>
#include "my_bouncing4.h"

int main(int argc, char **argv)
{
  const Condition cond = {
		    .width  = 75,
		    .height = 40,
		    .G = 6.67430e-11,
		    .dt = 60*60*24,
        .au = 149597870700,
  };

  if (argc < 3) {
    fprintf(stderr, "usage: <objnum> <filename>\n");
    return 1;
  }
  
  size_t objnum = atoi(argv[1]);
  Object objects[100];

  load_objects(objnum, objects, argv[2], cond);

  // シミュレーション. ループは整数で回しつつ、実数時間も更新する
  const double stop_time = (argc == 4 ? atof(argv[3]) : 365) * 60 * 60 * 24;
  double t = 0;
  int line = 0;
  char c = 0;

  for (int i = 0 ; t < stop_time ; i++) {

    if (line > 0) printf("\e[%dA", line); // カーソルを表示した分だけ上に戻す
    line = 0;

    t = i * cond.dt;
    my_update_positions(objects, objnum, cond);
    my_update_velocities(objects, objnum, cond);
    
    // 表示の座標系は width/2, height/2 のピクセル位置が原点となるようにする
    line += my_plot_objects(objects, objnum, t, cond);
    
    usleep(10 * 1000);
  }

  return EXIT_SUCCESS;
}

int my_plot_objects(Object objs[], const size_t numobj, const double t, const Condition cond) {

  int line = 0;

  char board[cond.height+2][cond.width+2];

  // 全て空白で埋める
  for (int y=0; y<cond.height+2; y++) {
    for (int x=0; x<cond.width+2; x++) {
      board[y][x] = ' ';
    }
  }

  // 物体
  for (int i=0; i<numobj; i++) {
    int y = objs[i].y / (cond.au / 10) + cond.height/2 + 1;
    int x = objs[i].x*2 / (cond.au / 10) + cond.width/2 + 1;
    if (0 <= y && y < cond.height+2 && 0 <= x && x < cond.width+2) {
      board[y][x] = 'o';
    }
  }

  // 四隅の +
  board[0][0] = board[0][cond.width+1] = board[cond.height+1][0] = board[cond.height+1][cond.width+1] = '+';

  // 上下の -
  for (int x=1; x<cond.width+1; x++) {
    board[0][x] = '-';
    board[cond.height+1][x] = '-';
  }

  // 左右の |
  for (int y=1; y<cond.height+1; y++) {
    board[y][0] = '|';
    board[y][cond.width+1] = '|';
  }

  // boardを表示
  for (int y=0; y<cond.height+2; y++) {
    for (int x=0; x<cond.width+2; x++) {
      printf("%c", board[y][x]);
    }
    printf("\r\n");
  }

  line += cond.height + 2;

  //情報を表示
  printf("t = %4.1lf days, numobj = %zu \r\n", t / 60 / 60 / 24, numobj);
  line++;
  for (int i=0; i<8 && i<numobj; i++) {
    printf("%d: .y = %6.2lf .x = %6.2lf [au] .vy = %6.3lf vx = %6.3lf [au/day]\r\n",
      i, objs[i].y / cond.au, objs[i].x / cond.au, objs[i].vy / cond.au * cond.dt, objs[i].vx / cond.au * cond.dt);
    line++;
  }

  return line;

}


void my_update_velocities(Object objs[], const size_t numobj, const Condition cond) {

  // 速度を更新
  for (int i=0; i<numobj; i++) {
    for (int j=0; j<numobj; j++) {
      if (i == j) continue;

      double dist = distance(objs[i], objs[j], cond);
      objs[i].vy += cond.G * objs[j].m * (objs[j].y - objs[i].y) / pow(dist, 3) * cond.dt;
      objs[i].vx += cond.G * objs[j].m * (objs[j].x - objs[i].x) / pow(dist, 3) * cond.dt;
    }
  }
}


void my_update_positions(Object objs[], const size_t numobj, const Condition cond) {

  // 現在の位置をprev_yに保存してから更新する
  for (int i=0; i<numobj; i++) {
    objs[i].prev_y = objs[i].y;
    objs[i].y += objs[i].vy * cond.dt;
    objs[i].prev_x = objs[i].x;
    objs[i].x += objs[i].vx * cond.dt;
  }

}

void load_objects(size_t numobj, Object objs[], char filename[], const Condition cond) {

  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    fprintf(stderr, "Couldn't open '%s'\r\n", filename);
    exit(-1);
  }

  int buffer_len = 1000;
  char buffer[buffer_len];
  int i = 0; //objsのインデックス
  while (i < numobj && (fgets(buffer, buffer_len-1, fp)) != NULL) {

    // #で始まる行はコメント
    if (buffer[0] == '#') continue;

    sscanf(buffer, "%lf %lf %lf %lf %lf", &objs[i].m, &objs[i].x, &objs[i].y, &objs[i].vx, &objs[i].vy);

    i++;
  }

  fclose(fp);

}

double distance(Object o1, Object o2, const Condition cond) {
  return sqrt(pow(o1.y - o2.y, 2) + pow(o1.x - o2.x, 2));
}

int in_screen(double y, double x, const Condition cond) {
  return -cond.height/2 <= y && y <= cond.height/2 && -cond.width/2 <= x && x <= cond.width/2;
}

int is_monotonic(double a, double b, double c) {
  return (a <= b && b <= c) || (c <= b && b <= a);
}
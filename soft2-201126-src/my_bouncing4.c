/*
  太陽系の惑星シミュレーション

  初期位置は2020/06/21 06:44:00(夏至)の惑星の日心座標とした。
  下記のサイトから「日心黄経(度)」「動径(au)」「日変化(度/日)」をファイルに保存し、x,y,vx,vyはこれらの情報から求めた。
  (国立天文台 惑星の日心座標: https://eco.mtk.nao.ac.jp/cgi-bin/koyomi/cande/planet_ecliptic.cgi)

  ただし、地球の日心座標が得られなかったため太陽の地心座標から計算した。
  太陽の地心黄経が90.000220度なので、地球の日心黄経は270.000220度とした(光行差を含むが補正はしていない)。
  地心距離が1.0162988auなので、そのまま動径とした。
  日変化は2020/06/21 07:44:00(1時間後)の地心黄経90.039996との差の24倍とした。
  (国立天文台 太陽の地心座標: https://eco.mtk.nao.ac.jp/cgi-bin/koyomi/cande/sun.cgi)

  惑星の質量と公転周期はWikipediaから得た。

  デフォルトでは0.1auが高さ1マス、幅2マス分になっている。
  (幅が2倍になっているのは、軌道が縦長に見えてしまうのを防ぐため。)

  コマンドライン引数は以下の通り
    [ファイル名 | moon] (縮尺[au/高さ1マス] シミュレーション時間[日] 時間刻み幅[日])

  ファイルの形式はmy_bouncing2.cで読み込むものと異なるので注意。
  また、オブジェクト数がファイルに書かれたものより多い場合もランダム生成はしない。

  シミュレーション時間に公転周期を設定すると1周して同じ位置に戻ってくるのが確認できる。
  (最初に1秒止まるのは初期位置を確認するため。)
  実行例:
    水星
    ./a.out data4_solar_system.dat 0.1 87.97

    金星
    ./a.out data4_solar_system.dat 0.1 224.70

    地球(引数を省略すると地球が一番見やすくなる設定になる)
    ./a.out data4_solar_system.dat

    火星
    ./a.out data4_solar_system.dat 0.1 686.98

    木星
    ./a.out data4_solar_system.dat 2 4329

    土星
    ./a.out data4_solar_system.dat 2 10779

    天王星(時間刻み幅を10日にしないと時間がかかる)
    ./a.out data4_solar_system.dat 2 30752 10

    海王星
    ./a.out data4_solar_system.dat 2 60148 10
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
		    .height = 38,
		    .G = 6.67430e-11,
		    .dt = 60*60*24 * (argc >= 5 ? atof(argv[4]) : 1),
        .au = 149597870700,
        .scale = (argc >= 3 ? atof(argv[2]) : 0.1)
  };

  if (argc < 2) {
    fprintf(stderr, "usage: <objnum> <filename>\n");
    return 1;
  }
  
  size_t objnum = 0;
  Object objects[100];

  load_objects(objects, &objnum, argv[1], cond);

  // シミュレーション. ループは整数で回しつつ、実数時間も更新する
  const double stop_time = (argc >= 4 ? atof(argv[3]) : 365) * 60 * 60 * 24;
  double t = 0;
  int line = 0; // 表示した行数
  
  line = my_plot_objects(objects, objnum, t, cond);
  usleep(1000 * 1000); //初期配置が分かるように一時停止

  for (int i = 0 ; t < stop_time ; i++) {

    if (line > 0) printf("\e[%dA", line); // カーソルを表示した分だけ上に戻す
    line = 0;

    t = i * cond.dt;
    my_update_positions(objects, objnum, cond);
    my_update_velocities(objects, objnum, cond);
    
    // 表示の座標系は width/2, height/2 のピクセル位置が原点となるようにする
    line += my_plot_objects(objects, objnum, t, cond);
    
    // シミュレーション時間が長いほどスリープ時間を短くする
    usleep(10 * 1000 / (stop_time / (60 * 60 * 24 * 265)));
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
    int y = objs[i].y / (cond.au * cond.scale) + cond.height/2 + 1;
    int x = objs[i].x / (cond.au * cond.scale/2) + cond.width/2 + 1;
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
  for (int i=0; i<numobj; i++) {
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

void load_objects(Object objs[], size_t *numobj, char filename[], const Condition cond) {

  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    fprintf(stderr, "Couldn't open '%s'\r\n", filename);
    exit(-1);
  }

  int buffer_len = 1000;
  char buffer[buffer_len];
  int i = 0; //objsのインデックス
  while ((fgets(buffer, buffer_len-1, fp)) != NULL) {

    // #で始まる行はコメント
    if (buffer[0] == '#') continue;

    double r, degree, v; // r[au], degree[度], v[度/日]

    sscanf(buffer, "%lf %lf %lf %lf", &objs[i].m, &degree, &r, &v);

    double dist = r * cond.au; // dist[m]
    double rad = degree / 360 * 2 * M_PI; // rad[rad]
    double vrad = v / 360 * 2 * M_PI; // vrad[rad]

    objs[i].x = dist *  cos(rad);
    objs[i].y = dist * -sin(rad); // y軸は下方向が正なので反転

    objs[i].vx = dist * vrad * -sin(rad) / (60 * 60 * 24);
    objs[i].vy = dist * vrad * -cos(rad) / (60 * 60 * 24);

    i++;
  }

  *numobj = i;

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
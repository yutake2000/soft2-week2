// シミュレーション条件を格納する構造体
// 反発係数CORを追加
typedef struct condition
{
  const int width; // 見えている範囲の幅
  const int height; // 見えている範囲の高さ
  const double G; // 重力定数
  const double dt; // シミュレーションの時間幅
  const double au; // 天文単位
  const double earth_to_moon; // 地球と月の平均距離
  const double scale; // scale[au]を高さ1マス分とする
  const double moon; // 太陽、地球、月を表示するモードなら1
} Condition;

// 個々の物体を表す構造体
typedef struct object
{
  double m;
  double y, x;
  double prev_y, prev_x;
  double vy, vx;
} Object;

int my_plot_objects(Object objs[], const size_t numobj, const double t, const Condition cond);
void my_update_velocities(Object objs[], const size_t numobj, const Condition cond);
void my_update_positions(Object objs[], const size_t numobj, const Condition cond);

// 座標が画面内にあるかどうか判定する
int in_screen(double y, double x, const Condition cond);

// a, b, cがこの順に単調増加または単調減少であるかどうかを判定する
int is_monotonic(double a, double b, double c);

// オブジェクトファイルを読み込む
void load_objects(Object objs[], size_t *numobj, char filename[], const Condition cond);

// 二つのオブジェクトの距離を求める
double distance(Object o1, Object o2, const Condition cond);
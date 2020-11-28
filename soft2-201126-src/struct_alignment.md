# アライメントの考察
struct_alignment.cを実行すると以下のような出力を得た。

struct student, tagged_student1, tagged_student2それぞれのメンバ変数のサイズを合計すると124, 125, 125なので、4バイト、11バイト、3バイトのパディングが存在することになる。

それぞれの構造体のアドレスが8の倍数(下1桁が0か8)になっているのは、8バイト単位で読み込むのが都合が良いからだろう。

## struct student
s_beginの3番目と4番目のアドレス差は8バイトだが3番目の変数は4バイトのintである。4番目が8バイトのdoubleなので、8の倍数に合わせるために4バイトのパディングが存在する(0x7ffee48e7994 - 0x7ffee48e7997)。

## struct tagged_student1
s2の3番目と4番目の変数の間にも同様の理由で4バイトのパディングが存在する(0x7ffee48e788c - 0x7ffee48e788f)。
また、s2の最後の変数とs1の最初の変数のアドレス差は8バイトだがここにはcharが入っているので7バイトのパディングが存在する。
構造体全体のサイズを8の倍数に合わせた結果だろう。よってtagged_student1には計11バイトのパディングが存在する。

## struct tagged_student2
s_endの1番目と2番目に注目するとcharの後に3バイトのパディングがあることがわかる。
これは次が4バイトのintでアドレスを4の倍数にするためである。
s_beginの場合と異なりintの場合は8の倍数にする必要はない。


```
address
              send                 s3                 s2                 s1             sbegin
    0x7ffee48e7720     0x7ffee48e77a0     0x7ffee48e7820     0x7ffee48e78a8     0x7ffee48e7928

sizeof
         student  tagged_student1  tagged_student2
             128              136              128

   s_end: 0x7ffee48e7720 0x7ffee48e7724 0x7ffee48e7728 0x7ffee48e778c 0x7ffee48e7790 0x7ffee48e7798
      s3: 0x7ffee48e77a0 0x7ffee48e77a4 0x7ffee48e77a8 0x7ffee48e780c 0x7ffee48e7810 0x7ffee48e7818
      s2: 0x7ffee48e7820 0x7ffee48e7824 0x7ffee48e7888 0x7ffee48e7890 0x7ffee48e7898 0x7ffee48e78a0
      s1: 0x7ffee48e78a8 0x7ffee48e78ac 0x7ffee48e7910 0x7ffee48e7918 0x7ffee48e7920
 s_begin: 0x7ffee48e7928 0x7ffee48e792c 0x7ffee48e7990 0x7ffee48e7998 0x7ffee48e79a0
```
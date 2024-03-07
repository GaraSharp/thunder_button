/* 
 * title : thunder_button.c  -  Xt / Xaw program no shuusaku 
 *         thunder graphics wo egaku.
 * begin : 2024-02-23 20:47 
 * make  : gcc -o thunder_button thunder_button.c -lXaw -lXt -lX11  (Ubuntu)
 *       : cc  -o thunder_button thunder_button.c -I /usr/X11R6/include/ -L /usr/X11R6/lib/ -L ~/lib -lXaw -lXt -lX11  (Mac, XQuartz)
 * 
 * 
 */

#include  <stdio.h>
#include  <stdlib.h>
#include  <X11/Intrinsic.h>
#include  <X11/StringDefs.h>
#include  <X11/Xaw/Cardinals.h>
#include  <X11/Xaw/Command.h>
#include  <X11/Xaw/Box.h>
#include  <time.h>


#define  WIDTH     160  //  cell field width
#define  HEIGHT    240  //  cell field height
#define  BSIZE     2    //  cell box size
#define  RATE      0.3  //  nonconductive cell density rate

//  vars for Xlib graphix
XColor  col[5];
GC      gc;
Pixmap  pix;

//  vars
int     grid[WIDTH][HEIGHT] ;   //  thunder maze cell array
double  rate = RATE ;           //  thunder cell maze density rate
time_t  initime ;               //  random number seed 


//  dump cell field for debuging (not used)
void  dump_grid() {
  for (int j = 0; j < HEIGHT; j++) {
    for (int i = 0; i < WIDTH; i++) {
      printf("%d  ", grid[i][j]) ;
    }
    printf(": \n") ;
  } 
}


//  allocate colors 
//  (見苦しいので、いずれ書き換えをしたく)
void
getPixelValues(Widget w)
{
  Display *d;
  int  r, g, b, i;

  d = XtDisplay(w);
  //  white 
  col[0].red   = 0xffff;
  col[0].green = 0xffff;
  col[0].blue  = 0xffff;
  XAllocColor(d, DefaultColormap(d, 0), &col[0]);

  //  red
  col[1].red   = 0xffff;
  col[1].green = 0x00;
  col[1].blue  = 0x00;
  XAllocColor(d, DefaultColormap(d, 0), &col[1]);

  //  green 
  col[2].red   = 0x00;
  col[2].green = 0x66ff;
  col[2].blue  = 0x00;
  XAllocColor(d, DefaultColormap(d, 0), &col[2]);

  //  yellow
  col[3].red   = 0xeeff;
  col[3].green = 0xeeff;
  col[3].blue  = 0x00;
  XAllocColor(d, DefaultColormap(d, 0), &col[3]);
}


//  cell box plotting 
void
plotcell(Widget w, int i, int j, int k)
{
  XSetForeground(XtDisplay(w), gc, col[k].pixel);
  XFillRectangle(XtDisplay(w), XtWindow(w), gc, 0+ i*BSIZE, 0+ j*BSIZE, BSIZE, BSIZE);
  XFillRectangle(XtDisplay(w), pix,         gc, 0+ i*BSIZE, 0+ j*BSIZE, BSIZE, BSIZE);
}

//  generate initial cell maze
void
genCellMaze() 
{
  int  i, j ;

  for (j = 0; j < HEIGHT; j++)
    for (i = 0; i < WIDTH; i++) {
      grid[i][j] = 0;
      if (((double)random()/RAND_MAX) <= rate) {
          grid[i][j] = -1;
      }
    }
}


//  seek out vonNeumann neighbourhoods
//  近傍に 1以上の数値の収蔵されたセルがあれば、その値を返す
int  
is_fire_near_p(int x, int y) {
  int width     = WIDTH  ;
  int height    = HEIGHT ;
  int dsp_x[]   = { 0, 1, 0, -1} ;
  int dsp_y[]   = {-1, 0, 1, 0} ;
  int count     = 0 ;

  for (int idx = 0; idx < 4; idx++) {
    //  上、右、下、左の順に、変位分を配列から参照
    int i = x + dsp_x[idx] ;  //  horizontal displacement
    int j = y + dsp_y[idx] ;  //  vertical displacement
    //  調べる座標が、範囲内に収まっているか ?
    if (i >= 0 && j >= 0 && i < width && j < height)  {
      //  調べるセルが 0 より大きいか ?
      //  その場合は、その値を保持しておく
      if (grid[i][j] > 0) {
        count = grid[i][j] ;
      }
    }
  }
  //  
  return count ;
}

//  evolve ; next cell field stage
int 
evolve() {
  int  cnt;
  int  end_p = 1 ;              //  return value 用の変数
  int tmp[WIDTH][HEIGHT] ;      //  変化を検出するためのコピー用配列

  //  tmp = grid
  memcpy(tmp, grid, WIDTH*HEIGHT*sizeof(int));

  //  セル空間 総なめループ
  for (int i = 0; i < WIDTH; i++) {
    for (int j = 0; j < HEIGHT; j++) {
      //  セルが 0 の場合に
      //  つぎの状態がどうなるのかを調べるだけで良い、と
      cnt = is_fire_near_p(i, j) ;
      if (grid[i][j] == 0 && cnt > 0) {
        tmp[i][j] = cnt + 1 ;
        //  セル状態に変化があったので、end_p に 0 (変化あり) を設定
        end_p = 0 ;
      }
    }
  }
  // grid = tmp 
  memcpy(grid, tmp, WIDTH*HEIGHT*sizeof(int));

  //  電撃先端が下端に到達ならば return 2
  for (int x = 0; x < WIDTH; x++) {
    if (tmp[x][HEIGHT-1] > 0) {
      end_p = 2;
    }
  }
  return end_p ;
}


//  draw cell fields
void
drawMaps(Widget w)
{
  int  i, j, k;

  for (j = 0; j < HEIGHT; j++)
    for (i = 0; i < WIDTH; i++) {
      k = 0;            //  conductive cell (WHITE)
      if (grid[i][j] == -1) {
          k = 1;        //  un-condctive cell (RED)
      } else 
      if (grid[i][j] >= 1) {
          k = 2;        //  cell which electric current through (GREEN)
      }
      plotcell(w, i, j, k) ;
    }
}

//  reverse tracking and display
void revtrace(Widget w, int run) {
  int  cell ;
  int  i, j ;

  if (run == 2) {
    printf("reverse tracking\n") ;
    //  電撃先端が下端に到達か 
    for (int x = 0; x < WIDTH; x++) {
      if (grid[x][HEIGHT-1] > 0) {
        i = x ;
        j = HEIGHT-1 ;
        cell = grid[i][j] ;
        while (cell > 1) {
          plotcell(w, i, j, 3) ;       //  cell which electric current through (YELLOW)

          //  first, check above side of cell 
          if (j > 0 && grid[i][j-1] == cell-1) {
            j -= 1 ;
          }
          //  and left side of cell 
          else
          if (i > 0 && grid[i-1][j] == cell-1) {
            i -= 1 ;
          }
          //  and right side of cell 
          else
          if (i < WIDTH-1 && grid[i+1][j] == cell-1) {
            i += 1 ;
          }
          //  at last, check below side of cell 
          else
          if (j < HEIGHT-1 && grid[i][j+1] == cell-1) {
            j += 1;
          }
        
          cell = grid[i][j] ;
        }
      }
    }
  }

}


//  spark !
void
sparkle(Widget command)
{
 //  generate cell maze
  genCellMaze() ;

  //  ignit at centre, top line of cell field
  grid[WIDTH/2][0] = 1 ;

  //  evolve, cell machine loop
  int run = 0 ;
  while (run == 0) {
//    printf("%d ", run) ;
    run = evolve();
  }
  printf("... evolved\n") ;

  // cell stage graphics wo egaku 
  drawMaps(command);

  //  draw 'thunder' line with reverse tracking
  revtrace(command, run) ;

}


//  button call back 
void
quit(Widget w, caddr_t *client_data, caddr_t *call_data)
{
  sparkle(w);
}


//  main app_loop
int
main(int argc, char **argv)
{
  XtAppContext  app_con;
  Widget  toplevel, box, command;
  Arg arg_list[5];
  char titlebar[32];

  //  get density param from arg list
  if (argc > 1) {
    rate = atof(argv[1]) ;
  }

  //  random number seeding
  time(&initime) ;
  srandom(initime);

  /* locale no tugou de, hituyou na 1 gyou */
  /* nani mo kangaezu, kou kaite okeba yoi rasii ... */
  XtSetLanguageProc(NULL, NULL, NULL);

  /* resource name wo 'Sample' to site, Xt kankyou wo shoki ka suru */
  /* kono sai, resource wo yomikomi, tekisetu na settei wo okonau */
  /* tatoeba, '-geometry ' nado no option mo shori sareru */
  toplevel = XtAppInitialize(&app_con, "Thunder Button", NULL, ZERO,
			     &argc, argv, NULL, NULL, ZERO);

  box = XtVaCreateManagedWidget("box", boxWidgetClass, toplevel,
                          NULL);

  /* Command wo moukeru */
  arg_list[0].name  = XtNinternational;
  arg_list[0].value = True;
  arg_list[1].name  = XtNwidth;
  arg_list[1].value = WIDTH*BSIZE;
  arg_list[2].name  = XtNheight;
  arg_list[2].value = HEIGHT*BSIZE;
  command = XtCreateWidget("command", commandWidgetClass, box, arg_list, 3);
  XtManageChild(command);
  XtAddCallback(command, XtNcallback, (XtCallbackProc)quit, NULL);

  /* widget wo hyouji suru */
  XtRealizeWidget(toplevel);

  /* Window ya Pixmap no ID wo torikomu niha, Realize no ato de nai to */
  /* naranai. sono tame, gazou ha koko de egaku koto ni naru */

  /* pixmap wo tukuru */
  pix = XCreatePixmap(XtDisplay(command), XtWindow(command), WIDTH*BSIZE, HEIGHT*BSIZE,
                   DefaultDepth(XtDisplay(command), 0));
  gc = XCreateGC(XtDisplay(command), pix, 0, 0);

  arg_list[0].name  = XtNbitmap;
  arg_list[0].value = pix;
  arg_list[1].name  = XtNinternalHeight;
  arg_list[1].value = 0;
  arg_list[2].name  = XtNinternalWidth;
  arg_list[2].value = 0;
  XtSetValues(command, arg_list, 3);
  
  sprintf(titlebar, "RATE : %4.2f", rate);
  XStoreName(XtDisplay(toplevel), XtWindow(toplevel), titlebar);

  /* pixel no atai wo youi site oku */
  getPixelValues(command);

  //  first spark
  sparkle(command);

  /* call back wo mati nagara, loop ni hairu */
  XtAppMainLoop(app_con);
}


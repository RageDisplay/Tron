// gcc -g game.c -o game -pthread -lncurses
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/fb.h>

#include <string.h>
#include <sys/mman.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <signal.h>
#include <ncurses.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define FRAME 0xFFFF00
int ar1[4],ar2[4],sum1,sum2;
int working_flag = 1;
int i = 0;
int fb;
size_t map_size,page_size,fb_size;

struct fb_var_screeninfo info;
uint32_t *ptr;
pthread_t line, line2, line_key1, line_key2, line_send, line_recv, line_press;
int n1=0;
int n2=0;
int way = 0;
int right = 0;
int way2 = 0;
int left = 0;
struct sockaddr_in me, other;
int my_fd;
int other_fd;
int size = sizeof(me);
FILE *f;
typedef struct Bike{
  unsigned int lose_flag;
  unsigned int x;
  unsigned int y;
  unsigned int name;
  unsigned int side;
  unsigned int key;
  int cord[8][5];
  uint32_t color;
  uint32_t enemy_color;
}Bike;


int win(struct Bike *moto)
{
  munmap(ptr, map_size);
  close(fb);
  if(moto->name == 66)
  {
  mvprintw(1,0, "%clue lost the game \nRed earned %d points",moto->name,i);
  }
  else{
    mvprintw(1,0, "%ced lost the game \nBlue earned %d points",moto->name,i);
  }
  refresh();
  endwin();
  exit(0);
}
void charget()
  {
    n1 = getch();
    if(n1 != "r")
    {
      usleep(100000);
    }
    if(n1 != "q")
    {
      usleep(100000);
    }
    if(n1 != "r")
    {
      usleep(100000);
    }
    if(n1 != "q")
    {
      usleep(100000);
    }
    if(n1 != "r")
    {
      usleep(100000);
    }
  }
void handler()
{
  working_flag = 0; munmap(ptr, map_size);
  close(fb);
  endwin();
  exit(0);
}

void* first_bike_press (void *args)
{
    while(1)
    {
        n1=getch();
        charget();
        sendto(my_fd,&n1,1,MSG_CONFIRM,(struct sockaddr*)&other,size);
    }
}
void* second_bike_press (void *args)
{
    while(1)
    {
        charget();
        recvfrom(my_fd,&n2,1,MSG_WAITALL,(struct sockaddr*)&me,&size);
    }
}
void* firstkeypress(void *args)
{
  Bike* biker = (Bike*) args;
  while(working_flag)
  {
    switch(n1)
    {
      case 'w':
          way = 1;
          biker->side = way;
        break;
      case 's':
          way = 2;
          biker->side = way;
        break;
      case 'a':
          way = 3;
          biker->side = way;
        break;
      case 'd':
          way = 4;
          biker->side = way;
        break;
      default:
        break;

      }
    
  }
}
void* secondkeypress(void *args)
{
  Bike* biker = (Bike*) args;
  while(working_flag)
  {
    switch(n2)
    {
      case 'w':
          way2 = 1;
          biker->side = way2;
        break;
      case 's':
          way2 = 2;
          biker->side = way2;
        break;
      case 'a':
          way2 = 3;
          biker->side = way2;
        break;
      case 'd':
          way2 = 4;
          biker->side = way2;
        break;
      default:
        break;
    }
    
  }
}

int initialization(int *args,char **argv)
{
    page_size = sysconf(_SC_PAGESIZE);
      if ( 0 > (fb = open("/dev/fb0", O_RDWR))) 
      {
        perror("open");
        return __LINE__;
      }
      if ( (-1) == ioctl(fb, FBIOGET_VSCREENINFO, &info)) 
      {
        perror("ioctl");
        close(fb);
        return __LINE__;
      }
      fb_size = sizeof(uint32_t) * info.xres_virtual * info.yres_virtual;
      map_size = (fb_size + (page_size - 1 )) & (~(page_size-1));
      srand(time(NULL));

      if((args[0]>info.xres_virtual)||(args[1])>info.yres_virtual)
      {
        printf("Max resolution of X %d\tY %d\n",info.xres_virtual,info.yres_virtual);
        munmap(ptr, map_size);
        close(fb);
        endwin();
        exit(0);
     }
      info.xres = args[0];
      info.yres = args[1];
      if( NULL == initscr()) {
          return __LINE__;
        }
      noecho();
      curs_set(2);
      keypad(stdscr,TRUE);
      ptr = mmap(NULL, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);
      if (MAP_FAILED == ptr) {
        perror("mmap");
        close(fb);
        return __LINE__;
      }
      for(int a = 0;a<=info.xres;a++)
      {
        for(int b = 0;b<=info.yres;b++)
	{
          ptr[b * info.xres_virtual + a]=0x00000000;
        }
      }
}
void FRAME_print()
{
  for(int a = 0;a<info.yres;a++)
  {
        ptr[a * info.xres_virtual] = FRAME;
      }
  for(int a=0;a<info.xres;a++)
  {
      ptr[a] = FRAME;
    }
  for(int a=0;a<info.xres;a++)
  {
    ptr[(info.yres-1) * info.xres_virtual + a] = FRAME;
  }
  for(int a=0;a<info.yres;a++)
  {
    ptr[a*info.xres_virtual +(info.xres-1)] = FRAME;
  }
}
void print_black_r(int x, int y)
{
  int cord_x[9];
  int cord_y[5];
  int a = x+1;
  int b = y;
  for(int i=0;i<9;i++)
  {
    cord_x[i] = a+i;
  }
  cord_y[0] = b-2;
  cord_y[1] = b-1;
  cord_y[2] = b-0;
  cord_y[3] = b+1;
  cord_y[4] = b+2;
  for(int i=0;i<9;i++)
  {
    for(int j=0;j<5;j++)
    {
      ptr[cord_y[j]*info.xres_virtual+cord_x[i]] = 0x00000000;
    }
  }
}
void print_black_l(int x, int y)
{
 int cord_x[9];
  int cord_y[5];
  int a = x-1;
  int b = y;
  for(int i=0;i<9;i++)
  {
    cord_x[i] = a-i;
  }
  cord_y[0] = b-2;
  cord_y[1] = b-1;
  cord_y[2] = b-0;
  cord_y[3] = b+1;
  cord_y[4] = b+2;
  for(int i = 0;i<9;i++)
  {
    for(int j = 0;j<5;j++)
    {
      ptr[cord_y[j]*info.xres_virtual+cord_x[i]] = 0x00000000;
    }
  }
}
void print_black_u(int x, int y)
{
  int cord_y[9];
  int cord_x[5];
  int a = x;
  int b = y-1;
  for(int i=0;i<9;i++)
  {
    cord_y[i] = b-i;
  }
  cord_x[0] = a-2;
  cord_x[1] = a-1;
  cord_x[2] = a-0;
  cord_x[3] = a+1;
  cord_x[4] = a+2;
  for(int i = 0;i<9;i++)
  {
    for(int j = 0;j<5;j++)
    {
      ptr[cord_y[i]*info.xres_virtual+cord_x[j]]=0x00000000;
    }
  }
}
void print_black_d(int x,int y)
{
  int cord_y[9];
  int cord_x[5];
  int a = x;
  int b = y+1;
  for(int i=0;i<9;i++)
  {
    cord_y[i] = b+i;
  }
  cord_x[0] = a-2;
  cord_x[1] = a-1;
  cord_x[2] = a-0;
  cord_x[3] = a+1;
  cord_x[4] = a+2;
  for(int i = 0;i<9;i++)
  {
    for(int j = 0;j<5;j++)
    {
      ptr[cord_y[i]*info.xres_virtual+cord_x[j]]=0x00000000;
    }
  }
}
void print_r(struct Bike *moto,int x,int y,uint32_t enemy_color)
{
  int cord_x[9];
  int tail[5];
  int a = x+1;
  int b = y;

  for(int i=0;i<9;i++)
  {
    cord_x[i] = a+i;
  }
  tail[0] = b-2;
  tail[1] = b-1;
  tail[2] = b-0;
  tail[3] = b+1;
  tail[4] = b+2;
  for(int i = 0;i<5;i++)
  {
    if( (ptr[tail[i]*info.xres_virtual + cord_x[8]] == enemy_color) ||
        (ptr[tail[i]*info.xres_virtual + cord_x[8]] == FRAME) ||
        (ptr[tail[i]*info.xres_virtual + cord_x[8]] == moto->color) ) 
    {
      moto->lose_flag = 1;
      win(moto);
      }
  }
  for(int i = 0;i<9;i++)
  {
    for(int j = 0;j<5;j++)
    {
      ptr[tail[j]*info.xres_virtual+cord_x[i]] = moto->color;
    }
  }
  ptr[tail[0]*info.xres_virtual+a] = 0x00000000;
  ptr[tail[1]*info.xres_virtual+a] = 0x00000000;
  ptr[tail[2]*info.xres_virtual+a] = moto->color;
  ptr[tail[3]*info.xres_virtual+a] = 0x00000000;
  ptr[tail[4]*info.xres_virtual+a] = 0x00000000;
  x=(a+9)%info.xres;

}
void print_l(struct Bike *moto,int x,int y,uint32_t enemy_color)
{
  int cord_x[9];
  int tail[5];
  int a = x-1;
  int b = y;

  for(int i=0;i<9;i++)
  {
    cord_x[i] = a-i;
  }
  tail[0] = b-2;
  tail[1] = b-1;
  tail[2] = b-0;
  tail[3] = b+1;
  tail[4] = b+2;
  for(int i = 0;i<5;i++)
  {
  if( (ptr[tail[i]*info.xres_virtual + cord_x[8]] == enemy_color) ||
        (ptr[tail[i]*info.xres_virtual + cord_x[8]] == FRAME) ||
        (ptr[tail[i]*info.xres_virtual + cord_x[8]] == moto->color) ) 
  {
      moto-> lose_flag = 1;
      win(moto);
      }
  }
  for(int i = 0;i<9;i++)
  {
    for(int j = 0;j<5;j++)
    {
      ptr[tail[j]*info.xres_virtual+cord_x[i]] = moto->color;
    }
  }
  ptr[tail[0]*info.xres_virtual+a] = 0x00000000;
  ptr[tail[1]*info.xres_virtual+a] = 0x00000000;
  ptr[tail[2]*info.xres_virtual+a] = moto->color;
  ptr[tail[3]*info.xres_virtual+a] = 0x00000000;
  ptr[tail[4]*info.xres_virtual+a] = 0x00000000;

  x=(a+9)%info.xres;

}
void print_u(struct Bike *moto,int x,int y,uint32_t enemy_color)
{
  int cord_y[9];
  int tail[5];
  int a = x;
  int b = y-1;
  for(int i=0;i<9;i++)
  {
    cord_y[i] = b-i;
  }
  tail[0] = a-2;
  tail[1] = a-1;
  tail[2] = a-0;
  tail[3] = a+1;
  tail[4] = a+2;
  for(int i = 0;i<5;i++)
  {
    if( (ptr[cord_y[8]*info.xres_virtual + tail[i]] == enemy_color) ||
        (ptr[cord_y[8]*info.xres_virtual + tail[i]] == FRAME) ||
        (ptr[cord_y[8]*info.xres_virtual + tail[i]] == moto->color) ) 
    {
      moto-> lose_flag = 1;
      win(moto);
      }
  }
  for(int i = 0;i<9;i++)
  {
    for(int j = 0;j<5;j++)
    {
      ptr[cord_y[i]*info.xres_virtual+tail[j]]=moto->color;
    }
  }
  ptr[b*info.xres_virtual+tail[0]] = 0x00000000;
  ptr[b*info.xres_virtual+tail[1]] = 0x00000000;
  ptr[b*info.xres_virtual+tail[2]] = moto->color;
  ptr[b*info.xres_virtual+tail[3]] = 0x00000000;
  ptr[b*info.xres_virtual+tail[4]] = 0x00000000;
  y = (b - 9)%info.yres;
}
void print_d(struct Bike *moto,int x,int y,uint32_t enemy_color)
{
  int cord_y[9];
  int tail[5];
  int a = x;
  int b = y+1;
  for(int i=0;i<9;i++)
  {
    cord_y[i] = b+i;
  }
  tail[0] = a-2;
  tail[1] = a-1;
  tail[2] = a-0;
  tail[3] = a+1;
  tail[4] = a+2;
  for(int i = 0;i<5;i++)
  {
    if( (ptr[cord_y[8]*info.xres_virtual + tail[i]] == enemy_color) ||
        (ptr[cord_y[8]*info.xres_virtual + tail[i]] == FRAME) ||
        (ptr[cord_y[8]*info.xres_virtual + tail[i]] == moto->color) ) 
    {
      moto-> lose_flag = 1;
      win(moto);
      }
  }
  for(int i = 0;i<9;i++)
  {
    for(int j = 0;j<5;j++)
    {
      ptr[cord_y[i]*info.xres_virtual+tail[j]]=moto->color;
    }
  }
  ptr[b*info.xres_virtual+tail[0]] = 0x00000000;
  ptr[b*info.xres_virtual+tail[1]] = 0x00000000;
  ptr[b*info.xres_virtual+tail[2]] = moto->color;
  ptr[b*info.xres_virtual+tail[3]] = 0x00000000;
  ptr[b*info.xres_virtual+tail[4]] = 0x00000000;
  y = (b + 9)%info.yres;
}
int print(struct Bike *moto,int *x,int *y,uint32_t enemy_color)
{
    if(moto->side == (1))
    {
      if(moto->key == 's') print_black_d(*x,*y);
      if(moto->key == 'a') print_black_l(*x,*y);
      if(moto->key == 'd') print_black_r(*x,*y);
      moto -> key = 'w';
      *y = (*y + info.yres - 1)%info.yres;
    }
    if(moto->side == (2))
    {
      if(moto->key == 'w') print_black_u(*x,*y);
      if(moto->key == 'a') print_black_l(*x,*y);
      if(moto->key == 'd') print_black_r(*x,*y);
      moto -> key = 's';
      *y = (*y+1)%info.yres;
    }
    if(moto->side == (3))
    {
      if(moto->key == 'w') print_black_u(*x,*y);
      if(moto->key == 's') print_black_d(*x,*y);
      if(moto->key == 'd') print_black_r(*x,*y);
      moto -> key = 'a';
      *x = (*x + info.xres -1)%info.xres;
    }
    if(moto->side == (4))
    {
      if(moto->key == 'w') print_black_u(*x,*y);
      if(moto->key == 's') print_black_d(*x,*y);
      if(moto->key == 'a') print_black_l(*x,*y);
      moto -> key = 'd';
      *x = (*x+1)%info.xres;
    }

      ptr[*y * info.xres_virtual + *x] = moto->color;
    FRAME_print();

    if(moto->side == 1)
    {
      print_u(moto,*x,*y,enemy_color);
    }
    if(moto->side == 2)
    {
      print_d(moto,*x,*y,enemy_color);
    }
    if(moto->side == 3 )
    {
      print_l(moto,*x,*y,enemy_color);
    }
    if(moto->side == 4)
    {
      print_r(moto,*x,*y,enemy_color);
    }
}
void* init_bikes_and_cycle(void *args)
{
  if(sum2>sum1)
  {
    fprintf(f,"LEFT %d sum1\t %d sum2\n",sum1,sum2);
  Bike first;
  first.x = info.xres - 10;
  first.y = info.yres - 30;
  first.color = 0x01009977;
  first.side = 0;
  first.key = '.';
  first.lose_flag = 0;
  Bike second;
  second.x = 10;
  second.y = 10;
  second.color = 0xff4500;
  second.side = 0;
  second.key = '.';
  second.lose_flag = 0;
  way = first.side;
  way2 = second.side;
  first.name = 'B';
  second.name = 'R';

  pthread_create(&line_key1,NULL, first_bike_press,NULL);
  pthread_create(&line_key2,NULL, second_bike_press,NULL);
  pthread_create(&line, NULL, firstkeypress, &first);
  pthread_create(&line2, NULL, secondkeypress, &second);

  while(working_flag)
  {
    print(&first,&first.x,&first.y,second.color);
    print(&second,&second.x,&second.y,first.color);
    i++;
    usleep(62500);
  }
  pthread_join(line_key1,NULL);
  pthread_join(line_key2,NULL);
  pthread_join(line,NULL);
  pthread_join(line2,NULL);
  }
 else if(sum1>sum2)
 {
   fprintf(f,"RIGht %d sum1\t %d sum2\n",sum1,sum2);
  Bike first;
  first.x = 10;
  first.y = 10;
  first.color = 0xff4500;
  first.side = 0;
  first.key = '.';
  first.lose_flag = 0;
  Bike second;
  second.x = info.xres - 10;
  second.y = info.yres - 30;
  second.color = 0x01009977;
  second.side = 0;
  second.key = '.';
  second.lose_flag = 0;
  way = first.side; 
  way2 = second.side;
  first.name = 'R';
  second.name = 'B';
  pthread_t line, line2, line_key1, line_key2;
  pthread_create(&line_key1,NULL, first_bike_press,NULL);
  pthread_create(&line_key2,NULL, second_bike_press,NULL);
  pthread_create(&line, NULL, firstkeypress, &first);
  pthread_create(&line2, NULL, secondkeypress, &second);

  while(working_flag)
  {
    print(&first,&first.x,&first.y,second.color);
    print(&second,&second.x,&second.y,first.color);
    i++;
    usleep(62500);
  }
  pthread_join(line_key1,NULL);
  pthread_join(line_key2,NULL);
  pthread_join(line,NULL);
  pthread_join(line2,NULL);
 }
}

int main(int argc, char *argv[])
{

  signal(SIGINT, handler);
  int args[2];
  if(argc < 5)
  {
    printf("Usage: %s Xres Yres Your_ip Enemy_ip\n",argv[0]);
    exit(0);
  }
  args[0] = atoi(argv[1]);
  args[1] = atoi(argv[2]);

 sscanf(argv[3],"%d.%d.%d.%d",&ar1[0],&ar1[1],&ar1[2],&ar1[3]);
 sscanf(argv[4],"%d.%d.%d.%d",&ar2[0],&ar2[1],&ar2[2],&ar2[3]);
 sum1 = ar1[0]+ar1[1]+ar1[2]+ar1[3];
 sum2 = ar2[0]+ar2[1]+ar2[2]+ar2[3];
 f = fopen("ORRO.txt","w");
 fprintf(f,"MAIN %d sum1\t %d sum2\n",sum1,sum2);

  if(sum1 > sum2)
  { 
	  right = 1;left = 0;
  }
  else if (sum1 < sum2)
  { 
	  left = 1;right = 0;
  }

  initialization(args,argv);

  if ( (my_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) 
  {
        perror("socket create failed");
        munmap(ptr, map_size);
        endwin();
        exit(EXIT_FAILURE);
    }
    me.sin_family = AF_INET;
    me.sin_addr.s_addr = inet_addr((argv[3]));
    me.sin_port = htons(12345);
    if ( bind(my_fd, (const struct sockaddr*)&me,sizeof(me)) < 0 )
    {
        perror("bind failed");
        munmap(ptr, map_size);
        endwin();
        exit(EXIT_FAILURE);
    }
     if ( (other_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) 
    {
        perror("socket create failed");
        munmap(ptr, map_size);
        endwin();
        exit(EXIT_FAILURE);
    }
    other.sin_family = AF_INET;
    other.sin_addr.s_addr = inet_addr((argv[4]));
    other.sin_port = htons(12345);

  pthread_t line_cycle;
  pthread_create(&line_cycle, NULL, init_bikes_and_cycle, NULL);
  pthread_join(line_cycle,NULL);
  munmap(ptr, map_size);
  close(fb);
  fclose(f);
  endwin();
  return 0;
}

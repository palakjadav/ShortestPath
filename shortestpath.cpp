/*
 AIM     :- Among a set of traingles obstacles, displays the shortest path between two input points.
 
 INPUT   :- I) With Command-line Argument:
                a) At run time, the user enters the file name that contains the traingle obstacles, one per line and each line starts with a T.
                b) User may draw additional set of traingle obstacles in the window. A vertice of a triangle may not fall inside a triangle.
                c) Once all the traingle obstacles are ready, user enters the right mouse click. (This indicates the target of all traingle obstacles)
                d) With left mouse clicks, the user now enters a set of two points, one as the start point and the other as a target point.
                e) To exit the program, the user enters the right mouse click.
            
            II) Without Command-line Argument:
                a) User draws the triangle obstacles on the window, through left mouse clicks. A vertice of a triangle may not fall inside a triangle.
                b) Once all the traingle obstacles are ready, user enters the right mouse click. (This indicates the target of all traingle obstacles)
                c) With left mouse clicks, the user now enters a set of two points, one as the start point and the other as a target point.
                d) To exit the program, the user enters the right mouse click.
 
 OUTPUT  :- Among a set of traingles obstacles, displays the shortest path between two input points.
 
 To COMPILE the program :- g++ -o path shortestpath.cpp -I/usr/X11R6/include -L/usr/X11R6/lib -lX11
 To RUN the program :- ./path input.txt
*/

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#define infinity 9999
using namespace std;

Display *display_ptr;
Screen *screen_ptr;
int screen_num;
char *display_name = NULL;
unsigned int display_width, display_height;

Window win;
int border_width;
unsigned int win_width, win_height;
int win_x, win_y;

XWMHints *wm_hints;
XClassHint *class_hints;
XSizeHints *size_hints;
XTextProperty win_name, icon_name;
char *win_name_string = "Shortest Path";
char *icon_name_string = "Icon for Shortest Path";

XEvent report;

GC gc, gc_light_blue, gc_dark_blue, gc_violet;
unsigned long valuemask = 0;
XGCValues gc_values, gc_light_blue_values, gc_dark_blue_values, gc_violet_values;
Colormap color_map;
XColor tmp_color1, tmp_color2;

struct Point
{
    int x;
    int y;
};

struct Edge
{
    struct Point v1;
    struct Point v2;
};

struct Triangle
{
    struct Point p;
    struct Point q;
    struct Point r;
};
int getDistance(struct Edge e);
int isInTriangle(struct Point pt , struct Triangle tri);
int pointOrientation(struct Point pt, struct Point a, struct Point b);
int isLineIntersect(struct Point p1,struct Point p2,struct Point p3, struct Point p4);
int isEdgeIntersect(struct Point v1,struct Point v2,struct Triangle triangle);
void dijkstraAlgo(struct Triangle *triangle, struct Edge *edge, struct Point start, struct Point target, int noOfTri, int noOfPath);
int vaildPath(struct Triangle *triangle, int t, struct Edge *edge);
void computePath(struct Triangle *triangle, struct Point startPt, struct Point targetPt, int tCount);

int main(int argc, char* argv[])
{
    
  /* opening display: basic connection to X Server */
  if( (display_ptr = XOpenDisplay(display_name)) == NULL )
    { printf("Could not open display. \n"); }
  printf("Connected to X server  %s\n", XDisplayName(display_name) );
  screen_num = DefaultScreen( display_ptr );
  screen_ptr = DefaultScreenOfDisplay( display_ptr );
  color_map  = XDefaultColormap( display_ptr, screen_num );
  display_width  = DisplayWidth( display_ptr, screen_num );
  display_height = DisplayHeight( display_ptr, screen_num );

  printf("Width %d, Height %d, Screen Number %d\n", 
           display_width, display_height, screen_num);
  /* creating the window */
  
  
  /*win_width = display_width/2;
  win_height = (int) (win_width / 1.7); rectangular window*/
    
    win_width = DisplayWidth(display_ptr, screen_num) / 1.7;
    win_height = DisplayHeight(display_ptr, screen_num) / 1.2;
    border_width = 10;
    win_x = 10; win_y = 10;

  win= XCreateSimpleWindow( display_ptr, RootWindow( display_ptr, screen_num),
                            win_x, win_y, win_width, win_height, border_width,
                            BlackPixel(display_ptr, screen_num),
                            WhitePixel(display_ptr, screen_num) );
  /* now try to put it on screen, this needs cooperation of window manager */
  size_hints = XAllocSizeHints();
  wm_hints = XAllocWMHints();
  class_hints = XAllocClassHint();
  if( size_hints == NULL || wm_hints == NULL || class_hints == NULL )
    { printf("Error allocating memory for hints. \n"); }

  size_hints -> flags = PPosition | PSize | PMinSize  ;
  size_hints -> min_width = 60;
  size_hints -> min_height = 60;

  XStringListToTextProperty( &win_name_string,1,&win_name);
  XStringListToTextProperty( &icon_name_string,1,&icon_name);
  
  wm_hints -> flags = StateHint | InputHint ;
  wm_hints -> initial_state = NormalState;
  wm_hints -> input = False;

  class_hints -> res_name = "x_use_example";
  class_hints -> res_class = "examples";

  XSetWMProperties( display_ptr, win, &win_name, &icon_name, argv, argc,
                    size_hints, wm_hints, class_hints );

  /* what events do we want to receive */
  XSelectInput( display_ptr, win, 
            ExposureMask | StructureNotifyMask | ButtonPressMask );
  
  /* finally: put window on screen */
  XMapWindow( display_ptr, win );

  XFlush(display_ptr);

  /* create graphics context, so that we may draw in this window */
  gc = XCreateGC( display_ptr, win, valuemask, &gc_values);
  XSetForeground( display_ptr, gc, BlackPixel( display_ptr, screen_num ) );
  XSetLineAttributes( display_ptr, gc, 2, LineSolid, CapRound, JoinRound);
  /* and three other graphics contexts, to draw in light_blue and dark_blue and violet*/
    
  gc_light_blue = XCreateGC( display_ptr, win, valuemask, &gc_light_blue_values);
  XSetLineAttributes(display_ptr, gc_light_blue, 2, LineSolid,CapRound, JoinRound);
  if( XAllocNamedColor( display_ptr, color_map, "#6495ED",
			&tmp_color1, &tmp_color2 ) == 0 )
    {printf("failed to get color light_blue\n");exit(-1); }
  else
    XSetForeground( display_ptr, gc_light_blue, tmp_color1.pixel );
    
  gc_dark_blue = XCreateGC( display_ptr, win, valuemask, &gc_dark_blue_values);
  XSetLineAttributes( display_ptr, gc_dark_blue, 2, LineSolid, CapRound, JoinRound);
  if( XAllocNamedColor( display_ptr, color_map, "#191970",
			&tmp_color1, &tmp_color2 ) == 0 )
    {printf("failed to get color dark_blue\n"); exit(-1);}
  else
    XSetForeground( display_ptr, gc_dark_blue, tmp_color1.pixel );
  
  gc_violet = XCreateGC( display_ptr, win, valuemask, &gc_violet_values);
  XSetLineAttributes( display_ptr, gc_violet, 3, LineSolid, CapRound, JoinRound);
  if( XAllocNamedColor( display_ptr, color_map, "#C71585",
			&tmp_color1, &tmp_color2 ) == 0 )
    {printf("failed to get color violet\n");exit(-1); }
  else
    XSetForeground( display_ptr, gc_violet, tmp_color1.pixel );

    static int input =-1, input2 = 0;
    int i=0,j=0,vCount=0,tCount=0,triVertice=0, *triCo;
    triCo=new int[3000];
    struct Triangle triangle[1000];
    struct Point start, target;
    char line[1024];
    
    char *str;
    FILE *fp;
    fp = fopen(argv[1], "r");

    if (fp == NULL)
    {
        printf("\n\033[1m\033[30mProgram Executed w/o an Input File\033[0m\n");
        printf("\n\033[1m\033[34mDraw Traingles with Left Mouse Clicks\033[0m\n");
    }
    else
    {
        printf("\n\033[1m\033[30mProgram Executed with an Input File\033[0m\n\n");
        while (fgets(line, 1024, fp) != NULL)
        {
            str = strtok(line, "ABCDEFGHIJKLMNOPQRSTUVWXYZ,() \n");
            while (str != NULL)
            {
                triCo[vCount]=atoi(str); //array of vertices
                str = strtok(NULL, "ABCDEFGHIJKLMNOPQRSTUVWXYZ,\n ()");
                vCount++;
            }
            tCount++; // counts number of triangle
        }
        for(j=0;j<tCount;j++)
        {
            triangle[j].p.x = triCo[i],triangle[j].p.y = triCo[i+1];
            triangle[j].q.x = triCo[i+2],triangle[j].q.y = triCo[i+3];
            triangle[j].r.x = triCo[i+4],triangle[j].r.y = triCo[i+5];
            i = i + 6;
        }
        fclose(fp);
    }

  /* and now it starts: the event loop */
  while(1)
    { XNextEvent( display_ptr, &report );
      switch( report.type )
	{
	case Expose:
          /* (re-)draw the example figure. This event happens
             each time some part ofthe window gets exposed (becomes visible) */
        {
            int i=0, j=0;
            int *v=triCo;

            for(j=0;j<tCount;j++)
            {
                {   XPoint points[] =
                    {
                        {v[i],v[i+1]},
                        {v[i+2],v[i+3]},
                        {v[i+4],v[i+5]},
                        {v[i],v[i+1]}
                    };
                    int npoints = sizeof(points)/sizeof(XPoint);
                    XDrawLines(display_ptr, win, gc_light_blue, points, npoints, CoordModeOrigin);
                };
                i = i + 6;
            }
        }
        break;
        case ConfigureNotify:
          /* This event happens when the user changes the size of the window*/
          win_width = report.xconfigure.width;
          win_height = report.xconfigure.height;
          break;
        case ButtonPress:
          /* This event happens when the user pushes a mouse button.  */
        {
            int x1=0,y1=0;
            int x[3],y[3];
            bool vertice;
            x1 = report.xbutton.x;
            y1 = report.xbutton.y;
            
            //User draws traingles with left mouse click
            if(input== -1)
            {
                if(report.xbutton.button == Button1)
                {
                    x[(triVertice%3)] = x1;
                    y[(triVertice%3)] = y1;
                    triVertice++;
                   
                    XFillArc( display_ptr, win, gc_light_blue,
                             x1 , y1 ,
                             4, 4, 0, 360*64);

                    if((triVertice%3) == 0)
                    {
                        XDrawLine(display_ptr, win, gc_light_blue, x[0], y[0], x[1], y[1]);
                        XDrawLine(display_ptr, win, gc_light_blue, x[1], y[1], x[2], y[2]);
                        XDrawLine(display_ptr, win, gc_light_blue, x[2], y[2], x[0], y[0]);
                        triangle[tCount].p.x = x[0], triangle[tCount].p.y = y[0];
                        triangle[tCount].q.x = x[1], triangle[tCount].q.y = y[1];
                        triangle[tCount].r.x = x[2], triangle[tCount].r.y = y[2];
                        tCount++;
                    }
                }
                else
                {
                    if(input2 == 0)
                    {
                        input=0,input2=1;
                    }
                    else
                    { exit(-1); }
                   
                    
                }
            }
            
            //Start Point
            else if(input==0)
            {
                if (report.xbutton.button == Button1)
                {
                    start.x = x1;
                    start.y = y1;
                    for(i=0;i<tCount;i++)
                    {
                        vertice = isInTriangle(start,triangle[i]);
                        if(vertice)
                        {
                            printf("\n\033[1m\033[31mError! Vertice cannot be inside a triangle!\033[0m \n");
                            exit(-1);
                        }
                    }
                  
                    XFillArc( display_ptr, win, gc_dark_blue,
                             start.x-win_height/90 , start.y-win_height/90,
                             win_height/70, win_height/70, 0, 360*64);
                    input=1;
                }
                else
                {
                    if(input2 == 1)
                    {
                    printf("\n\033[1m\033[32mProgram Quits with Right Mouse CLick!\033[0m\n\n");
                        exit(-1);}
                }
            }
            
            //Target Point
            else
            {
                if(report.xbutton.button == Button1 )
                {
                    target.x = x1;
                    target.y = y1;
                    for(i=0;i<tCount;i++)
                    {
                        vertice = isInTriangle(target,triangle[i]);
                        if(vertice)
                        {
                            printf("\n\033[1m\033[31mError! Vertice cannot be inside a triangle!\033[0m\n");
                            exit(-1);
                        }
                    }
                    XFillArc( display_ptr, win, gc_dark_blue,
                             target.x-win_height/90 , target.y-win_height/90,
                             win_height/70, win_height/70, 0, 360*64);

                    computePath(triangle, start, target, tCount);
                    input=0;
                }
                else
                {
                    if(input2 == 1){
                    printf("\n\033[1m\033[32mProgram Quits with Right Mouse CLick!\033[0m\n\n");
                        exit(-1);}
                }
            }
        }
        break;
        default:
	  /* this is a catch-all for other events; it does not do anything.
             One could look at the report type to see what the event was */ 
          break;
	}
    }
    return 0;
}




//To calculate length of an edge
int getDistance(struct Edge e)
{
    int diff1,diff2,diff1_sqr,diff2_sqr,distance;
    diff1 = (e.v1.x) - (e.v2.x);
    diff2 = (e.v1.y) - (e.v2.y);
    diff1_sqr = pow(diff1,2);
    diff2_sqr = pow(diff2,2);
    distance = sqrt(diff1_sqr + diff2_sqr);
    return distance;
}

//To check whether a point falls inside or outside the traingle obstacles
int isInTriangle(struct Point pt , struct Triangle tri)
{
    int d,e,f;
    d=((pt.x-tri.p.x) * (tri.q.y-tri.p.y)) - ((pt.y-tri.p.y) * (tri.q.x-tri.p.x));
    e=((pt.x-tri.q.x) * (tri.r.y-tri.q.y)) - ((pt.y-tri.q.y) * (tri.r.x-tri.q.x));
    f=((pt.x-tri.r.x) * (tri.p.y-tri.r.y)) - ((pt.y-tri.r.y) * (tri.p.x-tri.r.x));
    
    if((d>0) && (e>0) && (f>0)) return 1;
    else if((d<0) && (e<0) && (f < 0)) return 1;
    else return 0;
}

//To return the orientation of a point with respect to line
int pointOrientation(struct Point pt, struct Point a, struct Point b)
{
    int c=(a.y-pt.y) * (b.x-a.x) - (a.x-pt.x) * (b.y-a.y);
    return (c>0)?0:1;
}

//To check whether two lines intersect each other
int isLineIntersect(struct Point p1,struct Point p2,struct Point p3, struct Point p4)
{
    int a,b,c,d;
    a = pointOrientation(p1, p2, p3);
    b = pointOrientation(p1, p2, p4);
    c = pointOrientation(p3, p4, p1);
    d = pointOrientation(p3, p4, p2);
    if (a!=b && c!=d) return 1;
    else return 0;
}

//To check whether line between two vertices intersect an aedge of the triangle obstacle
int isEdgeIntersect(struct Point v1,struct Point v2,struct Triangle triangle)
{
    int r1 = 0,r2 = 0,r3 = 0;
    r1 = isLineIntersect(v1,v2,triangle.p,triangle.q);
    r2 = isLineIntersect(v1,v2,triangle.q,triangle.r);
    r3 = isLineIntersect(v1,v2,triangle.r,triangle.p);
    if((r1 == r2) && (r2 == r3)) return 1;
    else return 0;
}


//To form a path between vertices such that it does not intersect with triangle edges and keep the vaild path between start and target point
void computePath(struct Triangle *triangle, struct Point startPt, struct Point targetPt, int tCount)
{
    int intersect=0, m=0, n=0, a=0 ;
    int i=0, j=0, k=0, count=0, noOfValidPath=0, noOfPath=0, notintersect = 0;
    count = tCount;
    struct Point z;
    struct Edge edge[5000];
    noOfPath = vaildPath(triangle, tCount, edge);
    noOfValidPath=noOfPath;
    
    for(i = 0; i < count; i++)
    {
        int vertice=isEdgeIntersect(startPt,targetPt,triangle[i]);
        if(!vertice) {notintersect = 0; break;}
        else notintersect=1;
    }
    if(notintersect==1)
    {
        edge[noOfValidPath].v1.x=startPt.x,
        edge[noOfValidPath].v1.y=startPt.y;
        edge[noOfValidPath].v2.x=targetPt.x,
        edge[noOfValidPath].v2.y=targetPt.y;
        noOfValidPath++;
    }
    for(k=0;k<2;k++)
    {
        z=(k == 0)?startPt:targetPt;
        for(i=0;i<count;i++)
        {
            intersect = isLineIntersect(z,triangle[i].p,triangle[i].q,triangle[i].r);
            if(!intersect)
            {
                for(j=0;j<count;j++)
                {
                    if(i!=j)
                    {
                        n = isInTriangle(triangle[i].p,triangle[j]);
                        m = isEdgeIntersect(z,triangle[i].p,triangle[j]);
                        if (!m||n) { a=0; break; }
                        else a=1;
                    }
                }
                if(a==1)
                {
                    edge[noOfValidPath].v1.x = z.x , edge[noOfValidPath].v1.y = z.y;
                    edge[noOfValidPath].v2.x = triangle[i].p.x , edge[noOfValidPath].v2.y = triangle[i].p.y;
                    noOfValidPath++;
                }
            }
            intersect=isLineIntersect(z,triangle[i].q,triangle[i].r,triangle[i].p);
            if(!intersect)
            {
                for(j=0;j<count;j++)
                {
                    if(i!=j)
                    {
                        n = isInTriangle(triangle[i].q,triangle[j]);
                        m = isEdgeIntersect(z,triangle[i].q,triangle[j]);
                        if (!m||n) { a=0; break; }
                        else a=1;
                    }
                }
                if(a==1)
                {
                    edge[noOfValidPath].v1.x = z.x , edge[noOfValidPath].v1.y = z.y;
                    edge[noOfValidPath].v2.x = triangle[i].q.x , edge[noOfValidPath].v2.y = triangle[i].q.y;
                    noOfValidPath++;
                }
            }
            intersect = isLineIntersect(z,triangle[i].r,triangle[i].p,triangle[i].q);
            if(!intersect)
            {
                for(j=0; j<count; j++)
                {
                    if(i!=j)
                    {
                        n = isInTriangle(triangle[i].r,triangle[j]);
                        m = isEdgeIntersect(z,triangle[i].r,triangle[j]);
                        if (!m||n) { a=0; break; }
                        else a=1;
                    }
                }
                if(a==1)
                {
                    edge[noOfValidPath].v1.x=z.x,
                    edge[noOfValidPath].v1.y=z.y;
                    edge[noOfValidPath].v2.x=triangle[i].r.x,
                    edge[noOfValidPath].v2.y=triangle[i].r.y;
                    noOfValidPath++;
                }
            }
        }
    }
    dijkstraAlgo(triangle,edge,startPt,targetPt,tCount,noOfValidPath);
}

//To connect the vertices of traingles who form a valid path
int vaildPath(struct Triangle *triangle, int t, struct Edge *edge)
{
    int temp=0,m=0,o=0,triCount=0,i=0,j=0,u=0,noOfValidPath=0,verCount;
    triCount=t;
    verCount=(triCount * 3) ;
    struct Point v[verCount];
    for(i=0; i<triCount; i++)
    {
        v[j].x = triangle[i].p.x, v[j].y = triangle[i].p.y;
        v[j+1].x = triangle[i].q.x, v[j+1].y = triangle[i].q.y;
        v[j+2].x = triangle[i].r.x, v[j+2].y = triangle[i].r.y;
        j = j + 3;
    }
    for (i=0; i<verCount; i++)
    {
        int outside=1;
        for(j=0; j<triCount; j++)
        {
            if(((v[i].x != triangle[j].p.x) && (v[i].y != triangle[j].p.y))||
               ((v[i].x != triangle[j].q.x) && (v[i].y != triangle[j].q.y))||
               ((v[i].x != triangle[j].r.x) && (v[i].y != triangle[j].r.y)))
            {
                o = isInTriangle(v[i],triangle[j]);
                if(o) outside = 0;
            }
        }
        if(!outside) {v[i].x = infinity, v[i].y = infinity;}
    }
    for(i=0; i<verCount; i++)
    {
        for(j=0; j<verCount; j++)
        {
            if(i!=j)
            {
                temp=1;
                for(u = 0; u < triCount; u++)
                {
                    if((v[i].x == triangle[u].p.x) && (v[i].y == triangle[u].p.y))
                    {
                        m = isLineIntersect(v[i], v[j], triangle[u].q, triangle[u].r);
                        if((m == 0) && (temp == 1)) temp = 1;
                        else temp = 0;
                    }
                    else if((v[i].x == triangle[u].q.x) && (v[i].y == triangle[u].q.y))
                    {
                        m = isLineIntersect(v[i], v[j], triangle[u].r, triangle[u].p);
                        if((m==0) &&(temp == 1)) temp = 1;
                        else temp = 0;
                    }
                    else if((v[i].x == triangle[u].r.x) && (v[i].y == triangle[u].r.y))
                    {
                        m = isLineIntersect(v[i],v[j],triangle[u].p,triangle[u].q);
                        if((m==0) && (temp == 1)) temp = 1;
                        else temp = 0;
                    }
                    else if((v[j].x == triangle[u].p.x) && (v[j].y == triangle[u].p.y))
                    {
                        m = isLineIntersect(v[i],v[j],triangle[u].q,triangle[u].r);
                        if((m==0) && (temp == 1)) temp = 1 ;
                        else temp = 0 ;
                    }
                    else if((v[j].x == triangle[u].q.x) && (v[j].y == triangle[u].q.y))
                    {
                        m = isLineIntersect(v[i],v[j],triangle[u].r,triangle[u].p);
                        if((m==0) && (temp==1)) temp = 1;
                        else temp=0;
                    }
                    else if((v[j].x == triangle[u].r.x) && (v[j].y == triangle[u].r.y))
                    {
                        m = isLineIntersect(v[i],v[j],triangle[u].p,triangle[u].q);
                        if ((m==0) && (temp==1)) temp = 1;
                        else temp = 0;
                    }
                    else
                    {
                        m = isEdgeIntersect(v[i],v[j],triangle[u]);
                        if ((m == 1) && (temp==1)) temp = 1;
                        else temp = 0;
                    }
                }
                if(temp==1)
                {
                    if(((v[i].x!=infinity) && (v[i].y!=infinity) )
                       && ((v[j].x!=infinity)&& (v[j].y!=infinity)))
                    {
                        edge[noOfValidPath].v1.x=v[i].x , edge[noOfValidPath].v1.y=v[i].y;
                        edge[noOfValidPath].v2.x=v[j].x , edge[noOfValidPath].v2.y=v[j].y;
                        noOfValidPath++;
                    }
                }
            }
        }
    }
    return noOfValidPath;
}

//To find the shortest path between start and target points
void dijkstraAlgo(struct Triangle *triangle, struct Edge *edge, struct Point start, struct Point target, int noOfTri, int noOfPath)
{
    int noOfNode = (3*(noOfTri))+2;
    int i=0, j=0, k=0, s=0, d=0, nNum=0;
    int cost[noOfNode][noOfNode];
    
    struct Point node[noOfNode];
    
    
    node[nNum].x=start.x,node[nNum].y=start.y,s = nNum;
    nNum++;
    node[nNum].x=target.x,node[nNum].y=target.y,d=nNum;
    nNum++;
    for(i=0; i<(noOfTri) ;i++)
    {
        node[nNum].x = triangle[i].p.x, node[nNum].y = triangle[i].p.y;
        node[nNum+1].x = triangle[i].q.x, node[nNum+1].y = triangle[i].q.y;
        node[nNum+2].x = triangle[i].r.x, node[nNum+2].y = triangle[i].r.y;
        nNum = nNum + 3;
    }
    for(i=0; i<noOfNode; i++)
    {
        for(j=0; j<noOfNode; j++)
            cost[i][j] = infinity;
    }
    for(i=0; i<noOfNode; i++)
    {
        for(j=0; j<noOfNode; j++)
        {
            for(k=0; k<noOfPath; k++)
            {
                if(((node[i].x == edge[k].v1.x) && (node[i].y == edge[k].v1.y))
                   &&((node[j].x == edge[k].v2.x) && (node[j].y == edge[k].v2.y)))
                {
                    cost[i][j]=cost[j][i]=getDistance(edge[k]);
                }
            }
        }
    }
    
    int dist[noOfNode], previous[noOfPath], currNode[noOfNode], sPoint;
    struct Point path[noOfPath];
    int pos,minimum,l;
    for(i=0; i<noOfNode; i++)
    {
        dist[i]=infinity;
        currNode[i]=0;
        previous[i]=-1;
    }
    sPoint=s;
    currNode[sPoint]=1;
    dist[sPoint]=0;
    
    while(currNode[d]==0)
    {
        minimum=infinity;
        pos=0;
        for(i=0; i<noOfNode; i++)
        {
            l=dist[sPoint]+cost[sPoint][i];
            if(l<dist[i] && currNode[i]==0)
            {
                dist[i] = l;
                previous[i] = sPoint;
            }
            if(minimum>dist[i] && currNode[i]==0)
            {
                minimum = dist[i];
                pos = i;
            }
        }
        sPoint = pos;
        currNode[sPoint]=1;
    }
    sPoint=d;
    j=0;
    while(sPoint!=-1)
    {
        path[j].x = node[sPoint].x, path[j].y = node[sPoint].y ;
        sPoint = previous[sPoint];
        ++j;
    }
    for(i=0;i<j;i++)
    {
        k=i+1;
        if(k!=j)
        {
            XDrawLine(display_ptr,win,gc_violet,path[i].x,path[i].y,path[k].x,path[k].y);
            XDrawString(display_ptr, win, gc_violet, 10, 360, "*Represents the shortest distance between the two points", 56);
        }
    }
}


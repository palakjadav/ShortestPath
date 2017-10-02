# ShortestPath

The aim of this project is to create a program that takes a set of triangle obstacles as
input, as well as a start and target point, and shows the shortest path between them. It
provides a visual feedback by drawing the points, obstacles, and the path on the screen
using the xlib interface.

  The program takes either one command-line argument, a file name, or enters the
triangle obstacles interactively (both must be supported). If a file with triangle obstacles
is given, the file contains obstacles, one per line, each line starting with a T, as in the
following format example:
T (20,100) (55,63) (30,50)
The point coordinates are integers. You can parse these files easily with scanf. There
are less than 1000 obstacles.

  If no file name is give, the program just opens a window and takes left mouse-click
events as the vertices of triangle obstacles. Each time three points were entered, the
triangle is displayed. When a right mouse-click event is received, this ends the triangle
input; if there are one or two points of an unfinished triangle left, they are discarded.

  As first stage, if there was an input file, you read the input, find a bounding box (give
10% added space around the boundary), open a window, and display the obstacles in a
it. Else you open a window and take the triangle input. In either case, after all obstacles
have been defined and displayed, you need to get two left mouse-click events, which give
the start and target point of the shortest path. Then you construct a graph; the vertices
are the two points given by the mouseclicks, and all vertices of obstacles. Any pair (p, q)
of these points forms an edge of the graph if the line segment pq is not intersected by any
of the obstacles; if (p, q) is an edge, then its length is the euclidean distance of p and q.
Finally you run a shortest-path algorithm on this graph, and display the resulting path
on the screen. Repeat this as long as the user inputs point pairs with left mouseclicks;
when you receive a right mouseclick, end the program. Redraw the triangles (and path)
whenever you get an ‘expose’ event.

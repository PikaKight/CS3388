# Assignment 3 - Marcus Tuen Muk

## Adding Node

Adding a node is done when the user clicks on a blank space in the window. The callback functions are used to get the cursor position and passes it to the `add_Node` function.

The function sets a new Node variable called newNode and sets its property.

newNode:
    x -> cursor's x position
    y -> cursor's y position
    handle1.x -> cursor's x position
    handle1.y -> cursor's y position + 50
    hasHandle1 -> true
    hasHandle2 -> false

If the vector `nodes` has less than 2 nodes, then newNode is added to the vector and is one of the endpoints.

If nodes has at least 2 nodes, then the code checks which endpoint is closer to newNode by using `calc_dist` to get the distance.

Calc_dist calculates the difference between the 2 point's x and y coordinates and uses the formula $\sqrt{dx^2 + dy^2}$, where dx and dy are the differences.

newNode replaces whichever endpoint is closest as the new endpoint. The replaced endpoint then gains a second control point. The coordinate of the second control point is equal distant from the node as the first control point.

The newNode is either put in the front (index 0) or added to the back of the vector and is treated as an endpoint.

## Move Nodes and Control Point

When the user clicks and drags a point, they can move it. The callback functions uses the cursor's position to determine whether a node or control point was clicked.

The cursor callback function is used to move the selected point

### Nodes

When a node is clicked, the variable `selectedNode` is set to that node. The distance between the selectedNode and the cursor's position. The x and y coordinates of the selectedNode is set to be the cursor's x and y. The distance is used to move the related control points by adding the distance to its x and y coordinates.

### Control Points

When a control point is clicked, the variable `selectedHandle` is set to that point. The point's x and y are set to the cursor's position. A for loop is used to loop through all the nodes to find the node that has the selected control point. If that node has a second control point, the second control point is moved to be equal distant to the selectedHandle and kept in line with it.

## Draw

To draw the points and the bezier curve, the different drawing functions loops through `nodes`.

### Draw Nodes

The `draw_Point` function draws all the nodes as blue square dots by looping through the vector nodes.

### Draw Control Points

The `draw_Control_Point` function draws all the nodes as black circular dots by using the `GL_POINT_SMOOTH` to indicate it as smooth. The GL_POINT_SMOOTH is disabled after drawing to let the nodes be square when redrawn.

### Draw Connection Line

The `draw_Connections` function draws a dotted line between each node and their respective control points by using `GL_LINE_STIPPLE`, which is also disabled after drawing.

### Draw Spline

The `draw_Spline` function draws the spline with a smooth line by using `GL_LINE_SMOOTH`, which is disabled after drawing, and `GL_LINE_STRIP`. The loop looks at the current node and the next node to get the points needed to calculate the bezier curve. A second loop is used to add the line segment as the interval is calculated using `t += 1/N` where `t` is the current interval and `N` is the number of line segment (200).

`calc_bezier` is used to calculate the bezier curve point by taking the 2 nodes and their first control point. The function then uses the bezier formula for 3rd degree.

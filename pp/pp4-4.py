from OpenGL.GL import *

import glfw
import numpy as np

star = np.array([0.0, 10.0, 2.5, 2.5,
        10.0, 2.5, 4.0, -2.5,
        7.0, -10.0, 0.0, -5.0,
        -7.0, -10.0, -4.0, -2.5,
        -10.0, 2.5, -2.5, 2.5, 0.0, 10.0
        ], float)

decagon = np.array([5.0, 15.388, 13.0902, 9.51056,
           16.1803, 0.0, 13.0902, -9.51056,
           5.0, -15.388, -5.0, -15.388, -13.0902,
           -9.51056, -16.1803, 0.0, -13.0902, 9.51056,
           -5.0, 15.388, 5.0, 15.388
           ], float)

tween = lambda X, Y, t: X * (1-t) + Y * t

def main():

    frame = 0
    num_frame = 200
    direction = 1

    glfw.init()

    window = glfw.create_window(1280, 1000, "pp4: Tween", None, None)

    glfw.make_context_current(window)

    glMatrixMode(GL_PROJECTION)
    
    glLoadIdentity()

    glOrtho(-20, 20, -20, 20, -1.0, 1.0)
    
    glClearColor(1, 1, 1, 1)

    while not glfw.window_should_close(window):
        glfw.poll_events()

        glClear(GL_COLOR_BUFFER_BIT)

        glColor3f(0.0, 0.0, 0.0)

        glBegin(GL_LINE_LOOP)

        t = frame / num_frame

        for vert in tween(star, decagon, t).reshape(-1 ,2):
            glVertex2f(vert[0], vert[1])    

        glEnd()

        glFlush()

        glfw.swap_buffers(window)
        
        frame += 1 * direction

        match frame:

            case 0:
                direction = 1

            case num_frame:
                direction = -1

    glfw.terminate()

if __name__ == "__main__":
    main()
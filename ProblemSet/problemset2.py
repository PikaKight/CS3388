from OpenGL.GL import *
import glfw


glfw.init()

window = glfw.create_window(1280, 1000, "Hello World", None, None)

glfw.make_context_current(window)

while not glfw.window_should_close(window):
    glfw.poll_events()
    glClear(GL_COLOR_BUFFER_BIT)

    glColor3f(0.0, 50.0, 100.0)

    glBegin(GL_TRIANGLE_FAN)

    glVertex2f(0., 0.5)
    glVertex2f(0.2, 0.5)
    glVertex2f(0.4, 0.5)
    glVertex2f(0.6, 0.9)
    glVertex2f(0.7, 0.2)
    glVertex2f(0, -0.2)
    glVertex2f(-0.2, 0.0)

    glEnd()

    glBegin(GL_TRIANGLES)

    for i in range(1, 10):
    
        glVertex2f(0.09*i, -0.7)
        glVertex2f(-0.09*i, -0.3)
    
    glEnd()

    glFlush()

    glfw.swap_buffers(window)

glfw.terminate()

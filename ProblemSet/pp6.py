import glfw
import numpy as np

from OpenGL.GL import *
from glm import mat4, perspective, lookAt, radians, value_ptr, vec3

WIDTH = 1920
HEIGHT = 1080

glfw.init()

window = glfw.create_window(WIDTH, HEIGHT, "pp6", None, None)

glfw.make_context_current(window)

vertex_shader_code = """
    #version 330 core
    layout(location = 0) in vec3 vertexPosition; // Position attribute
    layout(location = 1) in vec3 vertexColor; // Color attribute
    out vec3 color; // Color output to fragment shader
    uniform mat4 MVP; // Model-View-Projection matrix
    void main() {
        gl_Position = MVP * vec4(vertexPosition, 1.0); // Transform vertex position
        color = vertexColor; // Pass color to fragment shader
    }
    """

# Define the fragment shader source code
fragment_shader_code = """
    #version 330 core
    in vec3 color; // Color input from vertex shader
    out vec4 fragColor; // Output color of the fragment
    void main() {
        fragColor = vec4(color, 1.0); // Set the fragment color
    }
    """

vertex_shader = glCreateShader(GL_VERTEX_SHADER)
glShaderSource(vertex_shader, vertex_shader_code)
glCompileShader(vertex_shader)

fragment_shader = glCreateShader(GL_FRAGMENT_SHADER)
glShaderSource(fragment_shader, fragment_shader_code)
glCompileShader(fragment_shader)

shader_program = glCreateProgram()
glAttachShader(shader_program, vertex_shader)
glAttachShader(shader_program, fragment_shader)
glLinkProgram(shader_program)

vertex_data = np.array([
    0.0, 0.0, 0.0, 1.0, 0.0, 0.0,
    1.0, -0.5, 0.0, 0.0, 1.0, 0.0,
    -1.0, -0.5, 0.0, 0.0, 0.0, 1.0
], dtype=np.float32)

vbo = glGenBuffers(1)
glBindBuffer(GL_ARRAY_BUFFER, vbo)
glBufferData(GL_ARRAY_BUFFER, vertex_data.nbytes, vertex_data, GL_STATIC_DRAW)

vertex_indices = np.array([0, 1, 2], dtype=np.uint32)

ebo = glGenBuffers(1)
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo)
glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertex_indices.nbytes, vertex_indices, GL_STATIC_DRAW)


glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*4, None)
glEnableVertexAttribArray(0)

glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*4, GLvoidp(3*4))
glEnableVertexAttribArray(1)

glEnable(GL_DEPTH_TEST)
glDepthFunc(GL_LESS)

glEnable(GL_BLEND)
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)


projection = perspective(radians(45), WIDTH/HEIGHT, 0.001, 1000)

eye = vec3(5, 2, 5)
up = vec3(0, 1, 0)
center = vec3(0, 0, 0)

V = lookAt(eye, center, up) # View matrix

M = mat4(1)

    
MVP = projection * V * M


while not glfw.window_should_close(window):
    glfw.poll_events()

    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT)

    glClearColor(1.0, 1.0, 1.0, 1.0)

    glUseProgram(shader_program)
    
    mvp_loc = glGetUniformLocation(shader_program, "MVP")
    glUniformMatrix4fv(mvp_loc, 1, GL_FALSE, value_ptr(MVP))

    glBindBuffer(GL_ARRAY_BUFFER, vbo)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo)
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, None)

    glfw.swap_buffers(window)
    
glDeleteBuffers(1, [vbo])
glDeleteProgram(shader_program)

glfw.terminate()
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#define TEXHEIGHT   256
#define TEXWIDTH    256
GLubyte texture[TEXHEIGHT][TEXWIDTH][3];

// typedef const char GLbyte;

// Handle to a program object
GLuint programObject;
GLuint program;
GLuint g_vbo;
GLuint g_ibo;
// Attribute locations
GLint  positionLoc;
GLint  texCoordLoc;
// Sampler location
GLint samplerLoc;
// Texture handle
GLuint textureId;


int LoadFile(char *filename);
GLuint CreateSimpleTexture2D();
void destroyEGL(EGLDisplay &display, EGLContext &context, EGLSurface &surface);
int initializeEGL(Display *xdisp, Window &xwindow, EGLDisplay &display, EGLContext &context, EGLSurface &surface);
void init();
void createBuffer();
void Draw();
void mainloop(Display *xdisplay, EGLDisplay display, EGLSurface surface);
void GetbufferData();
GLuint loadShader(GLenum shaderType, const char *source);
GLuint createProgram(const char *vshader, const char *fshader);
void deleteShaderProgram(GLuint shaderProgram);


//g++ EGL_test.cpp -o EGL_test -lX11 -lEGL -lGL

int main()
{
    Display *xdisplay = XOpenDisplay(nullptr);
    if (xdisplay == nullptr)
    {
        std::cerr << "Error XOpenDisplay." << std::endl;
        exit(EXIT_FAILURE);
    }

    Window xwindow = XCreateSimpleWindow(xdisplay, DefaultRootWindow(xdisplay), 100, 100, 960, 540,
                                         1, BlackPixel(xdisplay, 0), WhitePixel(xdisplay, 0));

    XMapWindow(xdisplay, xwindow);

    EGLDisplay display = nullptr;
    EGLContext context = nullptr;
    EGLSurface surface = nullptr;
    if (initializeEGL(xdisplay, xwindow, display, context, surface) < 0)
    {
        std::cerr << "Error initializeEGL." << std::endl;
        exit(EXIT_FAILURE);
    }

    mainloop(xdisplay, display, surface);

    destroyEGL(display, context, surface);
    XDestroyWindow(xdisplay, xwindow);
    XCloseDisplay(xdisplay);

    return 0;
}


int initializeEGL(Display *xdisp, Window &xwindow, EGLDisplay &display, EGLContext &context, EGLSurface &surface)
{
    display = eglGetDisplay(static_cast<EGLNativeDisplayType>(xdisp));
    if (display == EGL_NO_DISPLAY)
    {
        std::cerr << "Error eglGetDisplay." << std::endl;
        return -1;
    }
    if (!eglInitialize(display, nullptr, nullptr))
    {
        std::cerr << "Error eglInitialize." << std::endl;
        return -1;
    }

    EGLint attr[] = {EGL_BUFFER_SIZE, 16, EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_NONE};
    EGLConfig config = nullptr;
    EGLint numConfigs = 0;
    if (!eglChooseConfig(display, attr, &config, 1, &numConfigs))
    {
        std::cerr << "Error eglChooseConfig." << std::endl;
        return -1;
    }

    if (numConfigs != 1)
    {
        std::cerr << "Error numConfigs." << std::endl;
        return -1;
    }

    surface = eglCreateWindowSurface(display, config, xwindow, nullptr);
    if (surface == EGL_NO_SURFACE)
    {
        std::cerr << "Error eglCreateWindowSurface. " << eglGetError() << std::endl;
        return -1;
    }

    EGLint ctxattr[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
    context = eglCreateContext(display, config, EGL_NO_CONTEXT, ctxattr);
    if (context == EGL_NO_CONTEXT)
    {
        std::cerr << "Error eglCreateContext. " << eglGetError() << std::endl;
        return -1;
    }
    eglMakeCurrent(display, surface, surface, context);

    return 0;
}

void mainloop(Display *xdisplay, EGLDisplay display, EGLSurface surface)
{
    const char *vshader = R"(
        attribute vec4 vPosition;
        void main() {
            gl_Position = vPosition;
        }
    )";

    const char *fshader = R"(
        precision mediump float;
        void main() {
            gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
        }
    )";

   GLfloat vVertices[] = { -1.0f,  1.0f, 0.0f,  // Position 0
                            0.0f,  1.0f,        // TexCoord 0 
                           -1.0f, -1.0f, 0.0f,  // Position 1
                            0.0f,  0.0f,        // TexCoord 1
                            1.0f, -1.0f, 0.0f,  // Position 2
                            1.0f,  0.0f,        // TexCoord 2
                            1.0f,  1.0f, 0.0f,  // Position 3
                            1.0f,  1.0f         // TexCoord 3
                         };

   GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

    GLuint program = createProgram(vshader, fshader);
    glUseProgram(program);
    const GLfloat vertices[] = {0.0f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f};
    GLint gvPositionHandle = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(gvPositionHandle);


    while (true)
    {
        XPending(xdisplay);

        glClearColor(0.25f, 0.25f, 0.5f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // GetbufferData();

        // glVertexAttribPointer(gvPositionHandle, 2, GL_FLOAT, GL_FALSE, 0, vertices);

        // Load the vertex position
        glVertexAttribPointer ( positionLoc, 3, GL_FLOAT, 
                                GL_FALSE, 5 * sizeof(GLfloat), vVertices );
        // Load the texture coordinate
        // glVertexAttribPointer ( texCoordLoc, 2, GL_FLOAT,
                                // GL_FALSE, 5 * sizeof(GLfloat), &vVertices[3] );

        glEnableVertexAttribArray ( positionLoc );
        // glEnableVertexAttribArray ( texCoordLoc );

        glDrawElements ( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices );
        // glDrawArrays(GL_TRIANGLES, 0, 3);
        // glDrawArrays(GL_TRIANGLES, 1, 3);

        // GetbufferData();

        // glDrawElements ( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices );

        eglSwapBuffers(display, surface);
        usleep(1000);

        GetbufferData();
    }
    deleteShaderProgram(program);
}

void GetbufferData()
{
    GLint readType, readFormat;

    glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_TYPE, &readType);
    glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_FORMAT, &readFormat);

    printf("GL_IMPLEMENTATION_COLOR_READ_TYPE = %d\n", readType);
    printf("GL_IMPLEMENTATION_COLOR_READ_FORMAT = %d\n", readFormat);

    GLubyte* databuffer = (GLubyte*)malloc(960*540*4);
    // databuffer = {0};
    memset(databuffer, 0, sizeof(databuffer));

    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glReadBuffer( GL_BACK );
    glReadPixels(
        0,                 //読み取る領域の左下隅のx座標
        0,                 //読み取る領域の左下隅のy座標 //0 or getCurrentWidth() - 1
        // 1,            // imageWidth,             //読み取る領域の幅 
        // 1,            // imageHeight,            //読み取る領域の高さ
        960,            // imageWidth,             //読み取る領域の幅 
        540,            // imageHeight,            //読み取る領域の高さ
        GL_RGBA, //it means GL_BGR,           //取得したい色情報の形式
        GL_UNSIGNED_BYTE,  //読み取ったデータを保存する配列の型
        databuffer      //ビットマップのピクセルデータ（実際にはバイト配列）へのポインタ
        );
        int err;
        err = glGetError();
        printf("glGetError 2 = %d\n", err);


        printf("dataBuffer 0= %d\n", databuffer[0]);
        printf("dataBuffer 1= %d\n", databuffer[1]);
        printf("dataBuffer 2= %d\n", databuffer[2]);
        printf("dataBuffer 3= %d\n", databuffer[3]);
        printf("dataBuffer 4= %d\n", databuffer[4]);
        printf("dataBuffer 5= %d\n", databuffer[5]);
        printf("dataBuffer 6= %d\n", databuffer[6]);

        free(databuffer);
}



GLuint createProgram(const char *vshader, const char *fshader)
{
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, vshader);
    GLuint fragShader = loadShader(GL_FRAGMENT_SHADER, fshader);
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);
    GLint linkStatus = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if (linkStatus == GL_FALSE)
    {
        std::cerr << "Error glLinkProgram." << std::endl;
        exit(EXIT_FAILURE);
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragShader);
    return program;
}

GLuint loadShader(GLenum shaderType, const char *source)
{
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    GLint compiled = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled == GL_FALSE)
    {
        std::cerr << "Error glCompileShader." << std::endl;
        exit(EXIT_FAILURE);
    }
    return shader;
}

void destroyEGL(EGLDisplay &display, EGLContext &context, EGLSurface &surface)
{
    eglDestroyContext(display, context);
    eglDestroySurface(display, surface);
    eglTerminate(display);
}

void deleteShaderProgram(GLuint shaderProgram)
{
    glDeleteProgram(shaderProgram);
}

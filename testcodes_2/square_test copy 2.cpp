#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES/gl.h>
#include <EGL/egl.h>
#include <iostream>
#include <unistd.h>
#include <math.h>
#include <algorithm>
#define degree2radian(degree) ((degree * M_PI) / 180.0F)


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

    //画像読み込み
    char path[] = "./num256.bmp";
    int size = LoadFile(path);
    printf("LoadFile %d \n", size);

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

int LoadFile(char *filename)
{
  FILE *fp;
  long fsize;
  int n_read = 0;

  /* テクスチャ画像の読み込み */
  if ((fp = fopen(filename, "rb")) != NULL) {
    //ファイルヘッダ分、シークする。
    fseek(fp, 54L, SEEK_SET);

    //動的にファイルサイズを取得できるようにする。
    // n_read = fread(texture, 1, sizeof texture, fp);
    // n_read = fread(texture, 1, 262198, fp);
    // n_read = fread(texture, 1, 66614, fp);
    n_read = fread(texture, 1, 196662, fp);
    
    fclose(fp);
  }

  return n_read;
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
        attribute vec3 a_texCoord;
        varying vec3 v_texCoord;
        uniform mediump mat4 mRotation;
        void main() {
            gl_Position = mRotation * vPosition;
            v_texCoord  = a_texCoord;
        }
    )";

    const char *fshader = R"(
        precision mediump float;
        varying vec3 v_texCoord;
        uniform sampler2D s_texture;
        void main() {
            gl_FragColor = texture2DProj( s_texture, v_texCoord );
        }
    )";

    // const char *fshader = R"(
    //     precision mediump float;
    //     varying vec2 v_texCoord;
    //     uniform sampler2D s_texture;
    //     void main() {
    //         gl_FragColor = texture2D( s_texture, v_texCoord );
    //     }
    // )";

    // const char *fshader = R"(
    //     precision mediump float;
    //     void main() {
    //         gl_FragColor = vec4(0.3, 0.8, 0.3, 1.0);
    //     }
    // )";

   GLfloat vVertices[] = { -0.5f,  0.5f, -10.0f,  // Position 0
                            0.0f,  1.0f, -10.0f,        // TexCoord 0 
                           -0.5f, -0.5f, 0.0f,  // Position 1
                            0.0f,  0.0f, 0.0f,       // TexCoord 1
                            0.5f, -0.5f, 0.0f,  // Position 2
                            1.0f,  0.0f, 0.0f,       // TexCoord 2
                            0.5f,  0.5f, -10.0f,  // Position 3
                            1.0f,  1.0f, -10.0f,        // TexCoord 3
                         };

   GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

    glViewport(0, 0, 960, 540);

    GLuint program = createProgram(vshader, fshader);
    glUseProgram(program);
    GLint gvPositionHandle = glGetAttribLocation(program, "vPosition");
    GLint gvTexCoordHandle = glGetAttribLocation(program, "a_texCoord" );

    GLint gmSamplerHandle = glGetUniformLocation(program, "s_texture");
    GLint gmRotationHandle = glGetUniformLocation(program, "mRotation");

    // Use tightly packed data
    glPixelStorei ( GL_UNPACK_ALIGNMENT, 1 );

    // Generate a texture object
    glGenTextures ( 1, &textureId );

    // Bind the texture object
    glBindTexture ( GL_TEXTURE_2D, textureId );

    // Set the filtering mode
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGB, TEXWIDTH, TEXHEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, texture);

    int degree = 0;
    // int degree = (degree + 100) % 360;
    while (true)
    {
        XPending(xdisplay);

        // const GLfloat matrix[] = {
        // static_cast<GLfloat>(cos(degree2radian(degree))), 0.0f, static_cast<GLfloat>(sin(degree2radian(degree))), 0.0f,
        // 0.0f, 1.0f, 0.0f, 0.0f,
        // static_cast<GLfloat>(-sin(degree2radian(degree))), 0.0f, static_cast<GLfloat>(cos(degree2radian(degree))), 0.0f,
        // 0.0f, 0.0f, 0.0f, 1.0f};

        const GLfloat matrix[] = {
        static_cast<GLfloat>(cos(degree2radian(degree))), 
        0.0f, 
        static_cast<GLfloat>(sin(degree2radian(degree))), 
        0.0f,
        0.0f, 
        1.0f, 
        0.0f, 
        0.0f,
        static_cast<GLfloat>(-sin(degree2radian(degree))), 
        0.0f, 
        static_cast<GLfloat>(cos(degree2radian(degree))), 
        0.0f,
        0.0f, 
        0.0f, 
        0.0f, 
        1.0f
        };

        glClearColor(0.25f, 0.25f, 0.5f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



        // glVertexAttribPointer(gvPositionHandle, 2, GL_FLOAT, GL_FALSE, 0, vertices);

        // Load the vertex position
        glVertexAttribPointer ( gvPositionHandle, 3, GL_FLOAT, 
                                GL_FALSE, 6 * sizeof(GLfloat), vVertices );
        // Load the texture coordinate
        glVertexAttribPointer ( gvTexCoordHandle, 3, GL_FLOAT,
                                GL_FALSE, 6 * sizeof(GLfloat), &vVertices[3] );

        glEnableVertexAttribArray ( gvPositionHandle );
        glEnableVertexAttribArray ( gvTexCoordHandle );

        // Bind the texture
        glActiveTexture ( GL_TEXTURE0 );
        glBindTexture ( GL_TEXTURE_2D, textureId );

		glUniformMatrix4fv(gmRotationHandle, 1, GL_FALSE, matrix);


        glDrawElements ( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices );
        // glDrawArrays(GL_TRIANGLES, 0, 3);

        eglSwapBuffers(display, surface);
        // degree = (degree + 1) % 360;
        usleep(1000);
    }
    deleteShaderProgram(program);
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


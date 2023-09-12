#include "square_test.h"

//g++ EGL_test.cpp -o EGL_test -lX11 -lEGL -lGL


int main()
{
    Display *xdisplay = XOpenDisplay(nullptr);
    Window xwindow = XCreateSimpleWindow(xdisplay, DefaultRootWindow(xdisplay), 100, 100, 960, 540,
                                         1, BlackPixel(xdisplay, 0), WhitePixel(xdisplay, 0));

    XMapWindow(xdisplay, xwindow);

    int size = LoadFile("./num256.bmp");
    // int size = LoadFile("./paraboloid1.bmp");

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
  FILE *fp, *wp;
  long fsize;
  int n_read = 0;

  /* テクスチャ画像の読み込み */
  if ((fp = fopen(filename, "rb")) != NULL) {
    //ファイルヘッダ分、シークする。
    fseek(fp, 54L, SEEK_SET);
    n_read = fread(texture0, 1, 196662, fp);

    fclose(fp);

    //texture RGB作成
    for (int i = 0; i < TEXHEIGHT; i++) {
            for (int j = 0; j < TEXWIDTH; j++) {
                if(i<86){
                    texture0[i][j][0] = 255;
                    texture0[i][j][1] = 0;
                    texture0[i][j][2] = 0;
                }
                else if(i<172){
                    texture0[i][j][0] = 0;
                    texture0[i][j][1] = 255;
                    texture0[i][j][2] = 0;
                }
                else{
                    texture0[i][j][0] = 0;
                    texture0[i][j][1] = 0;
                    texture0[i][j][2] = 255;
                }
            }
    }

    //RGB2YUV ちゃんとできてる
    for (int i = 0; i < TEXHEIGHT; i++) {
            for (int j = 0; j < TEXWIDTH; j++) {
                // Y  =  0.257R + 0.504G + 0.098B + 16
                // Cb = -0.148R - 0.291G + 0.439B + 128
                // Cr =  0.439R - 0.368G - 0.071B + 128
                yuv[i][j][0] = texture0[i][j][0] * 0.257 + texture0[i][j][1] * 0.504 + texture0[i][j][2] * 0.098 + 16;
                yuv[i][j][1] = texture0[i][j][0] * -0.148 + texture0[i][j][1] * -0.291 + texture0[i][j][2] * 0.439 + 128;
                yuv[i][j][2] = texture0[i][j][0] * 0.439 + texture0[i][j][1] * -0.368 + texture0[i][j][2] * -0.071 + 128;
            }
    }


    //YUV2RGB
    for (int i = 0; i < TEXHEIGHT; i++) {
        for (int j = 0; j < TEXWIDTH; j++) {
            // if(yuv[i][j][0] - 16 < 0){
            //     std::cout << "yuv[i][j][0] - 16 < 0" << std::endl;
            //     yuv[i][j][0] = yuv[i][j][0] + 255;
            //     }
            // if(yuv[i][j][1] - 128 < 0){
            //     std::cout << "yuv[i][j][1] - 128 < 0" << std::endl;
            //     yuv[i][j][1] = yuv[i][j][1] + 255;
            //     }
            // if(yuv[i][j][2] - 128 < 0){
            //     std::cout << "yuv[i][j][2] - 128 < 0" << std::endl;
            //     yuv[i][j][2] = yuv[i][j][2] + 255;
            //     }

            int y,u,v;
            // unsigned char y,u,v;
            y = yuv[i][j][0] - 16;
            u = yuv[i][j][1] - 128;
            v = yuv[i][j][2] - 128;

            // texture[i][j][0] = (yuv[i][j][0] - 16) * 1.164 +                                (yuv[i][j][2] - 128) * 1.596;
            // texture[i][j][1] = (yuv[i][j][0] - 16) * 1.164 + (yuv[i][j][1] - 128) * -0.391 + (yuv[i][j][2] - 128) * -0.813;
            // texture[i][j][2] = (yuv[i][j][0] - 16) * 1.164 + (yuv[i][j][1] - 128) * 2.018;

            int R,G,B;
            // unsigned char R,G,B;
            // texture[i][j][0] = y * 1.164 +                                v * 1.596;
            // texture[i][j][1] = y * 1.164 + u * -0.391 + v * -0.813;
            // texture[i][j][2] = y * 1.164 + u * 2.018;

            R = y * 1.164 +                                v * 1.596;
            G = y * 1.164 + u * -0.391 + v * -0.813;
            B = y * 1.164 + u * 2.018;

            if(R < 0){R = 0;} else if(R > 255){R = 255;}
            if(G < 0){G = 0;} else if(G > 255){G = 255;}
		    if(B < 0){B = 0;} else if(B > 255){B = 255;}


            texture[i][j][0] = R;
            texture[i][j][1] = G;
            texture[i][j][2] = B;

            // if(texture[i][j][0] < 0){texture[i][j][0] = 0;} else if(texture[i][j][0] > 255){texture[i][j][0] = 255;}
            // if(texture[i][j][1] < 0){texture[i][j][1] = 0;} else if(texture[i][j][1] > 255){texture[i][j][1] = 255;}
		    // if(texture[i][j][2] < 0){texture[i][j][2] = 0;} else if(texture[i][j][2] > 255){texture[i][j][2] = 255;}

            if(((i==0)&&(j==0))||((i==87)&&(j==0))){
                std::cout << "texture[i][j][0] = " << (int)texture[i][j][0] << std::endl;   
                std::cout << "R Y = " << (int)yuv[i][j][0] << " Y' =  "<< (int)(yuv[i][j][0] - 16) * 1.164 << std::endl;
                std::cout << "R V = " << (int)yuv[i][j][2] << " V' =  "<< (int)(yuv[i][j][2] - 128) * 1.596 << std::endl;

                std::cout << "texture[i][j][1] = " << (int)texture[i][j][1] << std::endl;
                std::cout << "G Y = " << (int)yuv[i][j][0] << " Y' =  "<< (int)(yuv[i][j][0] - 16) * 1.164 << std::endl;
                std::cout << "G U = " << (int)yuv[i][j][1] << " U' =  "<< (int)(yuv[i][j][1] - 128) * -0.391 << std::endl;
                std::cout << "G V = " << (int)yuv[i][j][2] << " V' =  "<< (int)(yuv[i][j][2] - 128) * -0.813 << std::endl;

    		    if(texture[i][j][2] < 0){
                std::cout << "texture[i][j][2] < 0" << std::endl;
                    texture[i][j][2] = 0;
                }else if(texture[i][j][2] > 255){
                std::cout << " texture[i][j][2] > 255 " << std::endl;
                    texture[i][j][2] = 255;
                }

                std::cout << "texture[0][0][2] = " << (int)texture[0][0][2] << std::endl;
                std::cout << "B Y = " << (int)yuv[i][j][0] << " Y' =  "<< (int)(yuv[i][j][0] - 16) * 1.164 << std::endl;
                std::cout << "B U = " << (int)yuv[i][j][1] << " U' =  "<< (int)(yuv[i][j][1] - 128) * 2.018 << std::endl;
                // std::cout << "yuv[0][0][1] = " << (int)yuv[0][0][1] << std::endl;
                // std::cout << "yuv[0][0][2] = " << (int)yuv[0][0][2] << std::endl;
            }

            }
    }

    // std::cout << "yuv[0][0][0] = " << (int)yuv[0][0][0] << std::endl;
    // std::cout << "yuv[0][0][1] = " << (int)yuv[0][0][1] << std::endl;
    // std::cout << "yuv[0][0][2] = " << (int)yuv[0][0][2] << std::endl;

    // std::cout << "yuv[87][0][0] = " << (int)yuv[87][0][0] << std::endl;
    // std::cout << "yuv[87][0][1] = " << (int)yuv[87][0][1] << std::endl;
    // std::cout << "yuv[87][0][2] = " << (int)yuv[87][0][2] << std::endl;

    // std::cout << "yuv[173][0][0] = " << (int)yuv[173][0][0] << std::endl;
    // std::cout << "yuv[173][0][1] = " << (int)yuv[173][0][1] << std::endl;
    // std::cout << "yuv[173][0][2] = " << (int)yuv[173][0][2] << std::endl;

    // std::cout << "texture[100][100][0] = " << (int)texture[100][100][0] << std::endl;
    // std::cout << "texture[100][100][1] = " << (int)texture[100][100][1] << std::endl;
    // std::cout << "texture[100][100][2] = " << (int)texture[100][100][2] << std::endl;

    // std::cout << "texture[0][0][0] = " << (int)texture[0][0][0] << std::endl;
    // std::cout << "texture[0][0][1] = " << (int)texture[0][0][1] << std::endl;
    // std::cout << "texture[0][0][2] = " << (int)texture[0][0][2] << std::endl;

    // std::cout << "texture[87][0][0] = " << (int)texture[87][0][0] << std::endl;
    // std::cout << "texture[87][0][1] = " << (int)texture[87][0][1] << std::endl;
    // std::cout << "texture[87][0][2] = " << (int)texture[87][0][2] << std::endl;

    // std::cout << "texture[173][0][0] = " << (int)texture[173][0][0] << std::endl;
    // std::cout << "texture[173][0][1] = " << (int)texture[173][0][1] << std::endl;
    // std::cout << "texture[173][0][2] = " << (int)texture[173][0][2] << std::endl;

  }


//     for (int i = 0; i < TEXHEIGHT; i++) {
//             for (int j = 0; j < TEXWIDTH; j++) {
//                 texture[i][j][0] = (yuv[i][j][0] - 16) * 1.164 +                                (yuv[i][j][2] - 128) * 1.596;
//                 texture[i][j][1] = (yuv[i][j][0] - 16) * 1.164 + (yuv[i][j][1] - 128) * -0.391 + (yuv[i][j][2] - 128) * -0.813;
//                 texture[i][j][2] = (yuv[i][j][0] - 16) * 1.164 + (yuv[i][j][1] - 128) * 2.018;

//                 // texture[i][j][0] = yuv[i][j][0] * 1.164 +                         yuv[i][j][2] * 1.596;
//                 // texture[i][j][1] = yuv[i][j][0] * 1.164 + yuv[i][j][1] * -0.391 + yuv[i][j][2] * -0.813;
//                 // texture[i][j][2] = yuv[i][j][0] * 1.164 + yuv[i][j][1] * 2.018;

//                 if(texture[i][j][0] < 0){texture[i][j][0] = 0;} else if(texture[i][j][0] > 255){texture[i][j][0] = 255;}
//                 if(texture[i][j][1] < 0){texture[i][j][1] = 0;} else if(texture[i][j][1] > 255){texture[i][j][1] = 255;}
// 		        if(texture[i][j][2] < 0){texture[i][j][2] = 0;} else if(texture[i][j][2] > 255){texture[i][j][2] = 255;}
//             }
//     }

//     // std::cout << "texture create" << std::endl;

}


int initializeEGL(Display *xdisp, Window &xwindow, EGLDisplay &display, EGLContext &context, EGLSurface &surface)
{
    display = eglGetDisplay(static_cast<EGLNativeDisplayType>(xdisp));
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

    surface = eglCreateWindowSurface(display, config, xwindow, nullptr);

    EGLint ctxattr[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
    context = eglCreateContext(display, config, EGL_NO_CONTEXT, ctxattr);
    eglMakeCurrent(display, surface, surface, context);

    return 0;
}

void mainloop(Display *xdisplay, EGLDisplay display, EGLSurface surface)
{
	// const char* shader =
	// 	"#version 300 es\n"
	// 	// "layout(location = 0) in vec3 position;\n"
	// 	// "layout(location = 1) in vec2 vuv;\n"
    //     "out vec2 Flag_uv;\n"
	// 	"void main(void) {\n"
    //         "Flag_uv  = vec2(0,0);\n"
	// 		"gl_Position = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n"
	// 	"}\n";

	const char* vshader =
		"#version 300 es\n"
		"layout(location = 0) in vec3 position;\n"
		"layout(location = 1) in vec2 vuv;\n"
        "out vec2 Flag_uv;\n"
		"void main(void) {\n"
            "Flag_uv  = vuv;\n"
			"gl_Position = vec4(position, 1.0f);\n"
		"}\n";

	const char* fshader =
		"#version 300 es\n"
        "precision mediump float;"
        "in vec2 Flag_uv;\n"
		"out vec4 outFragmentColor;\n"
        "uniform sampler2D Texture;\n"
		"void main() {\n"
            "outFragmentColor = texture2D( Texture, Flag_uv );\n"
            // "outFragmentColor = vec4(0.3, 0.8, 0.3, 1.0);\n"
		"}\n";
	// const char* fshader =
	// 	"#version 300 es\n"
    //     "precision mediump float;"
    //     "in vec2 Flag_uv;\n"
	// 	"out vec4 outFragmentColor;\n"
    //     "uniform sampler2D Texture;\n"
	// 	"void main() {\n"
    //         // "outFragmentColor = texture2D( Texture, Flag_uv );\n"
    //         "vec3 yuv = texture( Texture, Flag_uv ).rgb;\n"
    //         "outFragmentColor.x = (float(298)*(yuv.x-float(16))+float(409)*(yuv.z-float(128)+float(128))/float(256);\n"
    //         "outFragmentColor.y = (float(298)*(yuv.x-float(16))-float(100)*(yuv.y-float(128)-float(208)*(yuv.z-float(128))+float(128))/float(256);\n"
    //         "outFragmentColor.z = (float(298)*(yuv.x-float(16))+float(516)*(yuv.y-float(128)+float(128))/float(256);\n"
    //         "if(outFragmentColor.x < 0){outFragmentColor.x = 0;} else if(outFragmentColor.x > 255){outFragmentColor.x = 255;}\n"
    //         "if(outFragmentColor.y < 0){outFragmentColor.y = 0;} else if(outFragmentColor.y > 255){outFragmentColor.y = 255;}\n"
	// 	    "if(outFragmentColor.z < 0){outFragmentColor.z = 0;} else if(outFragmentColor.z > 255){outFragmentColor.z = 255;}\n"
    //     "}\n";


	GLfloat points[] = {
                    -0.5f, 0.5f, 0.0f, 
				    -0.5f, -0.5f, 0.0f, 
				    0.5f, -0.5f,  0.0f,
				    0.5f, 0.5f, 0.0f

                // 0.3f, 0.8f, 0.0f,//四角形2つ目
    			// 0.5f, -0.3f, 0.0f,
	    		// -0.7f, 0.5f, 0.0f,
		    	// -0.2f, -0.2f, 0.0f
                };


	GLfloat colors[] = { 0.5f, 0.0f, 0.3f,
				 0.5f, 0.8f, 0.0f,
				 1.0f, 0.0f, 1.0f,
				 1.0f, 0.8f, 1.0f,

                0.5f, 0.0f, 1.0f,//四角形2つ目
                0.5f, 0.3f, 0.5f,
                1.0f, 0.0f, 1.0f,
                0.2f, 0.1f, 1.0f };

    GLfloat vertex_uv[] = { 
                0.0f, 1.0f,
                0.0f, 0.0f,
                1.0f, 0.0f,
                1.0f, 1.0f,
                };

    // GLfloat vertex_uv[] = { 1.0f, 0.0f,
    //           1.0f, 1.0f,
    //           0.0f, 1.0f,
    //           0.0f, 0.0f,
    //           };                

    GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

    // GLuint Shader = loadShader(GL_VERTEX_SHADER, vshader);

    GLuint program = createProgram(vshader, fshader);

    // GLint gvPositionHandle_1 = glGetAttribLocation(program, "vPosition");
    // GLint gvPositionHandle_2 = glGetAttribLocation(program, "aColor");

    // std::cout << "gvPositionHandle_1 = " << gvPositionHandle_1 << std::endl;
    // std::cout << "gvPositionHandle_2 = " << gvPositionHandle_2 << std::endl;

    GLuint vao, vertex_vbo, texture_vbo;

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// 頂点座標のVBOを作成	
	glGenBuffers(1, &vertex_vbo); //バッファを作成
	glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo); //以下よりvertex_vboでバインドされているバッファが処理される
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW); //実データを格納

	glGenBuffers(1, &texture_vbo); //バッファを作成
	glBindBuffer(GL_ARRAY_BUFFER, texture_vbo); //以下よりvertex_vboでバインドされているバッファが処理される
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_uv), vertex_uv, GL_STATIC_DRAW); //実データを格納

    
    // 追加：テクスチャ情報を送るuniform属性を設定する
    GLint textureLocation = glGetUniformLocation(program, "texture");


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


    // glBindBuffer(GL_ARRAY_BUFFER, 0);
    // glBindVertexArray(0);

    int degree = 0;
    while (true)
    {
        XPending(xdisplay);

        glClearColor(0.25f, 0.25f, 0.5f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        glUseProgram(program);

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, texture_vbo);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

        // glBindVertexArray(vao);

        // glDrawArrays(GL_TRIANGLES, 0, 3);

        // Bind the texture
        glActiveTexture ( GL_TEXTURE0 );
        glUniform1i(textureLocation, 0);
        glBindTexture ( GL_TEXTURE_2D, textureId );

        // glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        // glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);
        // glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glDrawElements ( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices );

        eglSwapBuffers(display, surface);
        // degree = (degree + 1) % 360;

        // glViewport(20,20,480,270);
        //x : 画面左下から数えたXピクセル座標
        //y : 画面左下から数えたYピクセル座標
        //width : Viewport の幅ピクセル数
        //height : Viewport の高さピクセル数

        usleep(1000);
    }
    deleteShaderProgram(program);
}



GLuint createProgram(const char *vshader, const char *fshader)
{
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, vshader);
    // std::cout << "vshader = " << vshader << std::endl;
    GLuint fragShader = loadShader(GL_FRAGMENT_SHADER, fshader);
    // std::cout << "fshader = " << fshader << std::endl;
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

    GLint max = 1000;
    GLchar *infolog;

    glGetShaderInfoLog(shader, max, &max, infolog);
    // std::cout << " infolog \n" << infolog << std::endl;

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

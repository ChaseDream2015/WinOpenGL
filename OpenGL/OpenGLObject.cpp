#include "stdafx.h"
#include "OpenGLObject.h"


OpenGLObject::OpenGLObject()
{
	m_rotate = 0.0f;
}


OpenGLObject::~OpenGLObject()
{
	glDeleteBuffers(1, &m_vertexBuffer);
	glDeleteVertexArrays(1, &m_vertexArray);
}

bool OpenGLObject::InitGlew(CWnd *window)
{
	//������ʱ���ڣ�ֻ��Ϊ�˳�ʼ��glew��
	PIXELFORMATDESCRIPTOR pfd;
	CWnd* tempWnd = new CWnd();
	tempWnd->Create(NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0, 0, 20, 20), window, NULL);
	HDC temHdc = tempWnd->GetDC()->GetSafeHdc();
	//�����������ظ�ʽ
	if (!SetPixelFormat(temHdc, 1, &pfd))//ÿ������ֻ������һ��
		return false;
	HGLRC temphRC = wglCreateContext(temHdc);//����һ����ʱ�Ļ���Ϊ�˳�ʼ��glew,��ʼ������ܹ�ʹ��wglChoosePixelFormatARB��wglCreateContextAttribsARB���� 
	wglMakeCurrent(temHdc, temphRC);//ֻ�����õ�ǰopengl��������ܹ���ʼ��glew�� 
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		MessageBox(NULL,L"GLEW��ʼ��ʧ�ܣ�", L"��ʾ", MB_ICONERROR);
		return false;
	}
	if (!WGLEW_ARB_create_context || !WGLEW_ARB_pixel_format)
	{
		return false;
	}
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(temphRC);
	tempWnd->DestroyWindow();//������ʱ����
	delete tempWnd;
	return true;
}

bool OpenGLObject::InitializeOpenGL(CDC* pDC)
{
	SetupPixelFormat(pDC);
	::wglMakeCurrent(pDC->GetSafeHdc(), m_hRc);

	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	return true;
}

bool OpenGLObject::SetupPixelFormat(CDC* pDC)
{
	PIXELFORMATDESCRIPTOR pfd;
	int nPixelFormat = -1;
	int nPixCount = 0;
	float fPixAttribs[] = { 0, 0 };
	int iPixAttribs[] = {
		WGL_SUPPORT_OPENGL_ARB, 1,//֧��OPENGL��Ⱦ
		WGL_DRAW_TO_WINDOW_ARB, 1,//���ظ�ʽ�������е�����
		WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,//��HW����
		WGL_COLOR_BITS_ARB, 24,//R,G,Bÿ����ɫ8bit
		WGL_DEPTH_BITS_ARB, 24,//24λ���
		WGL_DOUBLE_BUFFER_ARB, GL_TRUE,//˫����������
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,//pf should be RGBA type
		WGL_STENCIL_BITS_ARB, 8,//ģ�滺����8bit
		WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,//�������ز���
		WGL_SAMPLES_ARB, 4,//���ز�������Ϊ4
		0
	};
	//�µĲ�ѯ���ظ�ʽ�ĺ���
	wglChoosePixelFormatARB(pDC->GetSafeHdc(), iPixAttribs, fPixAttribs, 1, &nPixelFormat, (UINT*)&nPixCount);
	//���ز���ʱ�����Ӳ����֧�־�ʹ������Ĵ���رն��ز���
	if (nPixelFormat == -1)
	{
		//try again without MSAA
		iPixAttribs[18] = 1;
		wglChoosePixelFormatARB(pDC->GetSafeHdc(), iPixAttribs, fPixAttribs, 1, &nPixelFormat, (UINT*)&nPixCount);
	}
	//��ȡһ�����ظ�ʽ������������Ϊ��ǰ��������
	if (!SetPixelFormat(pDC->GetSafeHdc(), nPixelFormat, &pfd))
	{
		MessageBox(NULL,L"�������ظ�ʽʧ�ܣ�", L"��ʾ", MB_ICONERROR);
		return false;
	}
	GLint attribs[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, 3,//���汾3
		WGL_CONTEXT_MINOR_VERSION_ARB, 3,//�ΰ汾3
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,//Ҫ�󷵻ؼ���ģʽ����,�����ָ����ָ��ΪWGL_CONTEXT_CORE_PROFILE_BIT_ARB�᷵��ֻ�������Ĺ��ܵĻ��� 
		0
	};
	m_hRc = wglCreateContextAttribsARB(pDC->GetSafeHdc(), 0, attribs);
	if (m_hRc == NULL)
	{
		MessageBox(NULL,L"���ܴ���OGL3.3 context\n���Դ���3.2��", L"��ʾ", MB_OK);
		attribs[3] = 2;
		m_hRc = wglCreateContextAttribsARB(pDC->GetSafeHdc(), 0, attribs);
		if (m_hRc == NULL)
		{
			MessageBox(NULL,L"opengl3.2 create failed!", L"��ʾ", MB_OK);
			return false;
		}
	}
	wglMakeCurrent(NULL, NULL);
	return true;
}

void OpenGLObject::DrawScene()
{
	glClearColor(0.2f, 0.5f, 0.4f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	CShader m_Shader;
	std::string vertexShaderCode = m_Shader.GetFileStr("..\\ShaderFile\\vertex.glsl");
	std::string fragmentShaderCode = m_Shader.GetFileStr("..\\ShaderFile\\fragment.glsl");
	m_Shader.AddShader(GL_VERTEX_SHADER, vertexShaderCode);
	m_Shader.AddShader(GL_FRAGMENT_SHADER, fragmentShaderCode);
	m_Shader.LinkShaderProgram();
	m_rotate += 1.0f;
	if (m_rotate >= 360.0f)
		m_rotate = 0.0f;
	glm::mat4 model;
	model = glm::rotate(model, glm::radians(m_rotate), glm::vec3(0.0, 1.0, 0.0));
	glm::mat4 view;
	view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	m_Shader.UseProgram();


	glUniformMatrix4fv(glGetUniformLocation(m_Shader.m_shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(m_Shader.m_shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(m_projection));
	glUniformMatrix4fv(glGetUniformLocation(m_Shader.m_shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glBindVertexArray(m_vertexArray);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(0);
	SwapBuffers(wglGetCurrentDC());
}

void OpenGLObject::CreateSceneData()
{
	GLfloat vertices[] = {
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		0.0f, 0.5f, 0.0f
	};
	glGenBuffers(1, &m_vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenVertexArrays(1, &m_vertexArray);
	glBindVertexArray(m_vertexArray);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void OpenGLObject::DestroyOGL()
{
	if (wglGetCurrentContext() != NULL)
		wglMakeCurrent(m_pDC->GetSafeHdc(), NULL);
	if (m_hRc != NULL)
		wglDeleteContext(m_hRc);
	if (m_pDC)
		delete m_pDC;
}

void OpenGLObject::ResetViewPort(int x, int y, int width, int height)
{
	glViewport(x, y, width, height);
}

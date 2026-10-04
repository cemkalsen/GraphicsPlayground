#include "framebuffer.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>


void Framebuffer::create()
{
	glGenFramebuffers(1, &m_framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);


	if (m_config.hasColor)
	{
		glGenTextures(1, &m_textureColorBuffer);

		if (m_config.samples == 1)
		{
			glBindTexture(GL_TEXTURE_2D, m_textureColorBuffer);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_config.width, m_config.height, 0, GL_RGB, GL_FLOAT, NULL);


			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glBindTexture(GL_TEXTURE_2D, 0);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureColorBuffer, 0);

		}
		else
		{
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_textureColorBuffer);
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_config.samples, GL_RGBA16F, m_config.width, m_config.height, GL_TRUE);

			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_textureColorBuffer, 0);
		}
	}
	else
	{
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}


	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	switch (m_config.depthAttachment)
	{
	case DepthAttachment::Renderbuffer:
		glGenRenderbuffers(1, &m_depthAttachment);
		glBindRenderbuffer(GL_RENDERBUFFER, m_depthAttachment);

		if (m_config.samples == 1)
		{
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_config.width, m_config.height);
		}
		else
		{
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_config.samples, GL_DEPTH24_STENCIL8, m_config.width, m_config.height);
		}

		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthAttachment);
		break;

	case DepthAttachment::Texture:
		glGenTextures(1, &m_depthAttachment);
		glBindTexture(GL_TEXTURE_2D, m_depthAttachment);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_config.width, m_config.height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		glBindTexture(GL_TEXTURE_2D, 0);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthAttachment, 0);
		break;

	case DepthAttachment::None:
		break;

	}

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::destroy()
{
	if (m_config.hasColor)
	{
		glDeleteTextures(1, &m_textureColorBuffer);
	}

	switch (m_config.depthAttachment)
	{
	case DepthAttachment::Renderbuffer:
		glDeleteRenderbuffers(1, &m_depthAttachment);
		break;

	case DepthAttachment::Texture:
		glDeleteTextures(1, &m_depthAttachment);
		break;

	case DepthAttachment::None:
		break;

	}

	glDeleteFramebuffers(1, &m_framebuffer);

	m_framebuffer = 0;
	m_textureColorBuffer = 0;
	m_depthAttachment = 0;
}

Framebuffer::Framebuffer(const FramebufferConfig& config)
	: m_config{ config }
{
	create();
}

Framebuffer::~Framebuffer()
{
	destroy();

}

void Framebuffer::bind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
	glViewport(0, 0, m_config.width, m_config.height);
}

void Framebuffer::bindDefault(unsigned int width, unsigned int height)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, width, height);
}

void Framebuffer::resize(unsigned int width, unsigned int height)
{

	// if height or width is set to zero, or if they both are unchanged
	if ((!height || !width) || ((width == m_config.width) && (height == m_config.height)))
	{
		return;
	}

	m_config.width = width;
	m_config.height = height;

	destroy();
	create();
}

void Framebuffer::resolveTo(const Framebuffer& target) const
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_framebuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target.m_framebuffer);
	glBlitFramebuffer(0, 0, m_config.width, m_config.height, 0, 0, target.m_config.width, target.m_config.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
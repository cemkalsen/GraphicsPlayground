#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H


enum class DepthAttachment
{
	Texture,
	Renderbuffer,
	None
};

struct FramebufferConfig
{
	unsigned int width = 0;
	unsigned int height = 0;
	unsigned int samples = 1;
	bool hasColor = true;
	DepthAttachment depthAttachment = DepthAttachment::Renderbuffer;
};

class Framebuffer
{
public:

	explicit Framebuffer(const FramebufferConfig& config);

	Framebuffer(const Framebuffer& other) = delete;

	~Framebuffer();

	Framebuffer& operator=(const Framebuffer& other) = delete;

	// Also sets the viewport to the framebuffer's size
	void bind() const;

	static void bindDefault(unsigned int width, unsigned int height);

	void resize(unsigned int width, unsigned int height);

	// The both framebuffer configs should have the same size
	void resolveTo(const Framebuffer& target) const;



	unsigned int getColorTexture() const
	{
		return m_textureColorBuffer;
	}

	// Only valid with DepthAttachment::Texture
	unsigned int getDepthTexture() const
	{
		return m_depthAttachment;
	}

	unsigned int getWidth() const
	{
		return m_config.width;
	}

	unsigned int getHeight() const
	{
		return m_config.height;
	}


private:

	void create();
	void destroy();

	// m_depthAttachment depends on the selected attachment type in the config struct,
	// it can be a renderbuffer as well as a texture
	unsigned int m_framebuffer = 0, m_textureColorBuffer = 0, m_depthAttachment = 0;
	
	FramebufferConfig m_config;
};


#endif
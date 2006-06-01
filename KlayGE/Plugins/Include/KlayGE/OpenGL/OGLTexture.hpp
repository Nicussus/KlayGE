// OGLTexture.hpp
// KlayGE OpenGL纹理类 头文件
// Ver 2.7.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.7.0
// 增加了AddressingMode, Filtering和Anisotropy (2005.6.27)
// 增加了MaxMipLevel和MipMapLodBias (2005.6.28)
//
// 2.3.0
// 增加了CopyToMemory (2005.2.6)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLTEXTURE_HPP
#define _OGLTEXTURE_HPP

#include <KlayGE/Texture.hpp>

#define NOMINMAX
#include <windows.h>
#include <glloader/glloader.h>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_RenderEngine_OpenGL_d.lib")
#else
	#pragma comment(lib, "KlayGE_RenderEngine_OpenGL.lib")
#endif

namespace KlayGE
{
	class OGLTexture : public Texture
	{
	public:
		OGLTexture(TextureType type);
		virtual ~OGLTexture();

		std::wstring const & Name() const;

		void CustomAttribute(std::string const & name, void* data);

		virtual uint32_t Width(int level) const;
		virtual uint32_t Height(int level) const;
		virtual uint32_t Depth(int level) const;

		virtual void CopyToMemory1D(int level, void* data);
		virtual void CopyToMemory2D(int level, void* data);
		virtual void CopyToMemory3D(int level, void* data);
		virtual void CopyToMemoryCube(CubeFaces face, int level, void* data);

		virtual void CopyMemoryToTexture1D(int level, void* data, PixelFormat pf,
			uint32_t dst_width, uint32_t dst_xOffset, uint32_t src_width);
		virtual void CopyMemoryToTexture2D(int level, void* data, PixelFormat pf,
			uint32_t dst_width, uint32_t dst_height, uint32_t dst_xOffset, uint32_t dst_yOffset,
			uint32_t src_width, uint32_t src_height);
		virtual void CopyMemoryToTexture3D(int level, void* data, PixelFormat pf,
			uint32_t dst_width, uint32_t dst_height, uint32_t dst_depth,
			uint32_t dst_xOffset, uint32_t dst_yOffset, uint32_t dst_zOffset,
			uint32_t src_width, uint32_t src_height, uint32_t src_depth);
		virtual void CopyMemoryToTextureCube(CubeFaces face, int level, void* data, PixelFormat pf,
			uint32_t dst_width, uint32_t dst_height, uint32_t dst_xOffset, uint32_t dst_yOffset,
			uint32_t src_width, uint32_t src_height);

		void BuildMipSubLevels();

		using Texture::Usage;
		void Usage(TextureUsage usage);

		void GLBindTexture();
		GLuint GLTexture() const
		{
			return texture_;
		}
		GLenum GLType() const
		{
			return target_type_;
		}

	protected:
		PixelFormat SRGBToRGB(PixelFormat pf);
		void Convert(GLint& internalFormat, GLenum& glformat, GLenum& gltype, PixelFormat pf);

	protected:
		GLuint texture_;
		GLenum target_type_;
	};

	typedef boost::shared_ptr<OGLTexture> OGLTexturePtr;


	class OGLTexture1D : public OGLTexture
	{
	public:
		OGLTexture1D(uint32_t width, uint16_t numMipMaps, PixelFormat format);

		uint32_t Width(int level) const;

		void CopyToTexture(Texture& target);
		
		void CopyToMemory1D(int level, void* data);

		void CopyMemoryToTexture1D(int level, void* data, PixelFormat pf,
			uint32_t dst_width, uint32_t dst_xOffset, uint32_t src_width);

		RenderViewPtr CreateRenderView(int level);

	private:
		void UpdateParams();

	private:
		std::vector<uint32_t> widths_;
	};

	class OGLTexture2D : public OGLTexture
	{
	public:
		OGLTexture2D(uint32_t width, uint32_t height, uint16_t numMipMaps, PixelFormat format);

		uint32_t Width(int level) const;
		uint32_t Height(int level) const;

		void CopyToTexture(Texture& target);

		void CopyToMemory2D(int level, void* data);

		void CopyMemoryToTexture2D(int level, void* data, PixelFormat pf,
			uint32_t dst_width, uint32_t dst_height, uint32_t dst_xOffset, uint32_t dst_yOffset,
			uint32_t src_width, uint32_t src_height);

		RenderViewPtr CreateRenderView(int level);

	private:
		void UpdateParams();

	private:
		std::vector<uint32_t> widths_;
		std::vector<uint32_t> heights_;
	};

	class OGLTexture3D : public OGLTexture
	{
	public:
		OGLTexture3D(uint32_t width, uint32_t height, uint32_t depth, uint16_t numMipMaps, PixelFormat format);

		uint32_t Width(int level) const;
		uint32_t Height(int level) const;
		uint32_t Depth(int level) const;

		void CopyToTexture(Texture& target);

		void CopyToMemory3D(int level, void* data);

		void CopyMemoryToTexture3D(int level, void* data, PixelFormat pf,
			uint32_t dst_width, uint32_t dst_height, uint32_t dst_depth,
			uint32_t dst_xOffset, uint32_t dst_yOffset, uint32_t dst_zOffset,
			uint32_t src_width, uint32_t src_height, uint32_t src_depth);

		RenderViewPtr CreateRenderView(int slice, int level);

	private:
		void UpdateParams();

	private:
		std::vector<uint32_t> widths_;
		std::vector<uint32_t> heights_;
		std::vector<uint32_t> depths_;
	};

	class OGLTextureCube : public OGLTexture
	{
	public:
		OGLTextureCube(uint32_t size, uint16_t numMipMaps, PixelFormat format);

		uint32_t Width(int level) const;
		uint32_t Height(int level) const;

		void CopyToTexture(Texture& target);

		void CopyToMemoryCube(CubeFaces face, int level, void* data);

		void CopyMemoryToTextureCube(CubeFaces face, int level, void* data, PixelFormat pf,
			uint32_t dst_width, uint32_t dst_height, uint32_t dst_xOffset, uint32_t dst_yOffset,
			uint32_t src_width, uint32_t src_height);

		RenderViewPtr CreateRenderView(CubeFaces face, int level);

	private:
		void UpdateParams();

	private:
		std::vector<uint32_t> widths_;
	};
}

#endif			// _OGLTEXTURE_HPP

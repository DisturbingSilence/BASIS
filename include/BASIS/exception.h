#pragma once

#include <string>
#include <exception>

#define BASIS_DECLARE_EXCEPTION(child)			\
struct child : public BaseException				\
{												\
	using BaseException::BaseException;			\
	template<typename... Args>					\
	child(Args&&... args) :						\
	BaseException(std::forward<Args>(args)...)	\
	{}											\
};	
namespace BASIS
{	
	/*
	 * Possible reasons for all exceptions:
	 * 
	 * BaseException should never be thrown, acts as a template
	 *
	 * PipelineException:
	 * - linking failure
	 * - compilation failure
	 * 
	 * AssetException:
	 * - stbi failed to load image
	 * - unknown texture source when loading model
	 * - getTexture(uint64_t) was called with unrecognized hash
	 * - getModel(uint64_t) was called with unrecognized hash
	 * - insertModel/insertTexture cannot insert asset, as it already exists
	 * - outdated KTX texture passed to loadTexture() instead of KTX2
	 * FileException:
	 * - file not found
	 * - file already exists
	 * - unsupported file format
	 * ApplicationException:
	 * - window initialization failure
	 * - glad initialization failure
	 * - Bindless textures are unsupported
	 * 
	 * */
	 
	struct BaseException : std::exception
	{
		BaseException() = default;
		template<typename... Args>
		explicit BaseException(Args&&... args)
		{
			((m_message += args), ...);
		}
		const char* what() const noexcept override
		{
			return m_message.c_str();
		}
		protected:
			std::string m_message;
	};
	BASIS_DECLARE_EXCEPTION(FileException);
	BASIS_DECLARE_EXCEPTION(AssetException);
	BASIS_DECLARE_EXCEPTION(PipelineException);
	BASIS_DECLARE_EXCEPTION(ApplicationException);
};
#undef DECLARE_BASIS_EXCEPTION

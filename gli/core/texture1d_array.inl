#include "../levels.hpp"

namespace gli
{
	texture1d_array::texture1d_array()
	{}

	texture1d_array::texture1d_array(format_type Format, extent_type const& Extent, size_type Layers, swizzles_type const& Swizzles)
		: texture(TARGET_1D_ARRAY, Format, texture::extent_type(Extent.x, 1, 1), Layers, 1, gli::levels(Extent), Swizzles)
	{}

	texture1d_array::texture1d_array(format_type Format, extent_type const& Extent, size_type Layers, size_type Levels, swizzles_type const& Swizzles)
		: texture(TARGET_1D_ARRAY, Format, texture::extent_type(Extent.x, 1, 1), Layers, 1, Levels, Swizzles)
	{}

	texture1d_array::texture1d_array(texture const& Texture)
		: texture(Texture, TARGET_1D_ARRAY, Texture.format())
	{}

	texture1d_array::texture1d_array
	(
		texture const& Texture,
		format_type Format,
		size_type BaseLayer, size_type MaxLayer,
		size_type BaseFace, size_type MaxFace,
		size_type BaseLevel, size_type MaxLevel,
		swizzles_type const& Swizzles
	)
		: texture(
			Texture, TARGET_1D_ARRAY, Format,
			BaseLayer, MaxLayer,
			BaseFace, MaxFace,
			BaseLevel, MaxLevel,
			Swizzles)
	{}

	texture1d_array::texture1d_array
	(
		texture1d_array const& Texture,
		size_type BaseLayer, size_type MaxLayer,
		size_type BaseLevel, size_type MaxLevel
	)
		: texture(
			Texture, TARGET_1D_ARRAY,
			Texture.format(),
			Texture.base_layer() + BaseLayer, Texture.base_layer() + MaxLayer,
			Texture.base_face(), Texture.max_face(),
			Texture.base_level() + BaseLevel, Texture.base_level() + MaxLevel)
	{}

	texture1d texture1d_array::operator[](size_type Layer) const
	{
		GLI_ASSERT(!this->empty());
		GLI_ASSERT(Layer < this->layers());

		return texture1d(
			*this, this->format(),
			this->base_layer() + Layer, this->base_layer() + Layer,
			this->base_face(), this->max_face(),
			this->base_level(), this->max_level());
	}

	texture1d_array::extent_type texture1d_array::extent(size_type Level) const
	{
		return extent_type(this->texture::extent(Level));
	}

	template <typename gen_type>
	gen_type texture1d_array::load(extent_type const& TexelCoord, size_type Layer, size_type Level) const
	{
		return this->texture::load<gen_type>(texture::extent_type(TexelCoord.x, 0, 0), Layer, 0, Level);
	}

	template <typename gen_type>
	void texture1d_array::store(extent_type const& TexelCoord, size_type Layer, size_type Level, gen_type const& Texel)
	{
		this->texture::store<gen_type>(texture::extent_type(TexelCoord.x, 0, 0), Layer, 0, Level, Texel);
	}
}//namespace gli



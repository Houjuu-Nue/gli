namespace gli{
namespace detail
{
	size_t texel_linear_addressing
	(
		extent1d const& Extent,
		extent1d const& TexelCoord
	)
	{
		GLI_ASSERT(glm::all(glm::lessThan(TexelCoord, Extent)));

		return static_cast<size_t>(TexelCoord.x);
	}

	size_t texel_linear_addressing
	(
		extent2d const& Extent,
		extent2d const& TexelCoord
	)
	{
		GLI_ASSERT(TexelCoord.x < Extent.x);
		GLI_ASSERT(TexelCoord.y < Extent.y);

		return static_cast<size_t>(TexelCoord.x + Extent.x * TexelCoord.y);
	}

	size_t texel_linear_addressing
	(
		extent3d const& Extent,
		extent3d const& TexelCoord
	)
	{
		GLI_ASSERT(TexelCoord.x < Extent.x);
		GLI_ASSERT(TexelCoord.y < Extent.y);
		GLI_ASSERT(TexelCoord.z < Extent.z);

		return static_cast<size_t>(TexelCoord.x + Extent.x * (TexelCoord.y + Extent.y * TexelCoord.z));
	}

	size_t texel_morton_addressing
	(
		extent1d const& Extent,
		extent1d const& TexelCoord
	)
	{
		GLI_ASSERT(TexelCoord.x < Extent.x);

		return TexelCoord.x;
	}

	size_t texel_morton_addressing
	(
		extent2d const& Extent,
		extent2d const& TexelCoord
	)
	{
		GLI_ASSERT(TexelCoord.x < Extent.x && TexelCoord.x >= 0 && TexelCoord.x < std::numeric_limits<extent2d::value_type>::max());
		GLI_ASSERT(TexelCoord.y < Extent.y && TexelCoord.y >= 0 && TexelCoord.y < std::numeric_limits<extent2d::value_type>::max());

		glm::u32vec2 const Input(TexelCoord);

		return static_cast<size_t>(glm::bitfieldInterleave(Input.x, Input.y));
	}

	size_t texel_morton_addressing
	(
		extent3d const& Extent,
		extent3d const& TexelCoord
	)
	{
		GLI_ASSERT(TexelCoord.x < Extent.x);
		GLI_ASSERT(TexelCoord.y < Extent.y);
		GLI_ASSERT(TexelCoord.z < Extent.z);

		glm::u32vec3 const Input(TexelCoord);

		return static_cast<size_t>(glm::bitfieldInterleave(Input.x, Input.y, Input.z));
	}
}//namespace detail

	image::image()
		: Format(gli::FORMAT_UNDEFINED)
		, BaseLevel(0)
		, Data(nullptr)
		, Size(0)
	{}

	image::image
	(
		format_type Format,
		extent_type const& Extent
	)
		: Storage(std::make_shared<storage_linear>(Format, Extent, 1, 1, 1))
		, Format(Format)
		, BaseLevel(0)
		, Data(Storage->data())
		, Size(compute_size(0))
	{}

	image::image
	(
		std::shared_ptr<storage_linear> Storage,
		format_type Format,
		size_type BaseLayer,
		size_type BaseFace,
		size_type BaseLevel
	)
		: Storage(Storage)
		, Format(Format)
		, BaseLevel(BaseLevel)
		, Data(compute_data(BaseLayer, BaseFace, BaseLevel))
		, Size(compute_size(BaseLevel))
	{}

	image::image(
		texture const& Texture,
		format_type Format,
		size_type BaseLayer,
		size_type BaseFace,
		size_type BaseLevel
	)
		: Storage(Texture.Storage)
		, Format(Format)
		, BaseLevel(BaseLevel)
		, Data(compute_data(BaseLayer, BaseFace, BaseLevel))
		, Size(compute_size(BaseLevel))
	{}

	image::image
	(
		image const & Image,
		format_type Format
	)
		: Storage(Image.Storage)
		, Format(Format)
		, BaseLevel(Image.BaseLevel)
		, Data(Image.Data)
		, Size(Image.Size)
	{
		GLI_ASSERT(block_size(Format) == block_size(Image.format()));
	}

	bool image::empty() const
	{
		if(this->Storage.get() == nullptr)
			return true;

		return this->Storage->empty();
	}

	image::size_type image::size() const
	{
		GLI_ASSERT(!this->empty());

		return this->Size;
	}

	template <typename genType>
	image::size_type image::size() const
	{
		GLI_ASSERT(sizeof(genType) <= this->Storage->block_size());

		return this->size() / sizeof(genType);
	}

	image::format_type image::format() const
	{
		return this->Format;
	}

	image::extent_type image::extent() const
	{
		GLI_ASSERT(!this->empty());

		storage_linear::extent_type const& SrcExtent = this->Storage->extent(this->BaseLevel);
		storage_linear::extent_type const& DstExtent = SrcExtent * block_extent(this->format()) / this->Storage->block_extent();

		return glm::max(DstExtent, storage_linear::extent_type(1));
	}

	void* image::data()
	{
		GLI_ASSERT(!this->empty());

		return this->Data;
	}

	void const* image::data() const
	{
		GLI_ASSERT(!this->empty());

		return this->Data;
	}

	template <typename genType>
	genType* image::data()
	{
		GLI_ASSERT(!this->empty());
		GLI_ASSERT(this->Storage->block_size() >= sizeof(genType));

		return reinterpret_cast<genType *>(this->data());
	}

	template <typename genType>
	genType const* image::data() const
	{
		GLI_ASSERT(!this->empty());
		GLI_ASSERT(this->Storage->block_size() >= sizeof(genType));

		return reinterpret_cast<genType const *>(this->data());
	}

	void image::clear()
	{
		GLI_ASSERT(!this->empty());

		memset(this->data<gli::byte>(), 0, this->size<gli::byte>());
	}

	template <typename genType>
	void image::clear(genType const& Texel)
	{
		GLI_ASSERT(!this->empty());
		GLI_ASSERT(this->Storage->block_size() == sizeof(genType));

		for(size_type TexelIndex = 0; TexelIndex < this->size<genType>(); ++TexelIndex)
			*(this->data<genType>() + TexelIndex) = Texel;
	}

	image::data_type* image::compute_data(size_type BaseLayer, size_type BaseFace, size_type BaseLevel)
	{
		size_type const BaseOffset = this->Storage->base_offset(BaseLayer, BaseFace, BaseLevel);

		return this->Storage->data() + BaseOffset;
	}

	image::size_type image::compute_size(size_type Level) const
	{
		GLI_ASSERT(!this->empty());

		return this->Storage->level_size(Level);
	}

	template <typename genType>
	genType image::load(extent_type const& TexelCoord)
	{
		GLI_ASSERT(!this->empty());
		GLI_ASSERT(!is_compressed(this->format()));
		GLI_ASSERT(this->Storage->block_size() == sizeof(genType));
		GLI_ASSERT(glm::all(glm::lessThan(TexelCoord, this->extent())));

		return *(this->data<genType>() + detail::texel_linear_addressing(this->extent(), TexelCoord));
	}

	template <typename genType>
	void image::store(extent_type const& TexelCoord, genType const& Data)
	{
		GLI_ASSERT(!this->empty());
		GLI_ASSERT(!is_compressed(this->format()));
		GLI_ASSERT(this->Storage->block_size() == sizeof(genType));
		GLI_ASSERT(glm::all(glm::lessThan(TexelCoord, this->extent())));

		*(this->data<genType>() + detail::texel_linear_addressing(this->extent(), TexelCoord)) = Data;
	}

	int image::get_shared_storage_count() const {

		return static_cast<int>(this->Storage.use_count());
	}

	void destroy_image(image && Image) {
		Image.~image();
	}
}//namespace gli

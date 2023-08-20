#pragma once

namespace VeryCoolEngine {

	
	enum class ShaderDataType : uint8_t {
		Float,
		Float2,
		Float3,
		Float4,
		Int,
		Int2,
		Int3,
		Int4,
		UInt,
		UInt2,
		UInt3,
		UInt4,
		Mat3,
		Mat4,
		Bool,
		None
	};

	static uint32_t ShaderDataTypeNumElements(ShaderDataType t) {
		switch (t) {
		case ShaderDataType::Float: return 1;
		case ShaderDataType::Float2: return 2;
		case ShaderDataType::Float3: return 3;
		case ShaderDataType::Float4: return 4;
		case ShaderDataType::Int: return 1;
		case ShaderDataType::Int2: return 2;
		case ShaderDataType::Int3: return  3;
		case ShaderDataType::Int4: return  4;
		case ShaderDataType::UInt: return 1;
		case ShaderDataType::UInt2: return 2;
		case ShaderDataType::UInt3: return 3;
		case ShaderDataType::UInt4: return 4;
		case ShaderDataType::Mat3: return  3;
		case ShaderDataType::Mat4: return  4;
		case ShaderDataType::Bool: return 1;
		}
		VCE_ERROR("Trying to calculate number of elements in ShaderDataType::None");
	}

	static uint32_t ShaderDataTypeSize(ShaderDataType t) {
		switch (t) {
		case ShaderDataType::Float: return sizeof(float);
		case ShaderDataType::Float2: return sizeof(float) * 2;
		case ShaderDataType::Float3: return sizeof(float) * 3;
		case ShaderDataType::Float4: return sizeof(float) * 4;
		case ShaderDataType::Int: return sizeof(int);
		case ShaderDataType::Int2: return sizeof(int) * 2;
		case ShaderDataType::Int3: return sizeof(int) * 3;
		case ShaderDataType::Int4: return sizeof(int) * 4;
		case ShaderDataType::UInt: return sizeof(unsigned int);
		case ShaderDataType::UInt2: return sizeof(unsigned int) * 2;
		case ShaderDataType::UInt3: return sizeof(unsigned int) * 3;
		case ShaderDataType::UInt4: return sizeof(unsigned int) * 4;
		case ShaderDataType::Mat3: return sizeof(float) * 3 * 3;
		case ShaderDataType::Mat4: return sizeof(float) * 4 * 4;
		case ShaderDataType::Bool: return sizeof(bool);
		}
		VCE_ERROR("Trying to calculate size of ShaderDataType::None");
	}

	struct BufferElement {
		std::string _Name;
		uint32_t _Offset;
		uint32_t _Size;
		ShaderDataType _Type;
		bool _Normalized;
		bool _Instanced = false;
		unsigned int _divisor = 0;
		void* _data = nullptr;
		unsigned int _numEntries = 0;

		BufferElement(ShaderDataType type, const std::string& name, bool normalized = false, bool instanced = false, unsigned int divisor = 0, void* data = nullptr,unsigned int numEntries = 0) : _Name(name), _Type(type), _Size(ShaderDataTypeSize(type)), _Offset(0), _Normalized(normalized), _Instanced(instanced), _divisor(divisor), _data(data), _numEntries(numEntries) {
			
		}

		BufferElement() {};

		uint32_t GetComponentCount() const {
			switch (_Type) {
			case ShaderDataType::Float: return 1;
			case ShaderDataType::Float2: return 2;
			case ShaderDataType::Float3: return 3;
			case ShaderDataType::Float4: return 4;
			case ShaderDataType::Int: return 1;
			case ShaderDataType::Int2: return 2;
			case ShaderDataType::Int3: return 3;
			case ShaderDataType::Int4: return 4;
			case ShaderDataType::UInt: return 1;
			case ShaderDataType::UInt2: return 2;
			case ShaderDataType::UInt3: return 3;
			case ShaderDataType::UInt4: return 4;
			case ShaderDataType::Mat3: return 3 * 3;
			case ShaderDataType::Mat4: return 4 * 4;
			case ShaderDataType::Bool: return 1;
			}
		}
	};

	class BufferLayout {
	public:
		BufferLayout(const std::initializer_list<BufferElement>& elements) : _Elements(elements) {
			CalculateOffsetsAndStrides();
		};
		BufferLayout() {};
		std::vector<BufferElement>& GetElements() { return _Elements; };

		std::vector<BufferElement>::iterator begin() { return _Elements.begin(); }
		std::vector<BufferElement>::iterator end() { return _Elements.end(); }
		std::vector<BufferElement>::const_iterator begin() const { return _Elements.begin(); }
		std::vector<BufferElement>::const_iterator end() const { return _Elements.end(); }
		uint32_t GetStride() const { return _Stride; }
		void CalculateOffsetsAndStrides() {
			uint32_t offset = 0;
			_Stride = 0;
			for (BufferElement& element : _Elements) {
				element._Offset = offset;
				offset += element._Size;
				_Stride += element._Size;

			}
		}
	private:
		
		std::vector<BufferElement> _Elements;
		uint32_t _Stride = 0;
	};

	class VertexBuffer{
	public:
		virtual ~VertexBuffer() = default;

		virtual void UploadData() = 0;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void SetLayout(const BufferLayout& layout) = 0;
		virtual const BufferLayout& GetLayout() = 0;

		static VertexBuffer* Create(void* verts, size_t size);

	protected:
		BufferLayout _Layout;
	};
	
	class IndexBuffer {
	
	public:
		virtual ~IndexBuffer() = default;

		virtual void UploadData() = 0;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		static IndexBuffer* Create(uint32_t* indices, uint32_t count);

		uint32_t GetCount() const { return _Count; }
	protected:
		uint32_t _Count;
	};
}


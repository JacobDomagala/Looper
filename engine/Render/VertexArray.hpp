#pragma once

#include "VertexBuffer.hpp"

#include <vector>
#include <memory>

namespace dgame {

	class VertexArray
	{
	public:
		VertexArray();
		 ~VertexArray();

		 void Bind() const ;
		 void Unbind() const ;

		 void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) ;
		 void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) ;

		 const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const { return m_VertexBuffers; }
		 const std::shared_ptr<IndexBuffer>& GetIndexBuffer() const { return m_IndexBuffer; }

	private:
		uint32_t m_RendererID;
		uint32_t m_VertexBufferIndex = 0;
		std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;
		std::shared_ptr<IndexBuffer> m_IndexBuffer;
	};

}

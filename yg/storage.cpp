#include "../base/SRC_FIRST.hpp"
#include "storage.hpp"
#include "buffer_object.hpp"
#include "internal/opengl.hpp"

namespace yg
{
  namespace gl
  {
    Storage::Storage()
    {}

    Storage::Storage(size_t vbSize, size_t ibSize) :
      m_vertices(new BufferObject(vbSize, GL_ARRAY_BUFFER)),
      m_indices(new BufferObject(ibSize, GL_ELEMENT_ARRAY_BUFFER))
    {}
  }
}

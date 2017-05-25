#ifndef __Array_h__
#define __Array_h__


#include "common.h"
#include "Error.h"


template <typename DataType>
class Array {

public:

    Array(__INPUT u32_t length) :
        m_Array(NULL),
        m_Width(length),
        m_Height(1),
        m_Length(length),
        m_Size(length * sizeof(DataType)),
        m_Dimension(1)
    {
        try {
            m_Array = new DataType [m_Length];
            if (m_Array == NULL) {
                throw Error::ERROR_MEMORY_ALLOC;
            }
        }
        catch (const err_t code) {
            PRINT_ERROR_CODE(code);
            throw;
        }
    }

    Array(__INPUT u32_t width,
          __INPUT u32_t height) :
        m_Array(NULL),
        m_Width(width),
        m_Height(height),
        m_Length(width * height),
        m_Size(width * height * sizeof(DataType)),
        m_Dimension(2)
    {
        try {
            m_Array = new DataType [m_Length];
            if (m_Array == NULL) {
                throw Error::ERROR_MEMORY_ALLOC;
            }
        }
        catch (const err_t code) {
            PRINT_ERROR_CODE(code);
            throw;
        }
    }

    Array(__INPUT Array &object) :
        m_Array(NULL),
        m_Width(object.m_Width),
        m_Height(object.m_Height),
        m_Length(object.m_Length),
        m_Size(object.m_Size),
        m_Dimension(object.m_Dimension)
    {
        try {
            m_Array = new DataType [object.m_Length];
            if (m_Array == NULL) {
                throw Error::ERROR_MEMORY_ALLOC;
            }

            for (u32_t i = 0; i < m_Length; i++) {
                m_Array[i] = object.m_Array[i];
            }
        }
        catch (const err_t code) {
            PRINT_ERROR_CODE(code);
            throw;
        }
    }

    virtual ~Array()
    {
        delete[] m_Array;
    }

    DataType &At(__INPUT u32_t index)
    {
        try {
            if (m_Array == NULL) {
                throw Error::ERROR_MEMORY_ALLOC;
            }

            if ((m_Dimension != 1) || (index >= m_Size)) {
                throw Error::ERROR_INVALID_ARGUMENTS;
            }

            return m_Array[index];
        }
        catch (const err_t code) {
            PRINT_ERROR_CODE(code);

            return m_Array[0];
        }
    }

    DataType &At(__INPUT u32_t colIndex,
                 __INPUT u32_t rowIndex)
    {
        try {
            if (m_Array == NULL) {
                throw Error::ERROR_MEMORY_ALLOC;
            }

            u32_t index = rowIndex * m_Width + colIndex;
            if ((m_Dimension != 2) || (index >= m_Size)) {
                throw Error::ERROR_INVALID_ARGUMENTS;
            }

            return m_Array[index];
        }
        catch (const err_t code) {
            PRINT_ERROR_CODE(code);

            return m_Array[0];
        }
    }

    DataType *GetData(__INPUT u32_t index = 0)
    {
        try {
            if (m_Array == NULL) {
                throw Error::ERROR_MEMORY_ALLOC;
            }

            if ((m_Dimension != 1) || (index >= m_Size)) {
                throw Error::ERROR_INVALID_ARGUMENTS;
            }

            return &m_Array[index];
        }
        catch (const err_t code) {
            PRINT_ERROR_CODE(code);

            return NULL;
        }
    }

    DataType *GetData(__INPUT u32_t colIndex,
                      __INPUT u32_t rowIndex)
    {
        try {
            if (m_Array == NULL) {
                throw Error::ERROR_MEMORY_ALLOC;
            }

            u32_t index = rowIndex * m_Width + colIndex;
            if ((m_Dimension != 2) || (index >= m_Size)) {
                throw Error::ERROR_INVALID_ARGUMENTS;
            }

            return &m_Array[index];
        }
        catch (const err_t code) {
            PRINT_ERROR_CODE(code);

            return NULL;
        }
    }

    u32_t GetWidth() const
    {
        return m_Width;
    }

    u32_t GetHeight() const
    {
        return m_Height;
    }

    u32_t GetLength() const
    {
        return m_Length;
    }

    u32_t GetSize() const
    {
        return m_Size;
    }

    Array operator=(__INPUT Array &object)
    {
        try {
            m_Array = new DataType [object.m_Length];
            if (m_Array == NULL) {
                throw Error::ERROR_MEMORY_ALLOC;
            }
            
            for (u32_t i = 0; i < m_Length; i++) {
                m_Array[i] = object.m_Array[i];
            }

            m_Width     = object.m_Width;
            m_Height    = object.m_Height;
            m_Length    = object.m_Length;
            m_Size      = object.m_Size;
            m_Dimension = object.m_Dimension;

            return *this;
        }
        catch (const err_t code) {
            PRINT_ERROR_CODE(code);
            throw;
        }
    }

private:

    DataType *m_Array;
    u32_t    m_Width;
    u32_t    m_Height;
    u32_t    m_Length;
    u32_t    m_Size;
    u32_t    m_Dimension;

};


#endif // __Array_h__

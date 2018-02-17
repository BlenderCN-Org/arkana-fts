/**
 * \file DataContainer.h
 * \author Pompei2
 * \date 01 May 2006
 * \brief This file contains the definition of a raw data container.
 **/

#ifndef D_DATA_CONTAINER_H
#define D_DATA_CONTAINER_H

namespace FTS {
    class ConstRawDataContainer;

/// This abstract class (interface) is the base for any data container.
/// A data container should not do much but only store some data.
/// Compression, structured reading and more are (currently) not done by this
/// class's family.\n
/// The point of this class is to first have the data and its size in one same
/// location and second have somebody handle the data's memory.
class DataContainer
{
public:
    /// \return A read-only pointer to the data.
    virtual const uint8_t *getData() const = 0;
    /// \return The size in bytes of the data returned by \a getData.
    virtual size_t getSize() const = 0;

    uint32_t fletcher32() const;

    virtual ~DataContainer() {};

protected:
    DataContainer() {};
    DataContainer(const DataContainer&) {};
};

class RawDataContainer : public DataContainer {
protected:
    /// The data managed by this container.
    uint8_t *m_pData = nullptr;
    /// The size of the data managed by this container.
    size_t m_uiSize = 0;

public:
    /// Constructs a data container with no data :)
    RawDataContainer();
    
    /// Constructs a data container that allocates \a in_uiSize bytes of data.
    /// \param in_uiSize The size (in bytes) of the data to hold.
    RawDataContainer(size_t in_uiSize);
    
    /// Copies the whole data of some other data container.
    RawDataContainer(const DataContainer &);

    /// Copies the whole data of some other data container.
    RawDataContainer(const RawDataContainer &);

    /// Destroys the data container and deallocates the data it contains.
    virtual ~RawDataContainer();

    /// \return A modifiable pointer to the data.
    virtual uint8_t *getData();
    /// \return A read-only pointer to the data.
    const uint8_t *getData() const override;
    /// \return The size in bytes of the data returned by \a getData.
    size_t getSize() const override;

    /// Resizes the data that I hold (similar to realloc)
    /// \param in_uiNewSize The new size that the data should have. If this is
    ///                     less then the current size, data at the end will be
    ///                     cut off. If it is more, all data is kept.
    virtual void resize(size_t in_uiNewSize);
    
    /// Grows the data that I hold (similar to realloc)
    /// \param in_uiAdditional How much more space should be reserved at the end
    ///                        of the buffer.
    /// \note If additional size is more than 1GB nothing is changed.
    virtual void grow(size_t in_uiAdditional);
    
    /// Deallocates all data that is hold.
    /// \note this is automatically called in the destructor.
    virtual void destroy();
    
    /// \return A copy of this data container (containing a copy of the whole data).
    virtual RawDataContainer *copy() const;

    /// Appends some other data container's data onto myself.
    /// \param in_other What data container to get my data from.
    virtual void append(const ConstRawDataContainer& in_other);
};

/// This class represents a data container that is only good for reading the
/// data, neither modifying it, nor creating it.\n
/// Also, it does not destroy the data when it is being destroyed itself.
class ConstRawDataContainer : public DataContainer {
protected:
    /// Pointer to the data.
    const uint8_t * const m_pData;
    /// Size of the data I point to.
    const size_t m_uiSize;

public:
    /// Default constructor. Creates empty data.
    ConstRawDataContainer();
    /// Creates a const data container that points to the data of a raw data container.
    /// \param in_o The data to handle.
    ConstRawDataContainer(const DataContainer &in_o);
    /// Creates a const data container that points to some given data with some
    /// given size. The data will not be destroyed by this container.
    /// \param in_pData The data to handle.
    /// \param in_uiSize The size of the data to handle.
    ConstRawDataContainer(const uint8_t * const in_pData, size_t in_uiSize);
    /// Creates a const data container that points to some given data with some
    /// given size. The data will not be destroyed by this container.
    /// \param in_pData The data to handle.
    /// \param in_uiSize The size of the data to handle.
    ConstRawDataContainer(const void * const in_pData, size_t in_uiSize);
    /// Default destructor. Does not destroy the data it points to.
    virtual ~ConstRawDataContainer();

    /// \return A read-only pointer to the data.
    const uint8_t *getData() const override;
    /// \return The size in bytes of the data returned by \a getData.
    size_t getSize() const override;
    /// \return A copy of this data container (NOT a copy of the whole data).
    virtual DataContainer *copy() const;
};

/// Converts a data buffer from little endian into the system's endian.
/// \param out_pBuff The data whose endianness to swap.
/// \param in_uiSize The size of the data.
/// \note It's intentional that this function doesn't get a DataContainer as parameter.
void toSystemEndian(uint8_t *out_pBuff, size_t in_uiSize);
/// Checks if the system has the endianness like we store it on disk (little endian)
/// \return true if the system uses the little endian, false if not.
bool systemHasGoodEndian();

};

#endif // D_DATA_CONTAINER_H

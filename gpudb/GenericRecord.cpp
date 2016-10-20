#include "gpudb/GenericRecord.hpp"

#include "gpudb/Avro.hpp"
#include "gpudb/GPUdbException.hpp"

#include <avro/Compiler.hh>
#include <boost/lexical_cast.hpp>

namespace gpudb
{
    GenericRecord::GenericRecord(const Type& type) :
        m_type(type)
    {
        const std::vector<Type::Column>& columns = type.getColumns();
        size_t columnCount = columns.size();
        m_values.reserve(columnCount);

        for (size_t i = 0; i < columnCount; ++i)
        {
            const Type::Column& column = columns[i];
            initializeColumn(column.getType(), column.isNullable());
        }
    }

    const Type& GenericRecord::getType() const
    {
        return m_type;
    }

    const ::avro::ValidSchema& GenericRecord::getSchema() const
    {
        return m_type.getSchema();
    }

    #define DEFINE_ACCESSORS(T, TN) \
        T& GenericRecord::as##TN(const size_t index) \
        { \
            return *boost::any_cast<T>(&m_values[index]); \
        } \
        \
        const T& GenericRecord::as##TN(const size_t index) const \
        { \
            return *boost::any_cast<T>(&m_values[index]); \
        } \
        \
        T& GenericRecord::as##TN(const std::string& name) \
        { \
            return *boost::any_cast<T>(&m_values[m_type.getColumnIndex(name)]); \
        } \
        \
        const T& GenericRecord::as##TN(const std::string& name) const \
        { \
            return *boost::any_cast<T>(&m_values[m_type.getColumnIndex(name)]); \
        }

    DEFINE_ACCESSORS(std::vector<uint8_t>, Bytes)
    DEFINE_ACCESSORS(boost::optional<std::vector<uint8_t> >, NullableBytes)
    DEFINE_ACCESSORS(double, Double)
    DEFINE_ACCESSORS(boost::optional<double>, NullableDouble)
    DEFINE_ACCESSORS(float, Float)
    DEFINE_ACCESSORS(boost::optional<float>, NullableFloat)
    DEFINE_ACCESSORS(int32_t, Int)
    DEFINE_ACCESSORS(boost::optional<int32_t>, NullableInt)
    DEFINE_ACCESSORS(int64_t, Long)
    DEFINE_ACCESSORS(boost::optional<int64_t>, NullableLong)
    DEFINE_ACCESSORS(std::string, String)
    DEFINE_ACCESSORS(boost::optional<std::string>, NullableString)

    #define DECODE_VALUE(T) { \
        if (isNullable) \
        { \
            ::avro::decode(decoder, *boost::any_cast<boost::optional<T> >(&value)); \
        } \
        else \
        { \
            ::avro::decode(decoder, *boost::any_cast<T>(&value)); \
        } \
        \
        break; \
    }

    void decodeValue(::avro::Decoder& decoder, const Type::Column::ColumnType type, const bool isNullable, boost::any& value)
    {
        switch (type)
        {
            case Type::Column::BYTES: DECODE_VALUE(std::vector<uint8_t>)
            case Type::Column::DOUBLE: DECODE_VALUE(double)
            case Type::Column::FLOAT: DECODE_VALUE(float)
            case Type::Column::INT: DECODE_VALUE(int32_t)
            case Type::Column::LONG: DECODE_VALUE(int64_t)
            case Type::Column::STRING: DECODE_VALUE(std::string)
        }
    }

    #define ENCODE_VALUE(T) { \
        if (isNullable) \
        { \
            ::avro::encode(encoder, *boost::any_cast<boost::optional<T> >(&value)); \
        } \
        else \
        { \
            ::avro::encode(encoder, *boost::any_cast<T>(&value)); \
        } \
        \
        break; \
    }

    void encodeValue(::avro::Encoder& encoder, const Type::Column::ColumnType type, const bool isNullable, const boost::any& value)
    {
        switch (type)
        {
            case Type::Column::BYTES: ENCODE_VALUE(std::vector<uint8_t>)
            case Type::Column::DOUBLE: ENCODE_VALUE(double)
            case Type::Column::FLOAT: ENCODE_VALUE(float)
            case Type::Column::INT: ENCODE_VALUE(int32_t)
            case Type::Column::LONG: ENCODE_VALUE(int64_t)
            case Type::Column::STRING: ENCODE_VALUE(std::string)
        }
    }

    void GenericRecord::transpose(const std::string& schemaString, const std::vector<uint8_t>& encodedData, std::vector<GenericRecord>& result)
    {
        ::avro::ValidSchema schema;

        try
        {
            schema = ::avro::compileJsonSchemaFromString(schemaString);
        }
        catch (const std::exception& ex)
        {
            throw GPUdbException("Schema is invalid: " + std::string(ex.what()));
        }

        const ::avro::NodePtr& root = schema.root();

        if (root->type() != ::avro::AVRO_RECORD)
        {
            throw GPUdbException("Schema must be of type record.");
        }

        size_t fieldCount = root->leaves() - 1;
        std::vector<std::pair<Type::Column::ColumnType, bool> > columnTypes;
        columnTypes.reserve(fieldCount);

        for (size_t i = 0; i < fieldCount; ++i)
        {
            const ::avro::NodePtr& field = root->leafAt(i);

            if (field->type() != ::avro::AVRO_ARRAY)
            {
                throw GPUdbException("Field " + root->nameAt(i) + " must be of type array.");
            }

            const ::avro::NodePtr& fieldNode = field->leafAt(0);
            ::avro::Type fieldType = fieldNode->type();
            bool isNullable = false;

            if (fieldType == ::avro::AVRO_UNION)
            {
                if ((fieldNode->leaves() == 2) && (fieldNode->leafAt(1)->type() == ::avro::AVRO_NULL))
                {
                    fieldType = fieldNode->leafAt(0)->type();
                    isNullable = true;
                }
                else
                {
                    throw GPUdbException("Field " + root->nameAt(i) + " has invalid type.");
                }
            }

            switch (fieldType)
            {
                case ::avro::AVRO_BYTES:
                case ::avro::AVRO_DOUBLE:
                case ::avro::AVRO_FLOAT:
                case ::avro::AVRO_INT:
                case ::avro::AVRO_LONG:
                case ::avro::AVRO_STRING:
                    break;

                default:
                    throw GPUdbException("Field " + root->nameAt(i) + " must be of type bytes, double, float, int, long or string.");
            }

            columnTypes.push_back(std::make_pair(static_cast<Type::Column::ColumnType>(fieldType), isNullable));
        }

        std::vector<std::string> expressions;
        expressions.reserve(fieldCount);

        try
        {
            ::avro::DecoderPtr decoder = ::avro::binaryDecoder();
            std::auto_ptr< ::avro::InputStream> inputStream = ::avro::memoryInputStream(&encodedData[0], encodedData.size());
            decoder->init(*inputStream);
            size_t recordCount = 0;

            for (size_t i = 0; i < fieldCount; ++i)
            {
                const std::pair<Type::Column::ColumnType, bool>& columnType = columnTypes[i];
                size_t recordNumber = 0;

                for (size_t j = decoder->arrayStart(); j != 0; j = decoder->arrayNext())
                {
                    if ((i == 0) && (recordNumber + j > recordCount))
                    {
                        recordCount += j;
                        result.resize(recordCount, GenericRecord(columnTypes));
                    }

                    for (size_t k = 0; k < j; ++k)
                    {
                        if (recordNumber < recordCount)
                        {
                            decodeValue(*decoder, columnType.first, columnType.second, result[recordNumber].m_values[i]);
                        }

                        ++recordNumber;
                    }
                }

                if (recordNumber != recordCount)
                {
                    throw GPUdbException("Fields must all have the same number of elements.");
                }
            }

            ::avro::decode(*decoder, expressions);

            if (expressions.size() < fieldCount)
            {
                throw GPUdbException("Every field must have a corresponding expression.");
            }
        }
        catch (const std::exception& ex)
        {
            throw GPUdbException("Could not decode data: " + std::string(ex.what()));
        }

        std::vector<Type::Column> columns;
        columns.reserve(fieldCount);
        std::vector<std::string> nullable;

        for (size_t i = 0; i < fieldCount; ++i)
        {
            for (size_t j = 0; j < i; ++j)
            {
                if (expressions[i] == columns[j].getName())
                {
                    for (size_t n = 2; ; ++n)
                    {
                        std::string tempName = expressions[i] + "_" + boost::lexical_cast<std::string>(n);
                        bool found = false;

                        for (size_t k = 0; k < i; ++k)
                        {
                            if (tempName == columns[k].getName())
                            {
                                found = true;
                                break;
                            }
                        }

                        if (!found)
                        {
                            for (size_t k = i + 1; k < fieldCount; ++k)
                            {
                                if (tempName == expressions[k])
                                {
                                    found = true;
                                    break;
                                }
                            }
                        }

                        if (!found)
                        {
                            expressions[i] = tempName;
                            break;
                        }
                    }

                    break;
                }
            }

            const std::pair<Type::Column::ColumnType, bool>& columnType = columnTypes[i];

            if (columnType.second)
            {
                if (nullable.empty())
                {
                    nullable.push_back("nullable");
                }

                columns.push_back(Type::Column(expressions[i], columnType.first, nullable));
            }
            else
            {
                columns.push_back(Type::Column(expressions[i], columnType.first));
            }
        }

        Type type(columns);

        for (size_t i = 0; i < result.size(); ++i) {
            result[i].m_type = type;
        }
    }

    GenericRecord::GenericRecord(const std::vector<std::pair<Type::Column::ColumnType, bool> >& columnTypes)
    {
        size_t columnCount = columnTypes.size();
        m_values.reserve(columnCount);

        for (size_t i = 0; i < columnCount; ++i)
        {
            const std::pair<Type::Column::ColumnType, bool>& columnType = columnTypes[i];
            initializeColumn(columnType.first, columnType.second);
        }
    }

    #define INIT_COLUMN(T) { \
        if (isNullable) \
        { \
            m_values.push_back(boost::optional<T>()); \
        } \
        else \
        { \
            m_values.push_back(T()); \
        } \
        \
        break; \
    }

    void GenericRecord::initializeColumn(const Type::Column::ColumnType type, const bool isNullable)
    {
        switch (type)
        {
            case Type::Column::BYTES: INIT_COLUMN(std::vector<uint8_t>)
            case Type::Column::DOUBLE: INIT_COLUMN(double)
            case Type::Column::FLOAT: INIT_COLUMN(float)
            case Type::Column::INT: INIT_COLUMN(int32_t)
            case Type::Column::LONG: INIT_COLUMN(int64_t)
            case Type::Column::STRING: INIT_COLUMN(std::string)
        }
    }
}

namespace avro
{
    void codec_traits<gpudb::GenericRecord>::encode(Encoder& e, const gpudb::GenericRecord& v)
    {
        const gpudb::Type& type = v.getType();
        size_t columnCount = type.getColumnCount();

        for (size_t i = 0; i < columnCount; ++i)
        {
            const gpudb::Type::Column& column = type.getColumn(i);
            encodeValue(e, column.getType(), column.isNullable(), v.m_values[i]);
        }
    }

    void codec_traits<gpudb::GenericRecord>::decode(Decoder& d, gpudb::GenericRecord& v)
    {
        const gpudb::Type& type = v.getType();
        size_t columnCount = type.getColumnCount();

        if (::avro::ResolvingDecoder *rd = dynamic_cast< ::avro::ResolvingDecoder*>(&d))
        {
            const std::vector<size_t>& fo = rd->fieldOrder();

            for (std::vector<size_t>::const_iterator it = fo.begin(); it != fo.end(); ++it)
            {
                size_t field = *it;

                if (field < columnCount)
                {
                    const gpudb::Type::Column& column = type.getColumn(field);
                    decodeValue(d, column.getType(), column.isNullable(), v.m_values[field]);
                }
            }
        }
        else
        {
            for (size_t i = 0; i < columnCount; ++i)
            {
                const gpudb::Type::Column& column = type.getColumn(i);
                decodeValue(d, column.getType(), column.isNullable(), v.m_values[i]);
            }
        }
    }
}
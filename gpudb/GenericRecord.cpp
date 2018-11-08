#include "gpudb/GenericRecord.hpp"

#include "gpudb/Avro.hpp"
#include "gpudb/GPUdbException.hpp"
#include "gpudb/Type.hpp"

#include <iomanip>

#include <avro/Compiler.hh>
#include <boost/lexical_cast.hpp>

namespace
{
    void toString(const std::vector<uint8_t>& value, std::string& result)
    {
        std::ostringstream oss;
        oss << std::hex << std::setfill('0');

        for (size_t i = 0; i < value.size(); ++i)
        {
            oss << std::setw(2) << (unsigned)value[i];
        }

        result = oss.str();
    }
}

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

    #define DEFINE_VALUE_REF(T, TN) \
        T& GenericRecord::TN##Value(const size_t index) \
        { \
            return value<T>(index); \
        } \
        \
        const T& GenericRecord::TN##Value(const size_t index) const \
        { \
            return value<T>(index); \
        } \
        \
        T& GenericRecord::TN##Value(const std::string& name) \
        { \
            return value<T>(name); \
        } \
        \
        const T& GenericRecord::TN##Value(const std::string& name) const \
        { \
            return value<T>(name); \
        }

    DEFINE_VALUE_REF(std::vector<uint8_t>, bytes)
    DEFINE_VALUE_REF(double, double)
    DEFINE_VALUE_REF(float, float)
    DEFINE_VALUE_REF(int32_t, int)
    DEFINE_VALUE_REF(int64_t, long)
    DEFINE_VALUE_REF(std::string, string)
    DEFINE_VALUE_REF(boost::optional<std::vector<uint8_t> >, nullableBytes)
    DEFINE_VALUE_REF(boost::optional<double>, nullableDouble)
    DEFINE_VALUE_REF(boost::optional<float>, nullableFloat)
    DEFINE_VALUE_REF(boost::optional<int32_t>, nullableInt)
    DEFINE_VALUE_REF(boost::optional<int64_t>, nullableLong)
    DEFINE_VALUE_REF(boost::optional<std::string>, nullableString)

    bool GenericRecord::isNull(const size_t index) const
    {
        const Type::Column& column = m_type.getColumn(index);

        if (!column.isNullable())
        {
            return false;
        }

        switch (column.getType())
        {
            case Type::Column::BYTES: return !value<boost::optional<std::vector<uint8_t> > >(index);
            case Type::Column::DOUBLE: return !value<boost::optional<double> >(index);
            case Type::Column::FLOAT: return !value<boost::optional<float> >(index);
            case Type::Column::INT: return !value<boost::optional<int32_t> >(index);
            case Type::Column::LONG: return !value<boost::optional<int64_t> >(index);
            case Type::Column::STRING: return !value<boost::optional<std::string> >(index);
            default: throw std::bad_cast();
        }
    }

    bool GenericRecord::isNull(const std::string& name) const
    {
        return isNull(m_type.getColumnIndex(name));
    }

    void GenericRecord::getAsBytes(const size_t index, std::vector<uint8_t>& result) const
    {
        const Type::Column& column = m_type.getColumn(index);

        switch (column.getType())
        {
            case Type::Column::BYTES:
                result = column.isNullable()
                    ? *value<boost::optional<std::vector<uint8_t> > >(index)
                    : value<std::vector<uint8_t> >(index);
                break;

            default:
                throw std::bad_cast();
        }
    }

    void GenericRecord::getAsBytes(const std::string& name, std::vector<uint8_t>& result) const
    {
        getAsBytes(m_type.getColumnIndex(name), result);
    }

    std::vector<uint8_t> GenericRecord::getAsBytes(const size_t index) const
    {
        std::vector<uint8_t> result;
        getAsBytes(index, result);
        return result;
    }

    std::vector<uint8_t> GenericRecord::getAsBytes(const std::string& name) const
    {
        std::vector<uint8_t> result;
        getAsBytes(name, result);
        return result;
    }

    #define CASE_CAST_TO_NON_NULLABLE(CT, CTN, T, CF) \
        case Type::Column::CTN: \
            if (column.isNullable()) \
            { \
                const boost::optional<CT>& temp = value<boost::optional<CT> >(index); \
                \
                if (temp) \
                { \
                    result = CF<T>(*temp); \
                } \
                else \
                { \
                    throw std::bad_cast(); \
                } \
            } \
            else \
            { \
                result = CF<T>(value<CT>(index)); \
            } \
            \
            break;

    #define DEFINE_NON_NULLABLE_ACCESSORS(T, TN, CF, SCF) \
        void GenericRecord::getAs##TN(const size_t index, T& result) const \
        { \
            const Type::Column& column = m_type.getColumn(index); \
            \
            switch (column.getType()) \
            { \
                CASE_CAST_TO_NON_NULLABLE(double, DOUBLE, T, CF) \
                CASE_CAST_TO_NON_NULLABLE(float, FLOAT, T, CF) \
                CASE_CAST_TO_NON_NULLABLE(int32_t, INT, T, CF) \
                CASE_CAST_TO_NON_NULLABLE(int64_t, LONG, T, CF) \
                CASE_CAST_TO_NON_NULLABLE(std::string, STRING, T, SCF) \
                default: throw std::bad_cast(); \
            } \
        } \
        \
        void GenericRecord::getAs##TN(const std::string& name, T& result) const \
        { \
            getAs##TN(m_type.getColumnIndex(name), result); \
        } \
        \
        T GenericRecord::getAs##TN(const size_t index) const \
        { \
            T result; \
            getAs##TN(index, result); \
            return result; \
        } \
        \
        T GenericRecord::getAs##TN(const std::string& name) const \
        { \
            T result; \
            getAs##TN(name, result); \
            return result; \
        }

    DEFINE_NON_NULLABLE_ACCESSORS(double, Double, static_cast, boost::lexical_cast)
    DEFINE_NON_NULLABLE_ACCESSORS(float, Float, static_cast, boost::lexical_cast)
    DEFINE_NON_NULLABLE_ACCESSORS(int32_t, Int, static_cast, boost::lexical_cast)
    DEFINE_NON_NULLABLE_ACCESSORS(int64_t, Long, static_cast, boost::lexical_cast)
    DEFINE_NON_NULLABLE_ACCESSORS(std::string, String, boost::lexical_cast, static_cast)

    void GenericRecord::getAsNullableBytes(const size_t index, boost::optional<std::vector<uint8_t> >& result) const
    {
        const Type::Column& column = m_type.getColumn(index);

        switch (column.getType())
        {
            case Type::Column::BYTES:
                if (column.isNullable())
                {
                    result = value<boost::optional<std::vector<uint8_t> > >(index);
                }
                else
                {
                    result = value<std::vector<uint8_t> >(index);
                }

                break;

            default:
                throw std::bad_cast();
        }
    }

    void GenericRecord::getAsNullableBytes(const std::string& name, boost::optional<std::vector<uint8_t> >& result) const
    {
        getAsNullableBytes(m_type.getColumnIndex(name), result);
    }

    boost::optional<std::vector<uint8_t> > GenericRecord::getAsNullableBytes(const size_t index) const
    {
        boost::optional<std::vector<uint8_t> > result;
        getAsNullableBytes(index, result);
        return result;
    }

    boost::optional<std::vector<uint8_t> > GenericRecord::getAsNullableBytes(const std::string& name) const
    {
        boost::optional<std::vector<uint8_t> > result;
        getAsNullableBytes(name, result);
        return result;
    }

    #define CASE_CAST_TO_NULLABLE(CT, CTN, T, CF) \
        case Type::Column::CTN: \
        { \
            if (column.isNullable()) \
            { \
                const boost::optional<CT>& temp = value<boost::optional<CT> >(index); \
                \
                if (temp) \
                { \
                    result = CF<T>(*temp); \
                } \
                else \
                { \
                    result = boost::none; \
                } \
            } \
            else \
            { \
                result = CF<T>(value<CT>(index)); \
            } \
            \
            break; \
        }

    #define DEFINE_NULLABLE_ACCESSORS(T, TN, CF, SCF) \
        void GenericRecord::getAsNullable##TN(const size_t index, boost::optional<T>& result) const \
        { \
            const Type::Column& column = m_type.getColumn(index); \
            \
            switch (column.getType()) \
            { \
                CASE_CAST_TO_NULLABLE(double, DOUBLE, T, CF) \
                CASE_CAST_TO_NULLABLE(float, FLOAT, T, CF) \
                CASE_CAST_TO_NULLABLE(int32_t, INT, T, CF) \
                CASE_CAST_TO_NULLABLE(int64_t, LONG, T, CF) \
                CASE_CAST_TO_NULLABLE(std::string, STRING, T, SCF) \
                default: throw std::bad_cast(); \
            } \
        } \
        \
        void GenericRecord::getAsNullable##TN(const std::string& name, boost::optional<T>& result) const \
        { \
            getAsNullable##TN(m_type.getColumnIndex(name), result); \
        } \
        \
        boost::optional<T> GenericRecord::getAsNullable##TN(const size_t index) const \
        { \
            boost::optional<T> result; \
            getAsNullable##TN(index, result); \
            return result; \
        } \
        \
        boost::optional<T> GenericRecord::getAsNullable##TN(const std::string& name) const \
        { \
            boost::optional<T> result; \
            getAsNullable##TN(name, result); \
            return result; \
        }

    DEFINE_NULLABLE_ACCESSORS(double, Double, static_cast, boost::lexical_cast)
    DEFINE_NULLABLE_ACCESSORS(float, Float, static_cast, boost::lexical_cast)
    DEFINE_NULLABLE_ACCESSORS(int32_t, Int, static_cast, boost::lexical_cast)
    DEFINE_NULLABLE_ACCESSORS(int64_t, Long, static_cast, boost::lexical_cast)
    DEFINE_NULLABLE_ACCESSORS(std::string, String, boost::lexical_cast, static_cast)

    void GenericRecord::setNull(const size_t index)
    {
        const Type::Column& column = m_type.getColumn(index);

        if (!column.isNullable())
        {
            throw std::bad_cast();
        }

        switch (column.getType())
        {
            case Type::Column::BYTES: value<boost::optional<std::vector<uint8_t> > >(index) = boost::none; break;
            case Type::Column::DOUBLE: value<boost::optional<double> >(index) = boost::none; break;
            case Type::Column::FLOAT: value<boost::optional<float> >(index) = boost::none; break;
            case Type::Column::INT: value<boost::optional<int32_t> >(index) = boost::none; break;
            case Type::Column::LONG: value<boost::optional<int64_t> >(index) = boost::none; break;
            case Type::Column::STRING: value<boost::optional<std::string> >(index) = boost::none; break;
        }
    }

    void GenericRecord::setNull(const std::string& name)
    {
        setNull(m_type.getColumnIndex(name));
    }

    void GenericRecord::setAsBytes(const size_t index, const std::vector<uint8_t>& newValue)
    {
        const Type::Column& column = m_type.getColumn(index);

        switch (column.getType())
        {
            case Type::Column::BYTES:
                if (column.isNullable())
                {
                    value<boost::optional<std::vector<uint8_t> > >(index) = newValue;
                }
                else
                {
                    value<std::vector<uint8_t> >(index) = newValue;
                }

                break;

            default:
                throw std::bad_cast();
        }
    }

    void GenericRecord::setAsBytes(const std::string& name, const std::vector<uint8_t>& newValue)
    {
        setAsBytes(m_type.getColumnIndex(name), newValue);
    }

    #define CASE_SET_NON_NULLABLE(CT, CTN, CF) \
        case Type::Column::CTN: \
            if (column.isNullable()) \
            { \
                value<boost::optional<CT> >(index) = CF<CT>(newValue); \
            } \
            else \
            { \
                value<CT>(index) = CF<CT>(newValue); \
            } \
            \
            break;

    #define DEFINE_NON_NULLABLE_MUTATORS(T, TN, CF, SCF) \
        void GenericRecord::setAs##TN(const size_t index, const T& newValue) \
        { \
            const Type::Column& column = m_type.getColumn(index); \
            \
            switch (column.getType()) \
            { \
                CASE_SET_NON_NULLABLE(double, DOUBLE, CF) \
                CASE_SET_NON_NULLABLE(float, FLOAT, CF) \
                CASE_SET_NON_NULLABLE(int32_t, INT, CF) \
                CASE_SET_NON_NULLABLE(int64_t, LONG, CF) \
                CASE_SET_NON_NULLABLE(std::string, STRING, SCF) \
                default: throw std::bad_cast(); \
            } \
        } \
        \
        void GenericRecord::setAs##TN(const std::string& name, const T& newValue) \
        { \
            setAs##TN(m_type.getColumnIndex(name), newValue); \
        }

    DEFINE_NON_NULLABLE_MUTATORS(double, Double, static_cast, boost::lexical_cast)
    DEFINE_NON_NULLABLE_MUTATORS(float, Float, static_cast, boost::lexical_cast)
    DEFINE_NON_NULLABLE_MUTATORS(int32_t, Int, static_cast, boost::lexical_cast)
    DEFINE_NON_NULLABLE_MUTATORS(int64_t, Long, static_cast, boost::lexical_cast)
    DEFINE_NON_NULLABLE_MUTATORS(std::string, String, boost::lexical_cast, static_cast)

    void GenericRecord::setAsNullableBytes(const size_t index, const boost::optional<std::vector<uint8_t> >& newValue)
    {
        const Type::Column& column = m_type.getColumn(index);

        switch (column.getType())
        {
            case Type::Column::BYTES:
                if (column.isNullable())
                {
                    value<boost::optional<std::vector<uint8_t> > >(index) = newValue;
                }
                else
                {
                    if (newValue)
                    {
                        value<std::vector<uint8_t> >(index) = *newValue;
                    }
                    else
                    {
                        throw std::bad_cast();
                    }
                }

            default:
                throw std::bad_cast();
        }
    }

    #define CASE_SET_NULLABLE(CT, CTN, CF) \
        case Type::Column::CTN: \
            if (column.isNullable()) \
            { \
                if (newValue) \
                { \
                    value<boost::optional<CT> >(index) = CF<CT>(*newValue); \
                } \
                else \
                { \
                    value<boost::optional<CT> >(index) = boost::none; \
                } \
            } \
            else \
            { \
                if (newValue) \
                { \
                    value<CT>(index) = CF<CT>(*newValue); \
                } \
                else \
                { \
                    throw std::bad_cast(); \
                } \
            } \
            \
            break;

    #define DEFINE_NULLABLE_MUTATORS(T, TN, CF, SCF) \
        void GenericRecord::setAsNullable##TN(const size_t index, const boost::optional<T>& newValue) \
        { \
            const Type::Column& column = m_type.getColumn(index); \
            \
            switch (column.getType()) \
            { \
                CASE_SET_NULLABLE(double, DOUBLE, CF) \
                CASE_SET_NULLABLE(float, FLOAT, CF) \
                CASE_SET_NULLABLE(int32_t, INT, CF) \
                CASE_SET_NULLABLE(int64_t, LONG, CF) \
                CASE_SET_NULLABLE(std::string, STRING, SCF) \
                default: throw std::bad_cast(); \
            } \
        } \
        \
        void GenericRecord::setAsNullable##TN(const std::string& name, const boost::optional<T>& newValue) \
        { \
            setAsNullable##TN(m_type.getColumnIndex(name), newValue); \
        }

    DEFINE_NULLABLE_MUTATORS(double, Double, static_cast, boost::lexical_cast)
    DEFINE_NULLABLE_MUTATORS(float, Float, static_cast, boost::lexical_cast)
    DEFINE_NULLABLE_MUTATORS(int32_t, Int, static_cast, boost::lexical_cast)
    DEFINE_NULLABLE_MUTATORS(int64_t, Long, static_cast, boost::lexical_cast)
    DEFINE_NULLABLE_MUTATORS(std::string, String, boost::lexical_cast, static_cast)

    #define CASE_TO_STRING(CT, CTN, SCF) \
        case Type::Column::CTN: \
        { \
            if (column.isNullable()) \
            { \
                const boost::optional<CT>& temp = value<boost::optional<CT> >(index); \
                \
                if (temp) \
                { \
                    result = SCF<std::string>(*temp); \
                } \
                else \
                { \
                    result = ""; \
                } \
            } \
            else \
            { \
                result = SCF<std::string>(value<CT>(index)); \
            } \
            \
            break; \
        }

    void GenericRecord::toString(const size_t index, std::string& result) const
    {
        const Type::Column& column = m_type.getColumn(index);

        switch (column.getType())
        {
            case Type::Column::BYTES:
            {
                if (column.isNullable())
                {
                    const boost::optional<std::vector<uint8_t> >& temp = value<boost::optional<std::vector<uint8_t> > >(index);

                    if (temp)
                    {
                        ::toString(*temp, result);
                    }
                    else
                    {
                        result = "";
                    }
                }
                else
                {
                    ::toString(value<std::vector<uint8_t> >(index), result);
                }

                break;
            }

            CASE_TO_STRING(double, DOUBLE, boost::lexical_cast)
            CASE_TO_STRING(float, FLOAT, boost::lexical_cast)
            CASE_TO_STRING(int32_t, INT, boost::lexical_cast)
            CASE_TO_STRING(int64_t, LONG, boost::lexical_cast)
            CASE_TO_STRING(std::string, STRING, static_cast)
        }
    }

    void GenericRecord::toString(const std::string& name, std::string& result) const
    {
        toString(m_type.getColumnIndex(name), result);
    }

    std::string GenericRecord::toString(const size_t index) const
    {
        std::string result;
        toString(index, result);
        return result;
    }

    std::string GenericRecord::toString(const std::string& name) const
    {
        std::string result;
        toString(name, result);
        return result;
    }

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

    void GenericRecord::transpose( const std::string& schemaString,
                                   const std::vector<uint8_t>& encodedData,
                                   std::vector<GenericRecord>& data,
                                   gpudb_type_ptr_t &dataTypePtr )
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

        if ( root->leaves() < 3 )
        {
            throw GPUdbException("Schema must have at least three fields.");
        }
        size_t fieldCount = root->leaves() - 2;

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
        std::vector<std::string> types;
        types.reserve(fieldCount);

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
                        data.resize(recordCount, GenericRecord(columnTypes));
                    }

                    for (size_t k = 0; k < j; ++k)
                    {
                        if (recordNumber < recordCount)
                        {
                            decodeValue(*decoder, columnType.first, columnType.second, data[recordNumber].m_values[i]);
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

            ::avro::decode(*decoder, types);

            if (expressions.size() < fieldCount)
            {
                throw GPUdbException("Every field must have a corresponding type.");
            }
        }
        catch (const std::exception& ex)
        {
            throw GPUdbException("Could not decode data: " + std::string(ex.what()));
        }

        std::vector<Type::Column> columns;
        columns.reserve(fieldCount);

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
            const std::string& type = types[i];
            std::vector<std::string> columnProperties;

            if (columnType.second)
            {
                columnProperties.push_back(ColumnProperty::NULLABLE);
            }

            if (type == ColumnProperty::CHAR1
                || type == ColumnProperty::CHAR2
                || type == ColumnProperty::CHAR4
                || type == ColumnProperty::CHAR8
                || type == ColumnProperty::CHAR16
                || type == ColumnProperty::CHAR32
                || type == ColumnProperty::CHAR64
                || type == ColumnProperty::CHAR128
                || type == ColumnProperty::CHAR256
                || type == ColumnProperty::DATE
                || type == ColumnProperty::DECIMAL
                || type == ColumnProperty::INT8
                || type == ColumnProperty::INT16
                || type == ColumnProperty::IPV4
                || type == ColumnProperty::TIME
                || type == ColumnProperty::TIMESTAMP)
            {
                columnProperties.push_back(type);
            }

            columns.push_back(Type::Column(expressions[i], columnType.first, columnProperties));
        }

        dataTypePtr = gpudb_type_ptr_t( new Type( columns ) );

        for (size_t i = 0; i < data.size(); ++i) {
            data[i].m_type = *dataTypePtr;
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


    std::ostream &operator << (std::ostream  &os, GenericRecord &record)
    {
        std::vector<Type::Column> columns = record.getType().getColumns();

        std::vector<Type::Column>::const_iterator it;
        size_t i = 0;
        for ( it = columns.begin(); it != columns.end(); ++it )
        {

            os << " " << it->getName() << ": ";

            // Handle nulls
            if ( record.isNull( i ) )
            {
                os << "<NULL>";
            }
            else // not a nullable, or if so, not a null
            {
                switch ( it->getType() )
                {
                    case Type::Column::INT:
                    {
                        os << record.getAsInt( i ); break;
                    }
                    case Type::Column::LONG:
                    {
                        os << record.getAsLong( i ); break;
                    }
                    case Type::Column::FLOAT:
                    {
                        os << record.getAsFloat( i ); break;
                    }
                    case Type::Column::DOUBLE:
                    {
                        os << record.getAsDouble( i ); break;
                    }
                    case Type::Column::STRING:
                    {
                        os << record.getAsString( i ); break;
                    }
                    case Type::Column::BYTES:
                    {
                        std::vector<unsigned char> bytes = record.getAsBytes( i );
                        std::vector<unsigned char>::const_iterator it2;
                        for ( it2 = bytes.begin(); it2 != bytes.end(); ++it2 )
                            os << *it2;
                        break;
                    }
                }  // end switch
            }  // end else
            ++i;
        }

        return os;
    } // << overload


    std::ostream &operator << (std::ostream  &os, const GenericRecord &record)
    {
        std::vector<Type::Column> columns = record.getType().getColumns();

        std::vector<Type::Column>::const_iterator it;
        size_t i = 0;
        for ( it = columns.begin(); it != columns.end(); ++it )
        {

            os << " " << it->getName() << ": ";

            // Handle nulls
            if ( record.isNull( i ) )
            {
                os << "<NULL>";
            }
            else // not a nullable, or if so, not a null
            {
                switch ( it->getType() )
                {
                    case Type::Column::INT:
                    {
                        os << record.getAsInt( i ); break;
                    }
                    case Type::Column::LONG:
                    {
                        os << record.getAsLong( i ); break;
                    }
                    case Type::Column::FLOAT:
                    {
                        os << record.getAsFloat( i ); break;
                    }
                    case Type::Column::DOUBLE:
                    {
                        os << record.getAsDouble( i ); break;
                    }
                    case Type::Column::STRING:
                    {
                        os << record.getAsString( i ); break;
                    }
                    case Type::Column::BYTES:
                    {
                        std::vector<unsigned char> bytes = record.getAsBytes( i );
                        std::vector<unsigned char>::const_iterator it2;
                        for ( it2 = bytes.begin(); it2 != bytes.end(); ++it2 )
                            os << *it2;
                        break;
                    }
                }  // end switch
            }  // end else
            ++i;
        }

        return os;
    }  // << overload
    


} // namespace gpudb


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

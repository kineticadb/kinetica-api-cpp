#ifndef __GPUDB__GENERICRECORD_HPP__
#define __GPUDB__GENERICRECORD_HPP__

#include "gpudb/Type.hpp"

#include <avro/Schema.hh>
#include <avro/Specific.hh>

namespace gpudb
{
    class GenericRecord
    {
        friend class GPUdb;

        friend struct ::avro::codec_traits<GenericRecord>;

        public:
            GenericRecord(const Type& type);
            const Type& getType() const;
            const ::avro::ValidSchema& getSchema() const;

            template<typename T> T& value(const size_t index)
            {
                return *boost::any_cast<T>(&m_values[index]);
            }

            template<typename T> const T& value(const size_t index) const
            {
                return *boost::any_cast<T>(&m_values[index]);
            }

            template<typename T> T& value(const std::string& name)
            {
                return *boost::any_cast<T>(&m_values[m_type.getColumnIndex(name)]);
            }

            template<typename T> const T& value(const std::string& name) const
            {
                return *boost::any_cast<T>(&m_values[m_type.getColumnIndex(name)]);
            }

            std::vector<uint8_t>& asBytes(const size_t index);
            const std::vector<uint8_t>& asBytes(const size_t index) const;
            std::vector<uint8_t>& asBytes(const std::string& name);
            const std::vector<uint8_t>& asBytes(const std::string& name) const;

            boost::optional<std::vector<uint8_t> >& asNullableBytes(const size_t index);
            const boost::optional<std::vector<uint8_t> >& asNullableBytes(const size_t index) const;
            boost::optional<std::vector<uint8_t> >& asNullableBytes(const std::string& name);
            const boost::optional<std::vector<uint8_t> >& asNullableBytes(const std::string& name) const;

            double& asDouble(const size_t index);
            const double& asDouble(const size_t index) const;
            double& asDouble(const std::string& name);
            const double& asDouble(const std::string& name) const;

            boost::optional<double>& asNullableDouble(const size_t index);
            const boost::optional<double>& asNullableDouble(const size_t index) const;
            boost::optional<double>& asNullableDouble(const std::string& name);
            const boost::optional<double>& asNullableDouble(const std::string& name) const;

            float& asFloat(const size_t index);
            const float& asFloat(const size_t index) const;
            float& asFloat(const std::string& name);
            const float& asFloat(const std::string& name) const;

            boost::optional<float>& asNullableFloat(const size_t index);
            const boost::optional<float>& asNullableFloat(const size_t index) const;
            boost::optional<float>& asNullableFloat(const std::string& name);
            const boost::optional<float>& asNullableFloat(const std::string& name) const;

            int32_t& asInt(const size_t index);
            const int32_t& asInt(const size_t index) const;
            int32_t& asInt(const std::string& name);
            const int32_t& asInt(const std::string& name) const;

            boost::optional<int32_t>& asNullableInt(const size_t index);
            const boost::optional<int32_t>& asNullableInt(const size_t index) const;
            boost::optional<int32_t>& asNullableInt(const std::string& name);
            const boost::optional<int32_t>& asNullableInt(const std::string& name) const;

            int64_t& asLong(const size_t index);
            const int64_t& asLong(const size_t index) const;
            int64_t& asLong(const std::string& name);
            const int64_t& asLong(const std::string& name) const;

            boost::optional<int64_t>& asNullableLong(const size_t index);
            const boost::optional<int64_t>& asNullableLong(const size_t index) const;
            boost::optional<int64_t>& asNullableLong(const std::string& name);
            const boost::optional<int64_t>& asNullableLong(const std::string& name) const;

            std::string& asString(const size_t index);
            const std::string& asString(const size_t index) const;
            std::string& asString(const std::string& name);
            const std::string& asString(const std::string& name) const;

            boost::optional<std::string>& asNullableString(const size_t index);
            const boost::optional<std::string>& asNullableString(const size_t index) const;
            boost::optional<std::string>& asNullableString(const std::string& name);
            const boost::optional<std::string>& asNullableString(const std::string& name) const;

        private:
            static void transpose(const std::string& schemaString, const std::vector<uint8_t>& encodedData, std::vector<GenericRecord>& data);

            Type m_type;
            std::vector<boost::any> m_values;

            GenericRecord(const std::vector<std::pair<Type::Column::ColumnType, bool> >& columnTypes);
            void initializeColumn(const Type::Column::ColumnType type, const bool isNullable);
    };

    typedef GenericRecord DynamicTableRecord;
}

namespace avro
{
    template<> struct codec_traits<gpudb::GenericRecord>
    {
        static void encode(Encoder& e, const gpudb::GenericRecord& v);
        static void decode(Decoder& d, gpudb::GenericRecord& v);
    };
}

#endif
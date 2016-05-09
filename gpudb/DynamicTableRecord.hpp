#ifndef __GPUDB__DYNAMICTABLERECORD_HPP__
#define __GPUDB__DYNAMICTABLERECORD_HPP__

#include <avro/Generic.hh>
#include <avro/Schema.hh>

namespace gpudb
{
    class DynamicTableRecord
    {
        friend class GPUdb;

        public:
            const ::avro::ValidSchema& getSchema() const;
            const std::string& getExpression(const size_t index) const;
            const std::string& getExpression(const std::string& name) const;
            size_t getFieldCount() const;
            size_t getFieldIndex(const std::string& name) const;
            const std::string& getFieldName(const size_t index) const;
            ::avro::Type getFieldType(const size_t index) const;
            ::avro::Type getFieldType(const std::string& name) const;

            template<typename T> T& value(const size_t index)
            {
                return data->value< ::avro::GenericRecord>().fieldAt(index).value< ::avro::GenericArray>().value()[recordIndex].value<T>();
            }

            template<typename T> const T& value(const size_t index) const
            {
                return data->value< ::avro::GenericRecord>().fieldAt(index).value< ::avro::GenericArray>().value()[recordIndex].value<T>();
            }

            template<typename T> T& value(const std::string& name)
            {
                return data->value< ::avro::GenericRecord>().fieldAt(getFieldIndex(name)).value< ::avro::GenericArray>().value()[recordIndex].value<T>();
            }

            template<typename T> const T& value(const std::string& name) const
            {
                return data->value< ::avro::GenericRecord>().fieldAt(getFieldIndex(name)).value< ::avro::GenericArray>().value()[recordIndex].value<T>();
            }

            std::vector<uint8_t>& asBytes(const size_t index);
            const std::vector<uint8_t>& asBytes(const size_t index) const;
            std::vector<uint8_t>& asBytes(const std::string& name);
            const std::vector<uint8_t>& asBytes(const std::string& name) const;
            double& asDouble(const size_t index);
            const double& asDouble(const size_t index) const;
            double& asDouble(const std::string& name);
            const double& asDouble(const std::string& name) const;
            float& asFloat(const size_t index);
            const float& asFloat(const size_t index) const;
            float& asFloat(const std::string& name);
            const float& asFloat(const std::string& name) const;
            int32_t& asInt(const size_t index);
            const int32_t& asInt(const size_t index) const;
            int32_t& asInt(const std::string& name);
            const int32_t& asInt(const std::string& name) const;
            int64_t& asLong(const size_t index);
            const int64_t& asLong(const size_t index) const;
            int64_t& asLong(const std::string& name);
            const int64_t& asLong(const std::string& name) const;
            std::string& asString(const size_t index);
            const std::string& asString(const size_t index) const;
            std::string& asString(const std::string& name);
            const std::string& asString(const std::string& name) const;

        private:
            static void transpose(const std::string& schemaString, const std::vector<uint8_t>& encodedData, std::vector<DynamicTableRecord>& data);

            boost::shared_ptr< ::avro::ValidSchema> schema;
            boost::shared_ptr< ::avro::GenericDatum> data;
            size_t recordIndex;

            DynamicTableRecord(const boost::shared_ptr< ::avro::ValidSchema>& schema, const boost::shared_ptr< ::avro::GenericDatum>& data, const size_t recordIndex);
    };
}

#endif

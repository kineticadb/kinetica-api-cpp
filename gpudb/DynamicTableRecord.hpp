#ifndef __GPUDB__DYNAMICTABLERECORD_HPP__
#define __GPUDB__DYNAMICTABLERECORD_HPP__

#include "gpudb/Type.hpp"

#include <avro/Generic.hh>
#include <avro/Schema.hh>

namespace gpudb
{
    class DynamicTableRecord
    {
        friend class GPUdb;

        public:
            const Type& getType() const;
            const ::avro::ValidSchema& getSchema() const;

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
                return data->value< ::avro::GenericRecord>().fieldAt(type->getColumnIndex(name)).value< ::avro::GenericArray>().value()[recordIndex].value<T>();
            }

            template<typename T> const T& value(const std::string& name) const
            {
                return data->value< ::avro::GenericRecord>().fieldAt(type->getColumnIndex(name)).value< ::avro::GenericArray>().value()[recordIndex].value<T>();
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

            boost::shared_ptr<Type> type;
            boost::shared_ptr< ::avro::GenericDatum> data;
            size_t recordIndex;

            DynamicTableRecord(const boost::shared_ptr<Type>& type, const boost::shared_ptr< ::avro::GenericDatum>& data, const size_t recordIndex);
    };
}

#endif
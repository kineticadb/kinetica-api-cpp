#ifndef __GPUDB__GENERICRECORD_HPP__
#define __GPUDB__GENERICRECORD_HPP__

#include <avro/Schema.hh>
#include <avro/Specific.hh>

namespace gpudb
{
    class GenericRecord
    {
        public:
            GenericRecord(const ::avro::ValidSchema& schema);
            const ::avro::ValidSchema& getSchema() const;
            size_t getFieldCount() const;
            size_t getFieldIndex(const std::string& name) const;
            const std::string& getFieldName(const size_t index) const;
            ::avro::Type getFieldType(const size_t index) const;
            ::avro::Type getFieldType(const std::string& name) const;

            template<typename T> T& value(const size_t index)
            {
                return boost::any_cast<T>(values[index]);
            }

            template<typename T> const T& value(const size_t index) const
            {
                return boost::any_cast<T>(values[index]);
            }

            template<typename T> T& value(const std::string& name)
            {
                size_t index = 0;

                if (!schema.root()->nameIndex(name, index))
                {
                    throw ::avro::Exception("Invalid field name: " + name);
                }

                return boost::any_cast<T>(values[index]);
            }

            template<typename T> const T& value(const std::string& name) const
            {
                size_t index = 0;

                if (!schema.root()->nameIndex(name, index))
                {
                    throw ::avro::Exception("Invalid field name: " + name);
                }

                return boost::any_cast<T>(values[index]);
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
            ::avro::ValidSchema schema;
            std::vector<boost::any> values;
    };
}

namespace avro
{
    template<> struct codec_traits<gpudb::GenericRecord>
    {
        static void encode(Encoder& e, const gpudb::GenericRecord& g);
        static void decode(Decoder& d, gpudb::GenericRecord& g);
    };
}

#endif

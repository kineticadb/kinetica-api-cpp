#ifndef __GPUDB__GENERICRECORD_HPP__
#define __GPUDB__GENERICRECORD_HPP__

#include "gpudb/Type.hpp"

#include <avro/Schema.hh>
#include <avro/Specific.hh>

#include <boost/lexical_cast.hpp>
#if BOOST_VERSION >= 105600
    #include <boost/core/demangle.hpp>
#else
    #include <boost/units/detail/utility.hpp> // Old versions have demangle() here.
    namespace boost { namespace core { using boost::units::detail::demangle; } }
#endif

namespace gpudb
{
    typedef boost::shared_ptr<Type> gpudb_type_ptr_t;

    template<typename T> std::string JsonStringFromVector(const std::vector<T>& vector); 

    class GenericRecord
    {
        friend class GPUdb;

        friend struct ::avro::codec_traits<GenericRecord>;

        public:
            /// Create a blank GenericRecord object from a given type
            GenericRecord(const Type& type);

            const Type& getType() const;
            const ::avro::ValidSchema& getSchema() const;

            template<typename T> T& value(const size_t index)
            {
                T* o = boost::any_cast<T>(&m_values.at(index));
                if (!o) throw std::runtime_error("Error converting GenericRecord column " + boost::lexical_cast<std::string>(index) + " to type " + boost::core::demangle(typeid(T).name() ));
                return *o;
            }

            template<typename T> const T& value(const size_t index) const
            {
                const T* o = boost::any_cast<T>(&m_values.at(index));
                if (!o) throw std::runtime_error("Error converting GenericRecord column " + boost::lexical_cast<std::string>(index) + " to type " + boost::core::demangle(typeid(T).name() ));
                return *o;
            }

            template<typename T> T& value(const std::string& name)
            {
                T* o = boost::any_cast<T>(&m_values[m_type.getColumnIndex(name)]);
                if (!o) throw std::runtime_error("Error converting GenericRecord column '" + name + "' to type " + boost::core::demangle(typeid(T).name() ));
                return *o;
            }

            template<typename T> const T& value(const std::string& name) const
            {
                const T* o = boost::any_cast<T>(&m_values[m_type.getColumnIndex(name)]);
                if (!o) throw std::runtime_error("Error converting GenericRecord column '" + name + "' to type " + boost::core::demangle(typeid(T).name() ));
                return *o;
            }

            std::vector<uint8_t>& bytesValue(const size_t index);
            const std::vector<uint8_t>& bytesValue(const size_t index) const;
            std::vector<uint8_t>& bytesValue(const std::string& name);
            const std::vector<uint8_t>& bytesValue(const std::string& name) const;

            double& doubleValue(const size_t index);
            const double& doubleValue(const size_t index) const;
            double& doubleValue(const std::string& name);
            const double& doubleValue(const std::string& name) const;

            float& floatValue(const size_t index);
            const float& floatValue(const size_t index) const;
            float& floatValue(const std::string& name);
            const float& floatValue(const std::string& name) const;

            int32_t& intValue(const size_t index);
            const int32_t& intValue(const size_t index) const;
            int32_t& intValue(const std::string& name);
            const int32_t& intValue(const std::string& name) const;

            int64_t& longValue(const size_t index);
            const int64_t& longValue(const size_t index) const;
            int64_t& longValue(const std::string& name);
            const int64_t& longValue(const std::string& name) const;

            std::string& stringValue(const size_t index);
            const std::string& stringValue(const size_t index) const;
            std::string& stringValue(const std::string& name);
            const std::string& stringValue(const std::string& name) const;

            boost::optional<std::vector<uint8_t> >& nullableBytesValue(const size_t index);
            const boost::optional<std::vector<uint8_t> >& nullableBytesValue(const size_t index) const;
            boost::optional<std::vector<uint8_t> >& nullableBytesValue(const std::string& name);
            const boost::optional<std::vector<uint8_t> >& nullableBytesValue(const std::string& name) const;

            boost::optional<double>& nullableDoubleValue(const size_t index);
            const boost::optional<double>& nullableDoubleValue(const size_t index) const;
            boost::optional<double>& nullableDoubleValue(const std::string& name);
            const boost::optional<double>& nullableDoubleValue(const std::string& name) const;

            boost::optional<float>& nullableFloatValue(const size_t index);
            const boost::optional<float>& nullableFloatValue(const size_t index) const;
            boost::optional<float>& nullableFloatValue(const std::string& name);
            const boost::optional<float>& nullableFloatValue(const std::string& name) const;

            boost::optional<int32_t>& nullableIntValue(const size_t index);
            const boost::optional<int32_t>& nullableIntValue(const size_t index) const;
            boost::optional<int32_t>& nullableIntValue(const std::string& name);
            const boost::optional<int32_t>& nullableIntValue(const std::string& name) const;

            boost::optional<int64_t>& nullableLongValue(const size_t index);
            const boost::optional<int64_t>& nullableLongValue(const size_t index) const;
            boost::optional<int64_t>& nullableLongValue(const std::string& name);
            const boost::optional<int64_t>& nullableLongValue(const std::string& name) const;

            boost::optional<std::string>& nullableStringValue(const size_t index);
            const boost::optional<std::string>& nullableStringValue(const size_t index) const;
            boost::optional<std::string>& nullableStringValue(const std::string& name);
            const boost::optional<std::string>& nullableStringValue(const std::string& name) const;

            bool isNull(const size_t index) const;
            bool isNull(const std::string& name) const;

            template<typename T> void getAsArray(const size_t index, std::vector<T>& result) const;
            template<typename T> void getAsArray(const std::string& name, std::vector<T>& result) const;
            template<typename T> std::vector<T> getAsArray(const size_t index) const;
            template<typename T> std::vector<T> getAsArray(const std::string& name) const;

            void getAsBytes(const size_t index, std::vector<uint8_t>& result) const;
            void getAsBytes(const std::string& name, std::vector<uint8_t>& result) const;
            std::vector<uint8_t> getAsBytes(const size_t index) const;
            std::vector<uint8_t> getAsBytes(const std::string& name) const;

            void getAsDouble(const size_t index, double& result) const;
            void getAsDouble(const std::string& name, double& result) const;
            double getAsDouble(const size_t index) const;
            double getAsDouble(const std::string& name) const;

            void getAsFloat(const size_t index, float& result) const;
            void getAsFloat(const std::string& name, float& result) const;
            float getAsFloat(const size_t index) const;
            float getAsFloat(const std::string& name) const;

            void getAsInt(const size_t index, int32_t& result) const;
            void getAsInt(const std::string& name, int32_t& result) const;
            int32_t getAsInt(const size_t index) const;
            int32_t getAsInt(const std::string& name) const;

            void getAsLong(const size_t index, int64_t& result) const;
            void getAsLong(const std::string& name, int64_t& result) const;
            int64_t getAsLong(const size_t index) const;
            int64_t getAsLong(const std::string& name) const;

            void getAsString(const size_t index, std::string& result) const;
            void getAsString(const std::string& name, std::string& result) const;
            std::string getAsString(const size_t index) const;
            std::string getAsString(const std::string& name) const;

            void getAsVector(const size_t index, std::vector<float>& result) const;
            void getAsVector(const std::string& name, std::vector<float>& result) const;
            std::vector<float> getAsVector(const size_t index) const;
            std::vector<float> getAsVector(const std::string& name) const;

            void getAsNullableBytes(const size_t index, boost::optional<std::vector<uint8_t> >& result) const;
            void getAsNullableBytes(const std::string& name, boost::optional<std::vector<uint8_t> >& result) const;
            boost::optional<std::vector<uint8_t> > getAsNullableBytes(const size_t index) const;
            boost::optional<std::vector<uint8_t> > getAsNullableBytes(const std::string& name) const;

            void getAsNullableDouble(const size_t index, boost::optional<double>& result) const;
            void getAsNullableDouble(const std::string& name, boost::optional<double>& result) const;
            boost::optional<double> getAsNullableDouble(const size_t index) const;
            boost::optional<double> getAsNullableDouble(const std::string& name) const;

            void getAsNullableFloat(const size_t index, boost::optional<float>& result) const;
            void getAsNullableFloat(const std::string& name, boost::optional<float>& result) const;
            boost::optional<float> getAsNullableFloat(const size_t index) const;
            boost::optional<float> getAsNullableFloat(const std::string& name) const;

            void getAsNullableInt(const size_t index, boost::optional<int32_t>& result) const;
            void getAsNullableInt(const std::string& name, boost::optional<int32_t>& result) const;
            boost::optional<int32_t> getAsNullableInt(const size_t index) const;
            boost::optional<int32_t> getAsNullableInt(const std::string& name) const;

            void getAsNullableLong(const size_t index, boost::optional<int64_t>& result) const;
            void getAsNullableLong(const std::string& name, boost::optional<int64_t>& result) const;
            boost::optional<int64_t> getAsNullableLong(const size_t index) const;
            boost::optional<int64_t> getAsNullableLong(const std::string& name) const;

            void getAsNullableString(const size_t index, boost::optional<std::string>& result) const;
            void getAsNullableString(const std::string& name, boost::optional<std::string>& result) const;
            boost::optional<std::string> getAsNullableString(const size_t index) const;
            boost::optional<std::string> getAsNullableString(const std::string& name) const;

            void setNull(const size_t index);
            void setNull(const std::string& name);

            template<typename T> void setAsArray(const size_t index, const std::vector<T>& newValue);
            template<typename T> void setAsArray(const std::string& name, const std::vector<T>& newValue);

            void setAsBytes(const size_t index, const std::vector<uint8_t>& newValue);
            void setAsBytes(const std::string& name, const std::vector<uint8_t>& newValue);

            void setAsDouble(const size_t index, const double& newValue);
            void setAsDouble(const std::string& name, const double& newValue);

            void setAsFloat(const size_t index, const float& newValue);
            void setAsFloat(const std::string& name, const float& newValue);

            void setAsInt(const size_t index, const int32_t& newValue);
            void setAsInt(const std::string& name, const int32_t& newValue);

            void setAsLong(const size_t index, const int64_t& newValue);
            void setAsLong(const std::string& name, const int64_t& newValue);

            void setAsString(const size_t index, const std::string& newValue);
            void setAsString(const std::string& name, const std::string& newValue);

            void setAsVector(const size_t index, const std::vector<float>& newValue);
            void setAsVector(const std::string& name, const std::vector<float>& newValue);

            void setAsNullableBytes(const size_t index, const boost::optional<std::vector<uint8_t> >& newValue);
            void setAsNullableBytes(const std::string& name, const boost::optional<std::vector<uint8_t> >& newValue);

            void setAsNullableDouble(const size_t index, const boost::optional<double>& newValue);
            void setAsNullableDouble(const std::string& name, const boost::optional<double>& newValue);

            void setAsNullableFloat(const size_t index, const boost::optional<float>& newValue);
            void setAsNullableFloat(const std::string& name, const boost::optional<float>& newValue);

            void setAsNullableInt(const size_t index, const boost::optional<int32_t>& newValue);
            void setAsNullableInt(const std::string& name, const boost::optional<int32_t>& newValue);

            void setAsNullableLong(const size_t index, const boost::optional<int64_t>& newValue);
            void setAsNullableLong(const std::string& name, const boost::optional<int64_t>& newValue);

            void setAsNullableString(const size_t index, const boost::optional<std::string>& newValue);
            void setAsNullableString(const std::string& name, const boost::optional<std::string>& newValue);

            void toString(const size_t index, std::string& result) const;
            void toString(const std::string& name, std::string& result) const;
            std::string toString(const size_t index) const;
            std::string toString(const std::string& name) const;

            friend std::ostream &operator << (std::ostream  &os, GenericRecord &gr);
            friend std::ostream &operator << (std::ostream  &os, const GenericRecord &gr);

            /**
             * Decodes avro encoded data (given the schema string) into GenericRecord objects.
             *
             * @param[in]  schemaString  Avro schema string used for decoding the data
             * @param[in]  encodedData   Avro encoded binary data containing multiple records.
             * @param[out] data          Vector of decoded GenericRecord objects.  The vector
             *                           is cleared before operation begins.
             */
            static void decode(const std::string& schemaString, const std::vector<std::vector<uint8_t> >& encodedData, std::vector<GenericRecord>& data );

            /**
             * Decodes avro encoded data (given the schema string) into GenericRecord objects.
             *
             * @param[in]  schemaString  Avro schema string used for decoding the data
             * @param[in]  encodedData   Avro encoded binary data containing a single record.
             * @param[out] data          A decoded GenericRecord object
             */
            static void decode(const std::string& schemaString, const std::vector<uint8_t>& encodedData, GenericRecord& data );

        private:

            Type m_type;
            std::vector<boost::any> m_values;

            GenericRecord(const std::vector<std::pair<Type::Column::ColumnType, bool> >& columnTypes);
            void initializeColumn(const Type::Column::ColumnType type, const bool isNullable);

            static void transpose(const std::string& schemaString, const std::vector<uint8_t>& encodedData, std::vector<GenericRecord>& data, gpudb_type_ptr_t &dataTypePtr );

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

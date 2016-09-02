#include "gpudb/DynamicTableRecord.hpp"

#include "gpudb/Avro.hpp"
#include "gpudb/GPUdbException.hpp"

#include <avro/Exception.hh>
#include <boost/lexical_cast.hpp>

namespace gpudb {
    const Type& DynamicTableRecord::getType() const
    {
        return *type;
    }

    const ::avro::ValidSchema& DynamicTableRecord::getSchema() const
    {
        return type->getSchema();
    }

    std::vector<uint8_t>& DynamicTableRecord::asBytes(const size_t index)
    {
        return data->value< ::avro::GenericRecord>().fieldAt(index).value< ::avro::GenericArray>().value()[recordIndex].value<std::vector<uint8_t> >();
    }

    const std::vector<uint8_t>& DynamicTableRecord::asBytes(const size_t index) const
    {
        return data->value< ::avro::GenericRecord>().fieldAt(index).value< ::avro::GenericArray>().value()[recordIndex].value<std::vector<uint8_t> >();
    }

    std::vector<uint8_t>& DynamicTableRecord::asBytes(const std::string& name)
    {
        return data->value< ::avro::GenericRecord>().fieldAt(type->getColumnIndex(name)).value< ::avro::GenericArray>().value()[recordIndex].value<std::vector<uint8_t> >();
    }

    const std::vector<uint8_t>& DynamicTableRecord::asBytes(const std::string& name) const
    {
        return data->value< ::avro::GenericRecord>().fieldAt(type->getColumnIndex(name)).value< ::avro::GenericArray>().value()[recordIndex].value<std::vector<uint8_t> >();
    }

    double& DynamicTableRecord::asDouble(const size_t index)
    {
        return data->value< ::avro::GenericRecord>().fieldAt(index).value< ::avro::GenericArray>().value()[recordIndex].value<double>();
    }

    const double& DynamicTableRecord::asDouble(const size_t index) const
    {
        return data->value< ::avro::GenericRecord>().fieldAt(index).value< ::avro::GenericArray>().value()[recordIndex].value<double>();
    }

    double& DynamicTableRecord::asDouble(const std::string& name)
    {
        return data->value< ::avro::GenericRecord>().fieldAt(type->getColumnIndex(name)).value< ::avro::GenericArray>().value()[recordIndex].value<double>();
    }

    const double& DynamicTableRecord::asDouble(const std::string& name) const
    {
        return data->value< ::avro::GenericRecord>().fieldAt(type->getColumnIndex(name)).value< ::avro::GenericArray>().value()[recordIndex].value<double>();
    }

    float& DynamicTableRecord::asFloat(const size_t index)
    {
        return data->value< ::avro::GenericRecord>().fieldAt(index).value< ::avro::GenericArray>().value()[recordIndex].value<float>();
    }

    const float& DynamicTableRecord::asFloat(const size_t index) const
    {
        return data->value< ::avro::GenericRecord>().fieldAt(index).value< ::avro::GenericArray>().value()[recordIndex].value<float>();
    }

    float& DynamicTableRecord::asFloat(const std::string& name)
    {
        return data->value< ::avro::GenericRecord>().fieldAt(type->getColumnIndex(name)).value< ::avro::GenericArray>().value()[recordIndex].value<float>();
    }

    const float& DynamicTableRecord::asFloat(const std::string& name) const
    {
        return data->value< ::avro::GenericRecord>().fieldAt(type->getColumnIndex(name)).value< ::avro::GenericArray>().value()[recordIndex].value<float>();
    }

    int32_t& DynamicTableRecord::asInt(const size_t index)
    {
        return data->value< ::avro::GenericRecord>().fieldAt(index).value< ::avro::GenericArray>().value()[recordIndex].value<int32_t>();
    }

    const int32_t& DynamicTableRecord::asInt(const size_t index) const
    {
        return data->value< ::avro::GenericRecord>().fieldAt(index).value< ::avro::GenericArray>().value()[recordIndex].value<int32_t>();
    }

    int32_t& DynamicTableRecord::asInt(const std::string& name)
    {
        return data->value< ::avro::GenericRecord>().fieldAt(type->getColumnIndex(name)).value< ::avro::GenericArray>().value()[recordIndex].value<int32_t>();
    }

    const int32_t& DynamicTableRecord::asInt(const std::string& name) const
    {
        return data->value< ::avro::GenericRecord>().fieldAt(type->getColumnIndex(name)).value< ::avro::GenericArray>().value()[recordIndex].value<int32_t>();
    }

    int64_t& DynamicTableRecord::asLong(const size_t index)
    {
        return data->value< ::avro::GenericRecord>().fieldAt(index).value< ::avro::GenericArray>().value()[recordIndex].value<int64_t>();
    }

    const int64_t& DynamicTableRecord::asLong(const size_t index) const
    {
        return data->value< ::avro::GenericRecord>().fieldAt(index).value< ::avro::GenericArray>().value()[recordIndex].value<int64_t>();
    }

    int64_t& DynamicTableRecord::asLong(const std::string& name)
    {
        return data->value< ::avro::GenericRecord>().fieldAt(type->getColumnIndex(name)).value< ::avro::GenericArray>().value()[recordIndex].value<int64_t>();
    }

    const int64_t& DynamicTableRecord::asLong(const std::string& name) const
    {
        return data->value< ::avro::GenericRecord>().fieldAt(type->getColumnIndex(name)).value< ::avro::GenericArray>().value()[recordIndex].value<int64_t>();
    }

    std::string& DynamicTableRecord::asString(const size_t index)
    {
        return data->value< ::avro::GenericRecord>().fieldAt(index).value< ::avro::GenericArray>().value()[recordIndex].value<std::string>();
    }

    const std::string& DynamicTableRecord::asString(const size_t index) const
    {
        return data->value< ::avro::GenericRecord>().fieldAt(index).value< ::avro::GenericArray>().value()[recordIndex].value<std::string>();
    }

    std::string& DynamicTableRecord::asString(const std::string& name)
    {
        return data->value< ::avro::GenericRecord>().fieldAt(type->getColumnIndex(name)).value< ::avro::GenericArray>().value()[recordIndex].value<std::string>();
    }

    const std::string& DynamicTableRecord::asString(const std::string& name) const
    {
        return data->value< ::avro::GenericRecord>().fieldAt(type->getColumnIndex(name)).value< ::avro::GenericArray>().value()[recordIndex].value<std::string>();
    }

    void DynamicTableRecord::transpose(const std::string& schemaString, const std::vector<uint8_t>& encodedData, std::vector<DynamicTableRecord>& result)
    {
        ::avro::ValidSchema schema = ::avro::compileJsonSchemaFromString(schemaString);
        const ::avro::NodePtr& root = schema.root();

        if (root->type() != ::avro::AVRO_RECORD)
        {
            throw GPUdbException("Schema must be of type record.");
        }

        boost::shared_ptr< ::avro::GenericDatum> data = boost::make_shared< ::avro::GenericDatum>(schema);
        avro::decode(*data, encodedData);
        size_t fieldCount = root->leaves() - 1;
        size_t recordCount = data->value< ::avro::GenericRecord>().fieldAt(0).value< ::avro::GenericArray>().value().size();
        std::vector< ::avro::GenericDatum> expressions = data->value< ::avro::GenericRecord>().fieldAt(fieldCount).value< ::avro::GenericArray>().value();
        std::vector<Type::Column> columns;

        for (size_t i = 0; i < fieldCount; ++i)
        {
            const ::avro::NodePtr& leaf = root->leafAt(i);

            if (leaf->type() != ::avro::AVRO_ARRAY)
            {
                throw GPUdbException("Field " + root->nameAt(i) + " must be of type array.");
            }

            switch (leaf->leafAt(0)->type())
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

            if (data->value< ::avro::GenericRecord>().fieldAt(i).value< ::avro::GenericArray>().value().size() != recordCount)
            {
                throw GPUdbException("Fields must all have the same number of elements.");
            }

            std::string name = expressions[i].value<std::string>();

            for (size_t j = 0; j < i; ++j)
            {
                if (name == columns[j].getName())
                {
                    for (size_t n = 2; ; ++n)
                    {
                        std::string tempName = name + "_" + boost::lexical_cast<std::string>(n);
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
                                if (tempName == expressions[k].value<std::string>())
                                {
                                    found = true;
                                    break;
                                }
                            }
                        }

                        if (!found)
                        {
                            name = tempName;
                            break;
                        }
                    }

                    break;
                }
            }

            columns.push_back(Type::Column(name, static_cast<Type::Column::ColumnType>(leaf->leafAt(0)->type())));
        }

        boost::shared_ptr<Type> type = boost::make_shared<Type>(Type(columns));

        for (size_t i = 0; i < recordCount; ++i) {
            result.push_back(DynamicTableRecord(type, data, i));
        }
    }

    DynamicTableRecord::DynamicTableRecord(const boost::shared_ptr<Type>& type, const boost::shared_ptr< ::avro::GenericDatum>& data, const size_t recordIndex) :
        type(type),
        data(data),
        recordIndex(recordIndex)
    {
    }
}
#include "gpudb/DynamicTableRecord.hpp"

#include "gpudb/Avro.hpp"
#include "gpudb/GPUdbException.hpp"

#include <avro/Exception.hh>
#include <boost/lexical_cast.hpp>

namespace gpudb {
    DynamicTableRecord::DynamicTableRecord(const boost::shared_ptr< ::avro::ValidSchema>& schema, const boost::shared_ptr< ::avro::GenericDatum>& data, const size_t recordIndex) :
        schema(schema),
        data(data),
        recordIndex(recordIndex)
    {
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
        return data->value< ::avro::GenericRecord>().fieldAt(getFieldIndex(name)).value< ::avro::GenericArray>().value()[recordIndex].value<std::vector<uint8_t> >();
    }

    const std::vector<uint8_t>& DynamicTableRecord::asBytes(const std::string& name) const
    {
        return data->value< ::avro::GenericRecord>().fieldAt(getFieldIndex(name)).value< ::avro::GenericArray>().value()[recordIndex].value<std::vector<uint8_t> >();
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
        return data->value< ::avro::GenericRecord>().fieldAt(getFieldIndex(name)).value< ::avro::GenericArray>().value()[recordIndex].value<double>();
    }

    const double& DynamicTableRecord::asDouble(const std::string& name) const
    {
        return data->value< ::avro::GenericRecord>().fieldAt(getFieldIndex(name)).value< ::avro::GenericArray>().value()[recordIndex].value<double>();
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
        return data->value< ::avro::GenericRecord>().fieldAt(getFieldIndex(name)).value< ::avro::GenericArray>().value()[recordIndex].value<float>();
    }

    const float& DynamicTableRecord::asFloat(const std::string& name) const
    {
        return data->value< ::avro::GenericRecord>().fieldAt(getFieldIndex(name)).value< ::avro::GenericArray>().value()[recordIndex].value<float>();
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
        return data->value< ::avro::GenericRecord>().fieldAt(getFieldIndex(name)).value< ::avro::GenericArray>().value()[recordIndex].value<int32_t>();
    }

    const int32_t& DynamicTableRecord::asInt(const std::string& name) const
    {
        return data->value< ::avro::GenericRecord>().fieldAt(getFieldIndex(name)).value< ::avro::GenericArray>().value()[recordIndex].value<int32_t>();
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
        return data->value< ::avro::GenericRecord>().fieldAt(getFieldIndex(name)).value< ::avro::GenericArray>().value()[recordIndex].value<int64_t>();
    }

    const int64_t& DynamicTableRecord::asLong(const std::string& name) const
    {
        return data->value< ::avro::GenericRecord>().fieldAt(getFieldIndex(name)).value< ::avro::GenericArray>().value()[recordIndex].value<int64_t>();
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
        return data->value< ::avro::GenericRecord>().fieldAt(getFieldIndex(name)).value< ::avro::GenericArray>().value()[recordIndex].value<std::string>();
    }

    const std::string& DynamicTableRecord::asString(const std::string& name) const
    {
        return data->value< ::avro::GenericRecord>().fieldAt(getFieldIndex(name)).value< ::avro::GenericArray>().value()[recordIndex].value<std::string>();
    }

    const std::string& DynamicTableRecord::getExpression(const size_t index) const
    {
        return data->value< ::avro::GenericRecord>().fieldAt(getFieldCount()).value< ::avro::GenericArray>().value()[index].value<std::string>();
    }

    const std::string& DynamicTableRecord::getExpression(const std::string& name) const
    {
        return data->value< ::avro::GenericRecord>().fieldAt(getFieldCount()).value< ::avro::GenericArray>().value()[getFieldIndex(name)].value<std::string>();
    }

    size_t DynamicTableRecord::getFieldCount() const
    {
        return schema->root()->names();
    }

    size_t DynamicTableRecord::getFieldIndex(const std::string& name) const
    {
        size_t index = 0;
        
        if (!schema->root()->nameIndex(name, index))
        {
            throw ::avro::Exception("Invalid field name: " + name);
        }
        
        return index;
    }

    const std::string& DynamicTableRecord::getFieldName(const size_t index) const
    {
        return schema->root()->nameAt(index);
    }

    ::avro::Type DynamicTableRecord::getFieldType(const size_t index) const
    {
        return schema->root()->leafAt(index)->type();
    }

    ::avro::Type DynamicTableRecord::getFieldType(const std::string& name) const
    {
        return schema->root()->leafAt(getFieldIndex(name))->type();
    }

    const ::avro::ValidSchema& DynamicTableRecord::getSchema() const
    {
        return *schema;
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
        ::avro::RecordSchema recordSchema(root->name());
        size_t fieldCount = root->leaves() - 1;
        std::vector< ::avro::GenericDatum> expressions = data->value< ::avro::GenericRecord>().fieldAt(fieldCount).value< ::avro::GenericArray>().value();
        std::vector<std::string> names;

        for (size_t i = 0; i < fieldCount; ++i)
        {
            std::string name = expressions[i].value<std::string>();
            bool valid = true;
            
            for (size_t j = 0; j < name.length(); ++j)
            {
                char c = name[j];
                
                if (!(j > 0 && c >= '0' && c <= '9')
                        && !(c >= 'A' && c <= 'Z')
                        && !(c >= 'a' && c <= 'z')
                        && c != '_')
                {
                    valid = false;
                    break;
                }
            }
            
            if (!valid)
            {
                name = "column_" + boost::lexical_cast<std::string>(i);
                bool found = false;
                
                for (size_t j = 0; j < fieldCount; ++j)
                {
                    if (expressions[j].value<std::string>() == name)
                    {
                        found = true;
                        break;
                    }
                }

                if (found)
                {
                    int j = 2;
                    
                    do
                    {
                        name = "column_" + boost::lexical_cast<std::string>(i) + "_" + boost::lexical_cast<std::string>(j);
                        found = false;
                        
                        for (size_t k = 0; k < fieldCount; ++k)
                        {
                            if (expressions[k].value<std::string>() == name)
                            {
                                found = true;
                                break;
                            }
                        }
                        
                        j++;            
                    }
                    while (found);
                }
            }
            
            names.push_back(name);
        }
        
        for (size_t i = 0; i < fieldCount; ++i)
        {    
            const ::avro::NodePtr& leaf = root->leafAt(i);
            const std::string& name = root->nameAt(i);

            if (leaf->type() != ::avro::AVRO_ARRAY)
            {
                throw new GPUdbException("Field " + name + " must be of type AVRO_ARRAY.");
            }

            ::avro::Type type = leaf->leafAt(0)->type();

            switch (type)
            {
                case ::avro::AVRO_BYTES:
                    recordSchema.addField(names[i], ::avro::BytesSchema());
                    break;

                case ::avro::AVRO_DOUBLE:
                    recordSchema.addField(names[i], ::avro::DoubleSchema());
                    break;

                case ::avro::AVRO_FLOAT:
                    recordSchema.addField(names[i], ::avro::FloatSchema());
                    break;

                case ::avro::AVRO_INT:
                    recordSchema.addField(names[i], ::avro::IntSchema());
                    break;

                case ::avro::AVRO_LONG:
                    recordSchema.addField(names[i], ::avro::LongSchema());
                    break;

                case ::avro::AVRO_STRING:
                    recordSchema.addField(names[i], ::avro::StringSchema());
                    break;

                default:
                    throw std::invalid_argument("Field " + name + " must be of type AVRO_BYTES, AVRO_DOUBLE, AVRO_FLOAT, AVRO_INT, AVRO_LONG or AVRO_STRING.");
            }
        }

        boost::shared_ptr< ::avro::ValidSchema> newSchema = boost::make_shared< ::avro::ValidSchema>();
        newSchema->setSchema(recordSchema);
        size_t recordCount = data->value< ::avro::GenericRecord>().fieldAt(0).value< ::avro::GenericArray>().value().size();

        for (size_t i = 0; i < recordCount; ++i) {
            result.push_back(DynamicTableRecord(newSchema, data, i));
        }
    }
}

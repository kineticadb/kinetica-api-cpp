#include "Type.hpp"
#include "GPUdb.hpp"



namespace gpudb
{
    Type::Column::Column(const std::string& name, const ::avro::Type type) :
        name(name),
        type(type)
    {
        validate();
    }

    Type::Column::Column(const std::string& name, const ::avro::Type type, const std::vector<std::string>& properties) :
        name(name),
        type(type),
        properties(properties)
    {
        validate();
    }

    const std::string& Type::Column::getName() const
    {
        return name;
    }

    ::avro::Type Type::Column::getType() const
    {
        return type;
    }

    const std::vector<std::string>& Type::Column::getProperties() const
    {
        return properties;
    }

    void Type::Column::validate() const
    {
        if (name.empty())
        {
            throw std::invalid_argument("Name must not be empty.");
        }

        switch (type)
        {
            case ::avro::AVRO_BYTES:
            case ::avro::AVRO_DOUBLE:
            case ::avro::AVRO_FLOAT:
            case ::avro::AVRO_INT:
            case ::avro::AVRO_LONG:
            case ::avro::AVRO_STRING:
                break;

            default:
                throw std::invalid_argument("Column " + name + " must be of type AVRO_BYTES, AVRO_DOUBLE, AVRO_FLOAT, AVRO_INT, AVRO_LONG or AVRO_STRING.");
        }
    }

    Type::Type(const std::string& label, const std::vector<Type::Column>& columns) :
        label(label),
        columns(columns)
    {
        if (columns.empty())
        {
            throw std::invalid_argument("At least one column must be specified.");
        }

        ::avro::RecordSchema recordSchema("type_name");

        for (std::vector<Type::Column>::const_iterator it = columns.begin(); it != columns.end(); ++it)
        {
            switch (it->getType())
            {
                case ::avro::AVRO_BYTES:
                    recordSchema.addField(it->getName(), ::avro::BytesSchema());
                    break;

                case ::avro::AVRO_DOUBLE:
                    recordSchema.addField(it->getName(), ::avro::DoubleSchema());
                    break;

                case ::avro::AVRO_FLOAT:
                    recordSchema.addField(it->getName(), ::avro::FloatSchema());
                    break;

                case ::avro::AVRO_INT:
                    recordSchema.addField(it->getName(), ::avro::IntSchema());
                    break;

                case ::avro::AVRO_LONG:
                    recordSchema.addField(it->getName(), ::avro::LongSchema());
                    break;

                case ::avro::AVRO_STRING:
                    recordSchema.addField(it->getName(), ::avro::StringSchema());
                    break;

                default:
                    throw std::invalid_argument("Column " + it->getName() + " must be of type AVRO_BYTES, AVRO_DOUBLE, AVRO_FLOAT, AVRO_INT, AVRO_LONG or AVRO_STRING.");
            }
        }

        schema.setSchema(recordSchema);
    }

    Type::Type(const std::string& label, const std::string& typeSchema, const std::map<std::string, std::vector<std::string> >& properties) :
        label(label)
    {
        schema = ::avro::compileJsonSchemaFromString(typeSchema);
        ::avro::NodePtr root = schema.root();

        if (root->type() != ::avro::AVRO_RECORD)
        {
            throw std::invalid_argument("Schema must be of type record.");
        }

        size_t count = root->leaves();

        for (size_t i = 0; i < count; ++i)
        {
            const std::string& name = root->nameAt(i);
            ::avro::Type type = root->leafAt(i)->type();
            std::map<std::string, std::vector<std::string> >::const_iterator columnProperties = properties.find(name);

            switch (type)
            {
                case ::avro::AVRO_BYTES:
                case ::avro::AVRO_DOUBLE:
                case ::avro::AVRO_FLOAT:
                case ::avro::AVRO_INT:
                case ::avro::AVRO_LONG:
                case ::avro::AVRO_STRING:
                    break;

                default:
                    throw std::invalid_argument("Field " + name + " must be of type AVRO_BYTES, AVRO_DOUBLE, AVRO_FLOAT, AVRO_INT, AVRO_LONG or AVRO_STRING.");
            }

            if (columnProperties == properties.end())
            {
                columns.push_back(Column(name, type));
            }
            else
            {
                columns.push_back(Column(name, type, columnProperties->second));
            }
        }
    }

    Type Type::fromTable(const GPUdb& gpudb, const std::string& tableName)
    {
        ShowTableResponse response;
        std::map<std::string, std::string> options;
        gpudb.showTable(tableName, options, response);

        if (response.typeIds.empty())
        {
            throw GPUdbException("Table " + tableName + " does not exist.");
        }

        std::string& typeId = response.typeIds[0];

        for (std::vector<std::string>::const_iterator it = response.typeIds.begin(); it != response.typeIds.end(); ++it)
        {
            if (*it != typeId)
            {
                throw GPUdbException("Table " + tableName + " is not homogeneous.");
            }
        }

        return Type(response.typeLabels[0], response.typeSchemas[0], response.properties[0]);
    }

    Type Type::fromType(const GPUdb& gpudb, const std::string& typeId)
    {
        ShowTypesResponse response;
        std::map<std::string, std::string> options;
        gpudb.showTypes(typeId, "", options);

        if (response.typeIds.empty())
        {
            throw GPUdbException("Type " + typeId + " does not exist.");
        }

        return Type(response.labels[0], response.typeSchemas[0], response.properties[0]);
    }

    const std::vector<Type::Column>& Type::getColumns() const
    {
        return columns;
    }

    const std::string& Type::getLabel() const
    {
        return label;
    }

    const ::avro::ValidSchema& Type::getSchema() const
    {
        return schema;
    }

    std::string Type::create(const GPUdb& gpudb) const
    {
        std::ostringstream stream;
        schema.toJson(stream);
        std::map<std::string, std::vector<std::string> > properties;

        for (std::vector<Type::Column>::const_iterator it = columns.begin(); it != columns.end(); ++it)
        {
            properties[it->getName()] = it->getProperties();
        }

        CreateTypeResponse response;
        return gpudb.createType(stream.str(), label, properties, std::map<std::string, std::string>(), response).typeId;
    }
}

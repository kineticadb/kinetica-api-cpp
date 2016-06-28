#include "gpudb/Type.hpp"

#include "gpudb/GPUdb.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace gpudb
{
    Type::Column::Column(const std::string& name, const ColumnType type) :
        name(name),
        type(type)
    {
        validate();
    }

    Type::Column::Column(const std::string& name, const ColumnType type, const std::vector<std::string>& properties) :
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

    Type::Column::ColumnType Type::Column::getType() const
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
            case BYTES:
            case DOUBLE:
            case FLOAT:
            case INT:
            case LONG:
            case STRING:
                break;

            default:
                throw std::invalid_argument("Column " + name + " must be of type BYTES, DOUBLE, FLOAT, INT, LONG or STRING.");
        }

        for (std::vector<std::string>::const_iterator it = properties.begin(); it != properties.end(); ++it)
        {
            if (it->empty())
            {
                throw std::invalid_argument("Properties must not be empty.");
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

    Type::Type(const std::vector<Type::Column>& columns) :
        columns(columns)
    {
        validate();
        createSchema();
    }

    Type::Type(const std::string& label, const std::vector<Type::Column>& columns) :
        label(label),
        columns(columns)
    {
        validate();
        createSchema();
    }

    Type::Type(const std::string& typeSchema)
    {
        std::map<std::string, std::vector<std::string> > properties;
        createFromSchema(typeSchema, properties);
        createSchema();
    }

    Type::Type(const std::string& label, const std::string& typeSchema, const std::map<std::string, std::vector<std::string> >& properties) :
        label(label)
    {
        createFromSchema(typeSchema, properties);
        createSchema();
    }

    const std::string& Type::getLabel() const
    {
        return label;
    }

    const std::vector<Type::Column>& Type::getColumns() const
    {
        return columns;
    }

    const Type::Column& Type::getColumn(size_t index) const
    {
        return columns.at(index);
    }

    const Type::Column& Type::getColumn(const std::string& name) const
    {
        return columns[getColumnIndex(name)];
    }

    size_t Type::getColumnCount() const
    {
        return columns.size();
    }

    size_t Type::getColumnIndex(const std::string& name) const
    {
        std::map<std::string, size_t>::const_iterator column = columnMap.find(name);

        if (column == columnMap.end())
        {
            throw std::out_of_range("Column " + name + " does not exist.");
        }

        return column->second;
    }

    bool Type::hasColumn(const std::string& name) const
    {
        return columnMap.find(name) != columnMap.end();
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

    void Type::createFromSchema(const std::string& typeSchema, const std::map<std::string, std::vector<std::string> >& properties)
    {
        std::stringstream schemaStream;
        schemaStream << typeSchema;
        boost::property_tree::ptree root;

        try
        {
            boost::property_tree::read_json(schemaStream, root);
        }
        catch (const std::exception& ex)
        {
            throw GPUdbException("Schema is invalid: " + std::string(ex.what()));
        }

        boost::optional<std::string> rootType = root.get_optional<std::string>("type");

        if (!rootType || *rootType != "record")
        {
            throw GPUdbException("Schema must be of type record.");
        }

        boost::optional<boost::property_tree::ptree&> fields = root.get_child_optional("fields");

        if (!fields || fields->empty())
        {
            throw GPUdbException("Schema has no fields.");
        }

        for (boost::property_tree::ptree::const_iterator field = fields->begin(); field != fields->end(); ++field)
        {
            if (field->second.empty())
            {
                throw GPUdbException("Schema has invalid field.");
            }

            boost::optional<std::string> fieldName = field->second.get_optional<std::string>("name");

            if (!fieldName)
            {
                throw GPUdbException("Schema has unnamed field.");
            }

            for (std::vector<Column>::const_iterator column = columns.begin(); column != columns.end(); ++column)
            {
                if (column->getName() == *fieldName)
                {
                    throw GPUdbException("Duplicate field name " + *fieldName + ".");
                }
            }

            boost::optional<std::string> fieldType = field->second.get_optional<std::string>("type");

            if (!fieldType)
            {
                throw GPUdbException("Field " + *fieldName + " has no type.");
            }

            Column::ColumnType columnType;

            if (*fieldType == "bytes")
            {
                columnType = Column::BYTES;
            }
            else if (*fieldType == "double")
            {
                columnType = Column::DOUBLE;
            }
            else if (*fieldType == "float")
            {
                columnType = Column::FLOAT;
            }
            else if (*fieldType == "int")
            {
                columnType = Column::INT;
            }
            else if (*fieldType == "long")
            {
                columnType = Column::LONG;
            }
            else if (*fieldType == "string")
            {
                columnType = Column::STRING;
            }
            else
            {
                throw GPUdbException("Field " + *fieldName + " must be of type bytes, double, float, int, long or string.");
            }

            std::map<std::string, std::vector<std::string> >::const_iterator columnProperties = properties.find(*fieldName);

            if (columnProperties == properties.end())
            {
                columns.push_back(Column(*fieldName, columnType));
            }
            else
            {
                columns.push_back(Column(*fieldName, columnType, columnProperties->second));
            }
        }

        for (size_t i = 0; i < columns.size(); ++i)
        {
            columnMap[columns[i].getName()] = i;
        }
    }

    void Type::validate()
    {
        if (columns.empty())
        {
            throw std::invalid_argument("At least one column must be specified.");
        }

        for (size_t i = 0; i < columns.size(); ++i)
        {
            if (columnMap.find(columns[i].getName()) != columnMap.end())
            {
                throw std::invalid_argument("Duplicate column name " + columns[i].getName() + " specified.");
            }

            columnMap[columns[i].getName()] = i;
        }
    }

    void Type::createSchema()
    {
        ::avro::RecordSchema recordSchema("type_name");
        std::vector<std::string> fields;

        for (size_t i = 0; i < columns.size(); ++i)
        {
            std::string baseFieldName = columns[i].getName();

            for (size_t j = 0; j < baseFieldName.size(); ++j)
            {
                char c = baseFieldName[j];

                if (!(j > 0 && c >= '0' && c <= '9')
                        && !(c >= 'A' && c <= 'Z')
                        && !(c >= 'a' && c <= 'z')
                        && c != '_')
                {
                    baseFieldName[j] = '_';
                }
            }

            std::string fieldName;

            for (size_t n = 1; ; ++n)
            {
                fieldName = baseFieldName + (n > 1 ? "_" + boost::lexical_cast<std::string>(n) : "");
                bool found = false;

                for (size_t j = 0; j < columns.size(); ++j)
                {
                    if (j == i)
                    {
                        continue;
                    }

                    if (columns[j].getName() == fieldName)
                    {
                        found = true;
                        break;
                    }
                }

                if (!found)
                {
                    for (size_t j = 0; j < fields.size(); ++j)
                    {
                        if (fields[j] == fieldName)
                        {
                            found = true;
                            break;
                        }
                    }
                }

                if (!found)
                {
                    break;
                }
            }

            fields.push_back(fieldName);

            switch (columns[i].getType())
            {
                case Column::BYTES:
                    recordSchema.addField(fieldName, ::avro::BytesSchema());
                    break;

                case Column::DOUBLE:
                    recordSchema.addField(fieldName, ::avro::DoubleSchema());
                    break;

                case Column::FLOAT:
                    recordSchema.addField(fieldName, ::avro::FloatSchema());
                    break;

                case Column::INT:
                    recordSchema.addField(fieldName, ::avro::IntSchema());
                    break;

                case Column::LONG:
                    recordSchema.addField(fieldName, ::avro::LongSchema());
                    break;

                case Column::STRING:
                    recordSchema.addField(fieldName, ::avro::StringSchema());
                    break;

                default:
                    throw std::invalid_argument("Column " + columns[i].getName() + " must be of type BYTES, DOUBLE, FLOAT, INT, LONG or STRING.");
            }
        }

        schema.setSchema(recordSchema);
    }
}
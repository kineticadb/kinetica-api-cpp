#ifndef __GPUDB__TYPE_HPP__
#define __GPUDB__TYPE_HPP__

#include "ColumnProperties.h"

#include <avro/Schema.hh>

namespace gpudb
{
    class GPUdb;

    class Type
    {
        friend class GenericRecord;

        public:
            class Column
            {
                public:
                    enum ColumnType
                    {
                        BYTES = ::avro::AVRO_BYTES,
                        DOUBLE = ::avro::AVRO_DOUBLE,
                        FLOAT = ::avro::AVRO_FLOAT,
                        INT = ::avro::AVRO_INT,
                        LONG = ::avro::AVRO_LONG,
                        STRING = ::avro::AVRO_STRING
                    };

                    Column(const std::string& name, const ColumnType type, const std::string& property1 = "", const std::string& property2 = "", const std::string& property3 = "");
                    Column(const std::string& name, const ColumnType type, const std::vector<std::string>& properties);
                    const std::string& getName() const;
                    ColumnType getType() const;
                    bool isNullable() const;
                    const std::vector<std::string>& getProperties() const;
                    bool hasProperty( std::string property ) const;

                    friend bool operator==(const Column &lhs, const Column &rhs);  // overloading ==
                    friend std::ostream &operator << (std::ostream &os, const Column &column);
                    friend std::ostream &operator << (std::ostream &os, Column &column);

                private:
                    std::string m_name;
                    ColumnType m_type;
                    bool m_isNullable;
                    std::vector<std::string> m_properties;

                    void initialize();
            };

            static Type fromTable(const GPUdb& gpudb, const std::string& tableName);
            static Type fromType(const GPUdb& gpudb, const std::string& typeId);

            Type(const std::vector<Column>& columns);
            Type(const std::string& label, const std::vector<Column>& columns);
            Type(const std::string& typeSchema);
            Type(const std::string& label, const std::string& typeSchema, const std::map<std::string, std::vector<std::string> >& properties);
            const std::string& getLabel() const;
            const std::vector<Column>& getColumns() const;
            const Column& getColumn(const size_t index) const;
            const Column& getColumn(const std::string& name) const;
            size_t getColumnCount() const;
            size_t getColumnIndex(const std::string& name) const;
            bool hasColumn(const std::string& name) const;
            const ::avro::ValidSchema& getSchema() const;
            std::string create(const GPUdb& gpudb);

            friend bool operator==(const Type &lhs, const Type &rhs);  // overloading ==
            friend bool operator!=(const Type &lhs, const Type &rhs) { return !(lhs == rhs); }

        private:
            struct TypeData
            {
                std::string label;
                std::vector<Type::Column> columns;
                std::map<std::string, size_t> columnMap;
                ::avro::ValidSchema schema;
            };

            boost::shared_ptr<TypeData> m_data;
            std::string m_type_id;
            bool m_has_been_created = false;

            Type();
            Type(const std::string& label, const std::string& typeSchema, const std::map<std::string, std::vector<std::string> >& properties, const std::string& type_id);
            void initialize();
            void createFromSchema(const std::string& typeSchema, const std::map<std::string, std::vector<std::string> >& properties);
            void createSchema();
    };
}

#endif

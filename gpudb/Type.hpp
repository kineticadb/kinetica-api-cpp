#ifndef __GPUDB__TYPE_HPP__
#define __GPUDB__TYPE_HPP__

#include <avro/Schema.hh>

namespace gpudb
{
    class GPUdb;

    class Type
    {
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

                    Column(const std::string& name, const ColumnType type);
                    Column(const std::string& name, const ColumnType type, const std::vector<std::string>& properties);
                    const std::string& getName() const;
                    ColumnType getType() const;
                    const std::vector<std::string>& getProperties() const;

                private:
                    std::string name;
                    ColumnType type;
                    std::vector<std::string> properties;

                    void validate() const;
            };

            static Type fromTable(const GPUdb& gpudb, const std::string& tableName);
            static Type fromType(const GPUdb& gpudb, const std::string& typeId);

            Type(const std::vector<Column>& columns);
            Type(const std::string& label, const std::vector<Column>& columns);
            Type(const std::string& typeSchema);
            Type(const std::string& label, const std::string& typeSchema, const std::map<std::string, std::vector<std::string> >& properties);
            const std::string& getLabel() const;
            const std::vector<Column>& getColumns() const;
            const Column& getColumn(size_t index) const;
            const Column& getColumn(const std::string& name) const;
            size_t getColumnCount() const;
            size_t getColumnIndex(const std::string& name) const;
            bool hasColumn(const std::string& name) const;
            const ::avro::ValidSchema& getSchema() const;
            std::string create(const GPUdb& gpudb) const;

        private:
            std::string label;
            std::vector<Column> columns;
            std::map<std::string, size_t> columnMap;
            ::avro::ValidSchema schema;

            void createFromSchema(const std::string& typeSchema, const std::map<std::string, std::vector<std::string> >& properties);
            void validate();
            void createSchema();
    };
}

#endif
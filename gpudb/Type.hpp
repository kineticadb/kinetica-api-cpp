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
                    Column(const std::string& name, const ::avro::Type type);
                    Column(const std::string& name, const ::avro::Type type, const std::vector<std::string>& properties);
                    const std::string& getName() const;
                    ::avro::Type getType() const;
                    const std::vector<std::string>& getProperties() const;

                private:
                    std::string name;
                    ::avro::Type type;
                    std::vector<std::string> properties;

                    void validate() const;
            };

            static Type fromTable(const GPUdb& gpudb, const std::string& tableName);
            static Type fromType(const GPUdb& gpudb, const std::string& typeId);

            Type(const std::string& label, const std::vector<Column>& columns);
            Type(const std::string& label, const std::string& typeSchema, const std::map<std::string, std::vector<std::string> >& properties);
            const std::string& getLabel() const;
            const std::vector<Column>& getColumns() const;
            const ::avro::ValidSchema& getSchema() const;
            std::string create(const GPUdb& gpudb) const;

        private:
            std::string label;
            std::vector<Column> columns;
            ::avro::ValidSchema schema;
    };
}

#endif

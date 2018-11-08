#ifndef __GPUDB__TYPE_HPP__
#define __GPUDB__TYPE_HPP__

#include "ColumnProperties.h"

#include <avro/Schema.hh>

namespace gpudb
{
    class GPUdb;

    /*
     * This class is thread safe and immutable.
     */
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
                    std::string getTypeName() const;
                    bool isNullable() const;
                    const std::vector<std::string>& getProperties() const;
                    bool hasProperty( std::string property ) const;

                    /// Check if the given column is compatible with this column
                    /// (checks name, primitive types and type-related properties, including nullability).
                    /// By default, does not check query compatibility (e.g. 'data',
                    /// 'disk_optimized', 'store_only', and 'text_search'.), but will if specified.
                    bool isColumnCompatible(const Column &other, bool check_query_compatibility = false ) const;

                    friend bool operator==(const Column &lhs, const Column &rhs);  // overloading ==
                    friend std::ostream &operator << (std::ostream &os, const Column &column);
                    friend std::ostream &operator << (std::ostream &os, Column &column);

                private:
                    std::string m_name;
                    ColumnType m_type;
                    bool m_isNullable;
                    std::vector<std::string> m_properties;  // kept sorted

                    void initialize();
            };

            /// Create a Type object from a GPUdb table
            static Type fromTable(const GPUdb& gpudb, const std::string& tableName);
            /// Create a Type object from a GPUdb type
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
            std::string create(const GPUdb& gpudb) const;

            /// Check if the given type's columns' data types are compatible
            /// (checks primitive types and type-related properties, including nullability).
            /// By default, does not check query compatibility (e.g. 'data',
            /// 'disk_optimized', 'store_only', and 'text_search'.), but will if specified.
            bool isTypeCompatible(const Type &other, bool check_query_compatibility = false ) const;

            friend std::ostream &operator << (std::ostream  &os, Type &type);
            friend std::ostream &operator << (std::ostream  &os, const Type &type);


        private:
            struct TypeData
            {
                std::string label;
                std::vector<Type::Column> columns;
                std::map<std::string, size_t> columnMap;
                ::avro::ValidSchema schema;
            };

            // Using a shared pointer to store all relevant data so that there is only
            // one instance of it across all copies of the same type
            // Do NOT add any member outside of m_data.
            boost::shared_ptr<TypeData> m_data;

            Type();
            void initialize();
            void createFromSchema(const std::string& typeSchema, const std::map<std::string, std::vector<std::string> >& properties);
            void createSchema();
    };
}

#endif

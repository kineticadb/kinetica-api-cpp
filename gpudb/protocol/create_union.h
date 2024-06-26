/*
 *  This file was autogenerated by the Kinetica schema processor.
 *
 *  DO NOT EDIT DIRECTLY.
 */
#ifndef __CREATE_UNION_H__
#define __CREATE_UNION_H__

namespace gpudb
{
    /**
     * A set of parameters for @ref
     * GPUdb::createUnion(const CreateUnionRequest&) const
     * "GPUdb::createUnion".
     *
     * Merges data from one or more tables with comparable data types into a
     * new table.
     *
     * The following merges are supported:
     *
     * UNION (DISTINCT/ALL) - For data set union details and examples, see <a
     * href="../../../concepts/unions/" target="_top">Union</a>.  For
     * limitations, see <a
     * href="../../../concepts/unions/#limitations-and-cautions"
     * target="_top">Union Limitations and Cautions</a>.
     *
     * INTERSECT (DISTINCT/ALL) - For data set intersection details and
     * examples, see <a href="../../../concepts/intersect/"
     * target="_top">Intersect</a>.  For limitations, see <a
     * href="../../../concepts/intersect/#limitations" target="_top">Intersect
     * Limitations</a>.
     *
     * EXCEPT (DISTINCT/ALL) - For data set subtraction details and examples,
     * see <a href="../../../concepts/except/" target="_top">Except</a>.  For
     * limitations, see <a href="../../../concepts/except/#limitations"
     * target="_top">Except Limitations</a>.
     *
     * MERGE VIEWS - For a given set of <a
     * href="../../../concepts/filtered_views/" target="_top">filtered
     * views</a> on a single table, creates a single filtered view containing
     * all of the unique records across all of the given filtered data sets.
     *
     * Non-charN 'string' and 'bytes' column types cannot be merged, nor can
     * columns marked as <a href="../../../concepts/types/#data-handling"
     * target="_top">store-only</a>.
     */
    struct CreateUnionRequest
    {
        /**
         * Constructs a CreateUnionRequest object with default parameters.
         */
        CreateUnionRequest() :
            tableName(std::string()),
            tableNames(std::vector<std::string>()),
            inputColumnNames(std::vector<std::vector<std::string> >()),
            outputColumnNames(std::vector<std::string>()),
            options(std::map<std::string, std::string>())
        {
        }

        /**
         * Constructs a CreateUnionRequest object with the specified
         * parameters.
         *
         * @param[in] tableName_  Name of the table to be created, in
         *                        [schema_name.]table_name format, using
         *                        standard <a
         *                        href="../../../concepts/tables/#table-name-resolution"
         *                        target="_top">name resolution rules</a> and
         *                        meeting <a
         *                        href="../../../concepts/tables/#table-naming-criteria"
         *                        target="_top">table naming criteria</a>.
         * @param[in] tableNames_  The list of table names to merge, in
         *                         [schema_name.]table_name format, using
         *                         standard <a
         *                         href="../../../concepts/tables/#table-name-resolution"
         *                         target="_top">name resolution rules</a>.
         *                         Must contain the names of one or more
         *                         existing tables.
         * @param[in] inputColumnNames_  The list of columns from each of the
         *                               corresponding input tables.
         * @param[in] outputColumnNames_  The list of names of the columns to
         *                                be stored in the output table.
         * @param[in] options_  Optional parameters.
         *                      <ul>
         *                          <li>@ref
         *                              gpudb::create_union_create_temp_table
         *                              "create_union_create_temp_table": If
         *                              @ref gpudb::create_union_true "true", a
         *                              unique temporary table name will be
         *                              generated in the sys_temp schema and
         *                              used in place of @a tableName_. If @ref
         *                              gpudb::create_union_persist "persist"
         *                              is @ref gpudb::create_union_false
         *                              "false" (or unspecified), then this is
         *                              always allowed even if the caller does
         *                              not have permission to create tables.
         *                              The generated name is returned in @ref
         *                              gpudb::create_union_qualified_table_name
         *                              "qualified_table_name".
         *                              Supported values:
         *                              <ul>
         *                                  <li>@ref gpudb::create_union_true
         *                                      "create_union_true"
         *                                  <li>@ref gpudb::create_union_false
         *                                      "create_union_false"
         *                              </ul>
         *                              The default value is @ref
         *                              gpudb::create_union_false
         *                              "create_union_false".
         *                          <li>@ref
         *                              gpudb::create_union_collection_name
         *                              "create_union_collection_name":
         *                              [DEPRECATED--please specify the
         *                              containing schema for the projection as
         *                              part of @a tableName_ and use @ref
         *                              GPUdb::createSchema(const CreateSchemaRequest&) const
         *                              "GPUdb::createSchema" to create the
         *                              schema if non-existent]  Name of the
         *                              schema for the output table. If the
         *                              schema provided is non-existent, it
         *                              will be automatically created. The
         *                              default value is ''.
         *                          <li>@ref gpudb::create_union_mode
         *                              "create_union_mode": If @ref
         *                              gpudb::create_union_merge_views
         *                              "merge_views", then this operation will
         *                              merge the provided views. All @a
         *                              tableNames_ must be views from the same
         *                              underlying base table.
         *                              Supported values:
         *                              <ul>
         *                                  <li>@ref
         *                                      gpudb::create_union_union_all
         *                                      "create_union_union_all":
         *                                      Retains all rows from the
         *                                      specified tables.
         *                                  <li>@ref gpudb::create_union_union
         *                                      "create_union_union": Retains
         *                                      all unique rows from the
         *                                      specified tables (synonym for
         *                                      @ref
         *                                      gpudb::create_union_union_distinct
         *                                      "union_distinct").
         *                                  <li>@ref
         *                                      gpudb::create_union_union_distinct
         *                                      "create_union_union_distinct":
         *                                      Retains all unique rows from
         *                                      the specified tables.
         *                                  <li>@ref gpudb::create_union_except
         *                                      "create_union_except": Retains
         *                                      all unique rows from the first
         *                                      table that do not appear in the
         *                                      second table (only works on 2
         *                                      tables).
         *                                  <li>@ref
         *                                      gpudb::create_union_except_all
         *                                      "create_union_except_all":
         *                                      Retains all rows(including
         *                                      duplicates) from the first
         *                                      table that do not appear in the
         *                                      second table (only works on 2
         *                                      tables).
         *                                  <li>@ref
         *                                      gpudb::create_union_intersect
         *                                      "create_union_intersect":
         *                                      Retains all unique rows that
         *                                      appear in both of the specified
         *                                      tables (only works on 2
         *                                      tables).
         *                                  <li>@ref
         *                                      gpudb::create_union_intersect_all
         *                                      "create_union_intersect_all":
         *                                      Retains all rows(including
         *                                      duplicates) that appear in both
         *                                      of the specified tables (only
         *                                      works on 2 tables).
         *                                  <li>@ref
         *                                      gpudb::create_union_merge_views
         *                                      "create_union_merge_views":
         *                                      Merge two or more views (or
         *                                      views of views) of the same
         *                                      base data set into a new view.
         *                                      If this mode is selected @a
         *                                      inputColumnNames_ AND @a
         *                                      outputColumnNames_ must be
         *                                      empty. The resulting view would
         *                                      match the results of a SQL OR
         *                                      operation, e.g., if filter 1
         *                                      creates a view using the
         *                                      expression 'x = 20' and filter
         *                                      2 creates a view using the
         *                                      expression 'x &lt;= 10', then
         *                                      the merge views operation
         *                                      creates a new view using the
         *                                      expression 'x = 20 OR x &lt;=
         *                                      10'.
         *                              </ul>
         *                              The default value is @ref
         *                              gpudb::create_union_union_all
         *                              "create_union_union_all".
         *                          <li>@ref gpudb::create_union_chunk_size
         *                              "create_union_chunk_size": Indicates
         *                              the number of records per chunk to be
         *                              used for this output table.
         *                          <li>@ref
         *                              gpudb::create_union_chunk_column_max_memory
         *                              "create_union_chunk_column_max_memory":
         *                              Indicates the target maximum data size
         *                              for each column in a chunk to be used
         *                              for this output table.
         *                          <li>@ref
         *                              gpudb::create_union_chunk_max_memory
         *                              "create_union_chunk_max_memory":
         *                              Indicates the target maximum data size
         *                              for all columns in a chunk to be used
         *                              for this output table.
         *                          <li>@ref gpudb::create_union_create_indexes
         *                              "create_union_create_indexes":
         *                              Comma-separated list of columns on
         *                              which to create indexes on the output
         *                              table.  The columns specified must be
         *                              present in @a outputColumnNames_.
         *                          <li>@ref gpudb::create_union_ttl
         *                              "create_union_ttl": Sets the <a
         *                              href="../../../concepts/ttl/"
         *                              target="_top">TTL</a> of the output
         *                              table specified in @a tableName_.
         *                          <li>@ref gpudb::create_union_persist
         *                              "create_union_persist": If @ref
         *                              gpudb::create_union_true "true", then
         *                              the output table specified in @a
         *                              tableName_ will be persisted and will
         *                              not expire unless a @ref
         *                              gpudb::create_union_ttl "ttl" is
         *                              specified.   If @ref
         *                              gpudb::create_union_false "false", then
         *                              the output table will be an in-memory
         *                              table and will expire unless a @ref
         *                              gpudb::create_union_ttl "ttl" is
         *                              specified otherwise.
         *                              Supported values:
         *                              <ul>
         *                                  <li>@ref gpudb::create_union_true
         *                                      "create_union_true"
         *                                  <li>@ref gpudb::create_union_false
         *                                      "create_union_false"
         *                              </ul>
         *                              The default value is @ref
         *                              gpudb::create_union_false
         *                              "create_union_false".
         *                          <li>@ref gpudb::create_union_view_id
         *                              "create_union_view_id": ID of view of
         *                              which this output table is a member.
         *                              The default value is ''.
         *                          <li>@ref
         *                              gpudb::create_union_force_replicated
         *                              "create_union_force_replicated": If
         *                              @ref gpudb::create_union_true "true",
         *                              then the output table specified in @a
         *                              tableName_ will be replicated even if
         *                              the source tables are not.
         *                              Supported values:
         *                              <ul>
         *                                  <li>@ref gpudb::create_union_true
         *                                      "create_union_true"
         *                                  <li>@ref gpudb::create_union_false
         *                                      "create_union_false"
         *                              </ul>
         *                              The default value is @ref
         *                              gpudb::create_union_false
         *                              "create_union_false".
         *                          <li>@ref
         *                              gpudb::create_union_strategy_definition
         *                              "create_union_strategy_definition": The
         *                              <a
         *                              href="../../../rm/concepts/#tier-strategies"
         *                              target="_top">tier strategy</a> for the
         *                              table and its columns.
         *                      </ul>
         *                      The default value is an empty map.
         */
        CreateUnionRequest(const std::string& tableName_, const std::vector<std::string>& tableNames_, const std::vector<std::vector<std::string> >& inputColumnNames_, const std::vector<std::string>& outputColumnNames_, const std::map<std::string, std::string>& options_):
            tableName( tableName_ ),
            tableNames( tableNames_ ),
            inputColumnNames( inputColumnNames_ ),
            outputColumnNames( outputColumnNames_ ),
            options( options_ )
        {
        }

        /**
         * Name of the table to be created, in [ schema_name.\ ]table_name
         * format, using standard <a
         * href="../../../concepts/tables/#table-name-resolution"
         * target="_top">name resolution rules</a> and meeting <a
         * href="../../../concepts/tables/#table-naming-criteria"
         * target="_top">table naming criteria</a>.
         */
        std::string tableName;

        /**
         * The list of table names to merge, in [ schema_name.\ ]table_name
         * format, using standard <a
         * href="../../../concepts/tables/#table-name-resolution"
         * target="_top">name resolution rules</a>.  Must contain the names of
         * one or more existing tables.
         */
        std::vector<std::string> tableNames;

        /**
         * The list of columns from each of the corresponding input tables.
         */
        std::vector<std::vector<std::string> > inputColumnNames;

        /**
         * The list of names of the columns to be stored in the output table.
         */
        std::vector<std::string> outputColumnNames;

        /**
         * Optional parameters.
         * <ul>
         *     <li>@ref gpudb::create_union_create_temp_table
         *         "create_union_create_temp_table": If @ref
         *         gpudb::create_union_true "true", a unique temporary table
         *         name will be generated in the sys_temp schema and used in
         *         place of @ref tableName. If @ref gpudb::create_union_persist
         *         "persist" is @ref gpudb::create_union_false "false" (or
         *         unspecified), then this is always allowed even if the caller
         *         does not have permission to create tables. The generated
         *         name is returned in @ref
         *         gpudb::create_union_qualified_table_name
         *         "qualified_table_name".
         *         Supported values:
         *         <ul>
         *             <li>@ref gpudb::create_union_true "create_union_true"
         *             <li>@ref gpudb::create_union_false "create_union_false"
         *         </ul>
         *         The default value is @ref gpudb::create_union_false
         *         "create_union_false".
         *     <li>@ref gpudb::create_union_collection_name
         *         "create_union_collection_name": [DEPRECATED--please specify
         *         the containing schema for the projection as part of @ref
         *         tableName and use @ref
         *         GPUdb::createSchema(const CreateSchemaRequest&) const
         *         "GPUdb::createSchema" to create the schema if non-existent]
         *         Name of the schema for the output table. If the schema
         *         provided is non-existent, it will be automatically created.
         *         The default value is ''.
         *     <li>@ref gpudb::create_union_mode "create_union_mode": If @ref
         *         gpudb::create_union_merge_views "merge_views", then this
         *         operation will merge the provided views. All @ref tableNames
         *         must be views from the same underlying base table.
         *         Supported values:
         *         <ul>
         *             <li>@ref gpudb::create_union_union_all
         *                 "create_union_union_all": Retains all rows from the
         *                 specified tables.
         *             <li>@ref gpudb::create_union_union "create_union_union":
         *                 Retains all unique rows from the specified tables
         *                 (synonym for @ref gpudb::create_union_union_distinct
         *                 "union_distinct").
         *             <li>@ref gpudb::create_union_union_distinct
         *                 "create_union_union_distinct": Retains all unique
         *                 rows from the specified tables.
         *             <li>@ref gpudb::create_union_except
         *                 "create_union_except": Retains all unique rows from
         *                 the first table that do not appear in the second
         *                 table (only works on 2 tables).
         *             <li>@ref gpudb::create_union_except_all
         *                 "create_union_except_all": Retains all
         *                 rows(including duplicates) from the first table that
         *                 do not appear in the second table (only works on 2
         *                 tables).
         *             <li>@ref gpudb::create_union_intersect
         *                 "create_union_intersect": Retains all unique rows
         *                 that appear in both of the specified tables (only
         *                 works on 2 tables).
         *             <li>@ref gpudb::create_union_intersect_all
         *                 "create_union_intersect_all": Retains all
         *                 rows(including duplicates) that appear in both of
         *                 the specified tables (only works on 2 tables).
         *             <li>@ref gpudb::create_union_merge_views
         *                 "create_union_merge_views": Merge two or more views
         *                 (or views of views) of the same base data set into a
         *                 new view. If this mode is selected @ref
         *                 inputColumnNames AND @ref outputColumnNames must be
         *                 empty. The resulting view would match the results of
         *                 a SQL OR operation, e.g., if filter 1 creates a view
         *                 using the expression 'x = 20' and filter 2 creates a
         *                 view using the expression 'x &lt;= 10', then the
         *                 merge views operation creates a new view using the
         *                 expression 'x = 20 OR x &lt;= 10'.
         *         </ul>
         *         The default value is @ref gpudb::create_union_union_all
         *         "create_union_union_all".
         *     <li>@ref gpudb::create_union_chunk_size
         *         "create_union_chunk_size": Indicates the number of records
         *         per chunk to be used for this output table.
         *     <li>@ref gpudb::create_union_chunk_column_max_memory
         *         "create_union_chunk_column_max_memory": Indicates the target
         *         maximum data size for each column in a chunk to be used for
         *         this output table.
         *     <li>@ref gpudb::create_union_chunk_max_memory
         *         "create_union_chunk_max_memory": Indicates the target
         *         maximum data size for all columns in a chunk to be used for
         *         this output table.
         *     <li>@ref gpudb::create_union_create_indexes
         *         "create_union_create_indexes": Comma-separated list of
         *         columns on which to create indexes on the output table.  The
         *         columns specified must be present in @ref outputColumnNames.
         *     <li>@ref gpudb::create_union_ttl "create_union_ttl": Sets the <a
         *         href="../../../concepts/ttl/" target="_top">TTL</a> of the
         *         output table specified in @ref tableName.
         *     <li>@ref gpudb::create_union_persist "create_union_persist": If
         *         @ref gpudb::create_union_true "true", then the output table
         *         specified in @ref tableName will be persisted and will not
         *         expire unless a @ref gpudb::create_union_ttl "ttl" is
         *         specified.   If @ref gpudb::create_union_false "false", then
         *         the output table will be an in-memory table and will expire
         *         unless a @ref gpudb::create_union_ttl "ttl" is specified
         *         otherwise.
         *         Supported values:
         *         <ul>
         *             <li>@ref gpudb::create_union_true "create_union_true"
         *             <li>@ref gpudb::create_union_false "create_union_false"
         *         </ul>
         *         The default value is @ref gpudb::create_union_false
         *         "create_union_false".
         *     <li>@ref gpudb::create_union_view_id "create_union_view_id": ID
         *         of view of which this output table is a member. The default
         *         value is ''.
         *     <li>@ref gpudb::create_union_force_replicated
         *         "create_union_force_replicated": If @ref
         *         gpudb::create_union_true "true", then the output table
         *         specified in @ref tableName will be replicated even if the
         *         source tables are not.
         *         Supported values:
         *         <ul>
         *             <li>@ref gpudb::create_union_true "create_union_true"
         *             <li>@ref gpudb::create_union_false "create_union_false"
         *         </ul>
         *         The default value is @ref gpudb::create_union_false
         *         "create_union_false".
         *     <li>@ref gpudb::create_union_strategy_definition
         *         "create_union_strategy_definition": The <a
         *         href="../../../rm/concepts/#tier-strategies"
         *         target="_top">tier strategy</a> for the table and its
         *         columns.
         * </ul>
         * The default value is an empty map.
         */
        std::map<std::string, std::string> options;
    };
} // end namespace gpudb

namespace avro
{
    template<> struct codec_traits<gpudb::CreateUnionRequest>
    {
        static void encode(Encoder& e, const gpudb::CreateUnionRequest& v)
        {
            ::avro::encode(e, v.tableName);
            ::avro::encode(e, v.tableNames);
            ::avro::encode(e, v.inputColumnNames);
            ::avro::encode(e, v.outputColumnNames);
            ::avro::encode(e, v.options);
        }

        static void decode(Decoder& d, gpudb::CreateUnionRequest& v)
        {
            if (::avro::ResolvingDecoder *rd = dynamic_cast< ::avro::ResolvingDecoder*>(&d))
            {
                const std::vector<size_t> fo = rd->fieldOrder();

                for (std::vector<size_t>::const_iterator it = fo.begin(); it != fo.end(); ++it)
                {
                    switch (*it)
                    {
                        case 0:
                            ::avro::decode(d, v.tableName);
                            break;

                        case 1:
                            ::avro::decode(d, v.tableNames);
                            break;

                        case 2:
                            ::avro::decode(d, v.inputColumnNames);
                            break;

                        case 3:
                            ::avro::decode(d, v.outputColumnNames);
                            break;

                        case 4:
                            ::avro::decode(d, v.options);
                            break;

                        default:
                            break;
                    }
                }
            }
            else
            {
                ::avro::decode(d, v.tableName);
                ::avro::decode(d, v.tableNames);
                ::avro::decode(d, v.inputColumnNames);
                ::avro::decode(d, v.outputColumnNames);
                ::avro::decode(d, v.options);
            }
        }
    };
} // end namespace avro

namespace gpudb
{
    /**
     * A set of results returned by @ref
     * GPUdb::createUnion(const CreateUnionRequest&) const
     * "GPUdb::createUnion".
     */
    struct CreateUnionResponse
    {
        /**
         * Constructs a CreateUnionResponse object with default parameters.
         */
        CreateUnionResponse() :
            tableName(std::string()),
            info(std::map<std::string, std::string>())
        {
        }

        /**
         * Value of @ref gpudb::CreateUnionRequest::tableName "tableName".
         */
        std::string tableName;

        /**
         * Additional information.
         * <ul>
         *     <li>@ref gpudb::create_union_count "create_union_count": Number
         *         of records in the final table
         *     <li>@ref gpudb::create_union_qualified_table_name
         *         "create_union_qualified_table_name": The fully qualified
         *         name of the result table (i.e. including the schema)
         * </ul>
         * The default value is an empty map.
         */
        std::map<std::string, std::string> info;
    };
} // end namespace gpudb

namespace avro
{
    template<> struct codec_traits<gpudb::CreateUnionResponse>
    {
        static void encode(Encoder& e, const gpudb::CreateUnionResponse& v)
        {
            ::avro::encode(e, v.tableName);
            ::avro::encode(e, v.info);
        }

        static void decode(Decoder& d, gpudb::CreateUnionResponse& v)
        {
            if (::avro::ResolvingDecoder *rd = dynamic_cast< ::avro::ResolvingDecoder*>(&d))
            {
                const std::vector<size_t> fo = rd->fieldOrder();

                for (std::vector<size_t>::const_iterator it = fo.begin(); it != fo.end(); ++it)
                {
                    switch (*it)
                    {
                        case 0:
                            ::avro::decode(d, v.tableName);
                            break;

                        case 1:
                            ::avro::decode(d, v.info);
                            break;

                        default:
                            break;
                    }
                }
            }
            else
            {
                ::avro::decode(d, v.tableName);
                ::avro::decode(d, v.info);
            }
        }
    };
} // end namespace avro

#endif // __CREATE_UNION_H__

/*
 *  This file was autogenerated by the GPUdb schema processor.
 *
 *  DO NOT EDIT DIRECTLY.
 */
#ifndef __CREATE_TYPE_H__
#define __CREATE_TYPE_H__

namespace gpudb
{

    /**
     * A set of input parameters for {@link
     * #createType(const CreateTypeRequest&) const}.
     * <p>
     * Creates a new type in GPUdb describing the layout or schema of a table.
     * The type definition is a JSON string describing the fields (i.e.
     * columns) of the type. Each field consists of a name and a data type.
     * Supported data types are: double, float, int, long, string, and bytes.
     * In addition one or more properties can be specified for each column
     * which customize the memory usage and query availability of that column.
     * Note that some properties are mutually exclusive--i.e. they cannot be
     * specified for any given column simultaneously.  One example of mutually
     * exclusive properties are @a data and @a store_only.
     * <p>
     * To set a *primary key* on one or more columns include the property
     * 'primary_key' on the desired column_names. If a primary key is specified
     * then GPUdb enforces a uniqueness constraint in that only a single object
     * can exist with a given primary key. When {@link
     * #insertRecordsRaw(const RawInsertRecordsRequest&) const inserting}
     * data into a table with a primary key, depending on the parameters in the
     * request, incoming objects with primary keys that match existing objects
     * will either overwrite (i.e. update) the existing object or will be
     * skipped and not added into the set.
     * <p>
     * Example of a type definition with some of the parameters::
     * <p>
     *         {"type":"record",
     *         "name":"point",
     *         "fields":[{"name":"msg_id","type":"string"},
     *                         {"name":"x","type":"double"},
     *                         {"name":"y","type":"double"},
     *                         {"name":"TIMESTAMP","type":"double"},
     *                         {"name":"source","type":"string"},
     *                         {"name":"group_id","type":"string"},
     *                         {"name":"OBJECT_ID","type":"string"}]
     *         }
     * <p>
     * Properties::
     * <p>
     *         {"group_id":["store_only"],
     *         "msg_id":["store_only","text_search"]
     *         }
     */
    struct CreateTypeRequest
    {

        /**
         * Constructs a CreateTypeRequest object with default parameter values.
         */
        CreateTypeRequest() :
            typeDefinition(std::string()),
            label(std::string()),
            properties(std::map<std::string, std::vector<std::string> >()),
            options(std::map<std::string, std::string>())
        {
        }

        /**
         * Constructs a CreateTypeRequest object with the specified parameters.
         * 
         * @param[in] typeDefinition  a JSON string describing the columns of
         *                            the type to be registered.
         * @param[in] label  A user-defined description string which can be
         *                   used to differentiate between tables and types
         *                   with otherwise identical schemas.
         * @param[in] properties  Each key-value pair specifies the properties
         *                        to use for a given column where the key is
         *                        the column name.  All keys used must be
         *                        relevant column names for the given table.
         *                        Specifying any property overrides the default
         *                        properties for that column (which is based on
         *                        the column's data type).  Default value is an
         *                        empty std::map.
         * @param[in] options  Optional parameters.  Default value is an empty
         *                     std::map.
         * 
         */
        CreateTypeRequest(const std::string& typeDefinition, const std::string& label, const std::map<std::string, std::vector<std::string> >& properties, const std::map<std::string, std::string>& options):
            typeDefinition(typeDefinition),
            label(label),
            properties(properties),
            options(options)
        {
        }

        std::string typeDefinition;
        std::string label;
        std::map<std::string, std::vector<std::string> > properties;
        std::map<std::string, std::string> options;
    };
}

namespace avro
{
    template<> struct codec_traits<gpudb::CreateTypeRequest>
    {
        static void encode(Encoder& e, const gpudb::CreateTypeRequest& v)
        {
            ::avro::encode(e, v.typeDefinition);
            ::avro::encode(e, v.label);
            ::avro::encode(e, v.properties);
            ::avro::encode(e, v.options);
        }

        static void decode(Decoder& d, gpudb::CreateTypeRequest& v)
        {
            if (::avro::ResolvingDecoder *rd = dynamic_cast< ::avro::ResolvingDecoder*>(&d))
            {
                const std::vector<size_t> fo = rd->fieldOrder();

                for (std::vector<size_t>::const_iterator it = fo.begin(); it != fo.end(); ++it)
                {
                    switch (*it)
                    {
                        case 0:
                            ::avro::decode(d, v.typeDefinition);
                            break;

                        case 1:
                            ::avro::decode(d, v.label);
                            break;

                        case 2:
                            ::avro::decode(d, v.properties);
                            break;

                        case 3:
                            ::avro::decode(d, v.options);
                            break;

                        default:
                            break;
                    }
                }
            }
            else
            {
                ::avro::decode(d, v.typeDefinition);
                ::avro::decode(d, v.label);
                ::avro::decode(d, v.properties);
                ::avro::decode(d, v.options);
            }
        }
    };
}

namespace gpudb
{

    /**
     * A set of output parameters for {@link
     * #createType(const CreateTypeRequest&) const}.
     * <p>
     * Creates a new type in GPUdb describing the layout or schema of a table.
     * The type definition is a JSON string describing the fields (i.e.
     * columns) of the type. Each field consists of a name and a data type.
     * Supported data types are: double, float, int, long, string, and bytes.
     * In addition one or more properties can be specified for each column
     * which customize the memory usage and query availability of that column.
     * Note that some properties are mutually exclusive--i.e. they cannot be
     * specified for any given column simultaneously.  One example of mutually
     * exclusive properties are @a data and @a store_only.
     * <p>
     * To set a *primary key* on one or more columns include the property
     * 'primary_key' on the desired column_names. If a primary key is specified
     * then GPUdb enforces a uniqueness constraint in that only a single object
     * can exist with a given primary key. When {@link
     * #insertRecordsRaw(const RawInsertRecordsRequest&) const inserting}
     * data into a table with a primary key, depending on the parameters in the
     * request, incoming objects with primary keys that match existing objects
     * will either overwrite (i.e. update) the existing object or will be
     * skipped and not added into the set.
     * <p>
     * Example of a type definition with some of the parameters::
     * <p>
     *         {"type":"record",
     *         "name":"point",
     *         "fields":[{"name":"msg_id","type":"string"},
     *                         {"name":"x","type":"double"},
     *                         {"name":"y","type":"double"},
     *                         {"name":"TIMESTAMP","type":"double"},
     *                         {"name":"source","type":"string"},
     *                         {"name":"group_id","type":"string"},
     *                         {"name":"OBJECT_ID","type":"string"}]
     *         }
     * <p>
     * Properties::
     * <p>
     *         {"group_id":["store_only"],
     *         "msg_id":["store_only","text_search"]
     *         }
     */
    struct CreateTypeResponse
    {

        /**
         * Constructs a CreateTypeResponse object with default parameter
         * values.
         */
        CreateTypeResponse() :
            typeId(std::string()),
            typeDefinition(std::string()),
            label(std::string()),
            properties(std::map<std::string, std::vector<std::string> >())
        {
        }

        std::string typeId;
        std::string typeDefinition;
        std::string label;
        std::map<std::string, std::vector<std::string> > properties;
    };
}

namespace avro
{
    template<> struct codec_traits<gpudb::CreateTypeResponse>
    {
        static void encode(Encoder& e, const gpudb::CreateTypeResponse& v)
        {
            ::avro::encode(e, v.typeId);
            ::avro::encode(e, v.typeDefinition);
            ::avro::encode(e, v.label);
            ::avro::encode(e, v.properties);
        }

        static void decode(Decoder& d, gpudb::CreateTypeResponse& v)
        {
            if (::avro::ResolvingDecoder *rd = dynamic_cast< ::avro::ResolvingDecoder*>(&d))
            {
                const std::vector<size_t> fo = rd->fieldOrder();

                for (std::vector<size_t>::const_iterator it = fo.begin(); it != fo.end(); ++it)
                {
                    switch (*it)
                    {
                        case 0:
                            ::avro::decode(d, v.typeId);
                            break;

                        case 1:
                            ::avro::decode(d, v.typeDefinition);
                            break;

                        case 2:
                            ::avro::decode(d, v.label);
                            break;

                        case 3:
                            ::avro::decode(d, v.properties);
                            break;

                        default:
                            break;
                    }
                }
            }
            else
            {
                ::avro::decode(d, v.typeId);
                ::avro::decode(d, v.typeDefinition);
                ::avro::decode(d, v.label);
                ::avro::decode(d, v.properties);
            }
        }
    };
}

#endif
/*
 *  This file was autogenerated by the Kinetica schema processor.
 *
 *  DO NOT EDIT DIRECTLY.
 */
#ifndef __SHOW_SCHEMA_H__
#define __SHOW_SCHEMA_H__

namespace gpudb
{
    /**
     * A set of parameters for @ref
     * GPUdb::showSchema(const ShowSchemaRequest&) const "GPUdb::showSchema".
     *
     * Retrieves information about a <a href="../../../concepts/schemas/"
     * target="_top">schema</a> (or all schemas), as specified in @ref
     * schemaName.
     */
    struct ShowSchemaRequest
    {
        /**
         * Constructs a ShowSchemaRequest object with default parameters.
         */
        ShowSchemaRequest() :
            schemaName(std::string()),
            options(std::map<std::string, std::string>())
        {
        }

        /**
         * Constructs a ShowSchemaRequest object with the specified parameters.
         *
         * @param[in] schemaName_  Name of the schema for which to retrieve the
         *                         information. If blank, then info for all
         *                         schemas is returned.
         * @param[in] options_  Optional parameters.
         *                      <ul>
         *                          <li>@ref
         *                              gpudb::show_schema_no_error_if_not_exists
         *                              "show_schema_no_error_if_not_exists":
         *                              If @ref gpudb::show_schema_false
         *                              "false" will return an error if the
         *                              provided @a schemaName_ does not exist.
         *                              If @ref gpudb::show_schema_true "true"
         *                              then it will return an empty result if
         *                              the provided @a schemaName_ does not
         *                              exist.
         *                              Supported values:
         *                              <ul>
         *                                  <li>@ref gpudb::show_schema_true
         *                                      "show_schema_true"
         *                                  <li>@ref gpudb::show_schema_false
         *                                      "show_schema_false"
         *                              </ul>
         *                              The default value is @ref
         *                              gpudb::show_schema_false
         *                              "show_schema_false".
         *                      </ul>
         *                      The default value is an empty map.
         */
        ShowSchemaRequest(const std::string& schemaName_, const std::map<std::string, std::string>& options_):
            schemaName( schemaName_ ),
            options( options_ )
        {
        }

        /**
         * Name of the schema for which to retrieve the information. If blank,
         * then info for all schemas is returned.
         */
        std::string schemaName;

        /**
         * Optional parameters.
         * <ul>
         *     <li>@ref gpudb::show_schema_no_error_if_not_exists
         *         "show_schema_no_error_if_not_exists": If @ref
         *         gpudb::show_schema_false "false" will return an error if the
         *         provided @ref schemaName does not exist. If @ref
         *         gpudb::show_schema_true "true" then it will return an empty
         *         result if the provided @ref schemaName does not exist.
         *         Supported values:
         *         <ul>
         *             <li>@ref gpudb::show_schema_true "show_schema_true"
         *             <li>@ref gpudb::show_schema_false "show_schema_false"
         *         </ul>
         *         The default value is @ref gpudb::show_schema_false
         *         "show_schema_false".
         * </ul>
         * The default value is an empty map.
         */
        std::map<std::string, std::string> options;
    };
} // end namespace gpudb

namespace avro
{
    template<> struct codec_traits<gpudb::ShowSchemaRequest>
    {
        static void encode(Encoder& e, const gpudb::ShowSchemaRequest& v)
        {
            ::avro::encode(e, v.schemaName);
            ::avro::encode(e, v.options);
        }

        static void decode(Decoder& d, gpudb::ShowSchemaRequest& v)
        {
            if (::avro::ResolvingDecoder *rd = dynamic_cast< ::avro::ResolvingDecoder*>(&d))
            {
                const std::vector<size_t> fo = rd->fieldOrder();

                for (std::vector<size_t>::const_iterator it = fo.begin(); it != fo.end(); ++it)
                {
                    switch (*it)
                    {
                        case 0:
                            ::avro::decode(d, v.schemaName);
                            break;

                        case 1:
                            ::avro::decode(d, v.options);
                            break;

                        default:
                            break;
                    }
                }
            }
            else
            {
                ::avro::decode(d, v.schemaName);
                ::avro::decode(d, v.options);
            }
        }
    };
} // end namespace avro

namespace gpudb
{
    /**
     * A set of results returned by @ref
     * GPUdb::showSchema(const ShowSchemaRequest&) const "GPUdb::showSchema".
     */
    struct ShowSchemaResponse
    {
        /**
         * Constructs a ShowSchemaResponse object with default parameters.
         */
        ShowSchemaResponse() :
            schemaName(std::string()),
            schemaNames(std::vector<std::string>()),
            schemaTables(std::vector<std::vector<std::string> >()),
            additionalInfo(std::vector<std::map<std::string, std::string> >()),
            info(std::map<std::string, std::string>())
        {
        }

        /**
         * Value of @ref gpudb::ShowSchemaRequest::schemaName "schemaName".
         */
        std::string schemaName;

        /**
         * A list of all schema names for which information is returned
         */
        std::vector<std::string> schemaNames;

        /**
         * An array of arrays containing a list of tables in each of the
         * respective @ref schemaNames.
         */
        std::vector<std::vector<std::string> > schemaTables;

        /**
         * Additional information about the respective tables in @ref
         * schemaNames.
         */
        std::vector<std::map<std::string, std::string> > additionalInfo;

        /**
         * Additional information.
         */
        std::map<std::string, std::string> info;
    };
} // end namespace gpudb

namespace avro
{
    template<> struct codec_traits<gpudb::ShowSchemaResponse>
    {
        static void encode(Encoder& e, const gpudb::ShowSchemaResponse& v)
        {
            ::avro::encode(e, v.schemaName);
            ::avro::encode(e, v.schemaNames);
            ::avro::encode(e, v.schemaTables);
            ::avro::encode(e, v.additionalInfo);
            ::avro::encode(e, v.info);
        }

        static void decode(Decoder& d, gpudb::ShowSchemaResponse& v)
        {
            if (::avro::ResolvingDecoder *rd = dynamic_cast< ::avro::ResolvingDecoder*>(&d))
            {
                const std::vector<size_t> fo = rd->fieldOrder();

                for (std::vector<size_t>::const_iterator it = fo.begin(); it != fo.end(); ++it)
                {
                    switch (*it)
                    {
                        case 0:
                            ::avro::decode(d, v.schemaName);
                            break;

                        case 1:
                            ::avro::decode(d, v.schemaNames);
                            break;

                        case 2:
                            ::avro::decode(d, v.schemaTables);
                            break;

                        case 3:
                            ::avro::decode(d, v.additionalInfo);
                            break;

                        case 4:
                            ::avro::decode(d, v.info);
                            break;

                        default:
                            break;
                    }
                }
            }
            else
            {
                ::avro::decode(d, v.schemaName);
                ::avro::decode(d, v.schemaNames);
                ::avro::decode(d, v.schemaTables);
                ::avro::decode(d, v.additionalInfo);
                ::avro::decode(d, v.info);
            }
        }
    };
} // end namespace avro

#endif // __SHOW_SCHEMA_H__

/*
 *  This file was autogenerated by the GPUdb schema processor.
 *
 *  DO NOT EDIT DIRECTLY.
 */
#ifndef __CREATE_STATE_TABLE_H__
#define __CREATE_STATE_TABLE_H__

namespace gpudb
{

    /**
     * A set of input parameters for {@link
     * #createStateTable(const CreateStateTableRequest&) const}.
     * <p>
     * @private
     */
    struct CreateStateTableRequest
    {

        /**
         * @private
         * Constructs a CreateStateTableRequest object with default parameter
         * values.
         */
        CreateStateTableRequest() :
            tableName(std::string()),
            inputTableName(std::string()),
            initTableName(std::string()),
            options(std::map<std::string, std::string>())
        {
        }

        /**
         * @private
         * Constructs a CreateStateTableRequest object with the specified
         * parameters.
         * 
         * @param[in] tableName_
         * @param[in] inputTableName_
         * @param[in] initTableName_
         * @param[in] options_
         * 
         */
        CreateStateTableRequest(const std::string& tableName_, const std::string& inputTableName_, const std::string& initTableName_, const std::map<std::string, std::string>& options_):
            tableName( tableName_ ),
            inputTableName( inputTableName_ ),
            initTableName( initTableName_ ),
            options( options_ )
        {
        }

        std::string tableName;
        std::string inputTableName;
        std::string initTableName;
        std::map<std::string, std::string> options;
    };
}

    /**
     * @private
     */

namespace avro
{
    template<> struct codec_traits<gpudb::CreateStateTableRequest>
    {
        static void encode(Encoder& e, const gpudb::CreateStateTableRequest& v)
        {
            ::avro::encode(e, v.tableName);
            ::avro::encode(e, v.inputTableName);
            ::avro::encode(e, v.initTableName);
            ::avro::encode(e, v.options);
        }

        static void decode(Decoder& d, gpudb::CreateStateTableRequest& v)
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
                            ::avro::decode(d, v.inputTableName);
                            break;

                        case 2:
                            ::avro::decode(d, v.initTableName);
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
                ::avro::decode(d, v.tableName);
                ::avro::decode(d, v.inputTableName);
                ::avro::decode(d, v.initTableName);
                ::avro::decode(d, v.options);
            }
        }
    };
}

namespace gpudb
{

    /**
     * A set of output parameters for {@link
     * #createStateTable(const CreateStateTableRequest&) const}.
     * <p>
     * @private
     */
    struct CreateStateTableResponse
    {

        /**
         * @private
         * Constructs a CreateStateTableResponse object with default parameter
         * values.
         */
        CreateStateTableResponse() :
            tableName(std::string()),
            info(std::map<std::string, std::string>())
        {
        }

        std::string tableName;
        std::map<std::string, std::string> info;
    };
}

    /**
     * @private
     */

namespace avro
{
    template<> struct codec_traits<gpudb::CreateStateTableResponse>
    {
        static void encode(Encoder& e, const gpudb::CreateStateTableResponse& v)
        {
            ::avro::encode(e, v.tableName);
            ::avro::encode(e, v.info);
        }

        static void decode(Decoder& d, gpudb::CreateStateTableResponse& v)
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
}

#endif
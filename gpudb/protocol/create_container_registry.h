/*
 *  This file was autogenerated by the GPUdb schema processor.
 *
 *  DO NOT EDIT DIRECTLY.
 */
#ifndef __CREATE_CONTAINER_REGISTRY_H__
#define __CREATE_CONTAINER_REGISTRY_H__

namespace gpudb
{

    /**
     * A set of input parameters for {@link
     * #createContainerRegistry(const CreateContainerRegistryRequest&) const}.
     * <p>
     * @private
     */
    struct CreateContainerRegistryRequest
    {

        /**
         * @private
         * Constructs a CreateContainerRegistryRequest object with default
         * parameter values.
         */
        CreateContainerRegistryRequest() :
            registryName(std::string()),
            uri(std::string()),
            credential(std::string()),
            options(std::map<std::string, std::string>())
        {
        }

        /**
         * @private
         * Constructs a CreateContainerRegistryRequest object with the
         * specified parameters.
         * 
         * @param[in] registryName_
         * @param[in] uri_
         * @param[in] credential_
         * @param[in] options_
         *                      <ul>
         *                              <li>
         *                      gpudb::create_container_registry_user_name:
         *                      The default value is ''.
         *                              <li>
         *                      gpudb::create_container_registry_password:
         *                      The default value is ''.
         *                              <li>
         *                      gpudb::create_container_registry_email:   The
         *                      default value is ''.
         *                      </ul>
         * 
         */
        CreateContainerRegistryRequest(const std::string& registryName_, const std::string& uri_, const std::string& credential_, const std::map<std::string, std::string>& options_):
            registryName( registryName_ ),
            uri( uri_ ),
            credential( credential_ ),
            options( options_ )
        {
        }

        std::string registryName;
        std::string uri;
        std::string credential;
        std::map<std::string, std::string> options;
    };
}

    /**
     * @private
     */

namespace avro
{
    template<> struct codec_traits<gpudb::CreateContainerRegistryRequest>
    {
        static void encode(Encoder& e, const gpudb::CreateContainerRegistryRequest& v)
        {
            ::avro::encode(e, v.registryName);
            ::avro::encode(e, v.uri);
            ::avro::encode(e, v.credential);
            ::avro::encode(e, v.options);
        }

        static void decode(Decoder& d, gpudb::CreateContainerRegistryRequest& v)
        {
            if (::avro::ResolvingDecoder *rd = dynamic_cast< ::avro::ResolvingDecoder*>(&d))
            {
                const std::vector<size_t> fo = rd->fieldOrder();

                for (std::vector<size_t>::const_iterator it = fo.begin(); it != fo.end(); ++it)
                {
                    switch (*it)
                    {
                        case 0:
                            ::avro::decode(d, v.registryName);
                            break;

                        case 1:
                            ::avro::decode(d, v.uri);
                            break;

                        case 2:
                            ::avro::decode(d, v.credential);
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
                ::avro::decode(d, v.registryName);
                ::avro::decode(d, v.uri);
                ::avro::decode(d, v.credential);
                ::avro::decode(d, v.options);
            }
        }
    };
}

namespace gpudb
{

    /**
     * A set of output parameters for {@link
     * #createContainerRegistry(const CreateContainerRegistryRequest&) const}.
     * <p>
     * @private
     */
    struct CreateContainerRegistryResponse
    {

        /**
         * @private
         * Constructs a CreateContainerRegistryResponse object with default
         * parameter values.
         */
        CreateContainerRegistryResponse() :
            registryName(std::string()),
            entityId(int32_t()),
            info(std::map<std::string, std::string>())
        {
        }

        std::string registryName;
        int32_t entityId;
        std::map<std::string, std::string> info;
    };
}

    /**
     * @private
     */

namespace avro
{
    template<> struct codec_traits<gpudb::CreateContainerRegistryResponse>
    {
        static void encode(Encoder& e, const gpudb::CreateContainerRegistryResponse& v)
        {
            ::avro::encode(e, v.registryName);
            ::avro::encode(e, v.entityId);
            ::avro::encode(e, v.info);
        }

        static void decode(Decoder& d, gpudb::CreateContainerRegistryResponse& v)
        {
            if (::avro::ResolvingDecoder *rd = dynamic_cast< ::avro::ResolvingDecoder*>(&d))
            {
                const std::vector<size_t> fo = rd->fieldOrder();

                for (std::vector<size_t>::const_iterator it = fo.begin(); it != fo.end(); ++it)
                {
                    switch (*it)
                    {
                        case 0:
                            ::avro::decode(d, v.registryName);
                            break;

                        case 1:
                            ::avro::decode(d, v.entityId);
                            break;

                        case 2:
                            ::avro::decode(d, v.info);
                            break;

                        default:
                            break;
                    }
                }
            }
            else
            {
                ::avro::decode(d, v.registryName);
                ::avro::decode(d, v.entityId);
                ::avro::decode(d, v.info);
            }
        }
    };
}

#endif
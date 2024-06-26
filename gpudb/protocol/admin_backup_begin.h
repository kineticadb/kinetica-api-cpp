/*
 *  This file was autogenerated by the Kinetica schema processor.
 *
 *  DO NOT EDIT DIRECTLY.
 */
#ifndef __ADMIN_BACKUP_BEGIN_H__
#define __ADMIN_BACKUP_BEGIN_H__

namespace gpudb
{
    /**
     * A set of parameters for @ref
     * GPUdb::adminBackupBegin(const AdminBackupBeginRequest&) const
     * "GPUdb::adminBackupBegin".
     *
     * Prepares the system for a backup by closing all open file handles after
     * allowing current active jobs to complete. When the database is in backup
     * mode, queries that result in a disk write operation will be blocked
     * until backup mode has been completed by using @ref
     * GPUdb::adminBackupEnd(const AdminBackupEndRequest&) const
     * "GPUdb::adminBackupEnd".
     */
    struct AdminBackupBeginRequest
    {
        /**
         * Constructs an AdminBackupBeginRequest object with default
         * parameters.
         */
        AdminBackupBeginRequest() :
            options(std::map<std::string, std::string>())
        {
        }

        /**
         * Constructs an AdminBackupBeginRequest object with the specified
         * parameters.
         *
         * @param[in] options_  Optional parameters. The default value is an
         *                      empty map.
         */
        AdminBackupBeginRequest(const std::map<std::string, std::string>& options_):
            options( options_ )
        {
        }

        /**
         * Optional parameters. The default value is an empty map.
         */
        std::map<std::string, std::string> options;
    };
} // end namespace gpudb

namespace avro
{
    template<> struct codec_traits<gpudb::AdminBackupBeginRequest>
    {
        static void encode(Encoder& e, const gpudb::AdminBackupBeginRequest& v)
        {
            ::avro::encode(e, v.options);
        }

        static void decode(Decoder& d, gpudb::AdminBackupBeginRequest& v)
        {
            if (::avro::ResolvingDecoder *rd = dynamic_cast< ::avro::ResolvingDecoder*>(&d))
            {
                const std::vector<size_t> fo = rd->fieldOrder();

                for (std::vector<size_t>::const_iterator it = fo.begin(); it != fo.end(); ++it)
                {
                    switch (*it)
                    {
                        case 0:
                            ::avro::decode(d, v.options);
                            break;

                        default:
                            break;
                    }
                }
            }
            else
            {
                ::avro::decode(d, v.options);
            }
        }
    };
} // end namespace avro

namespace gpudb
{
    /**
     * A set of results returned by @ref
     * GPUdb::adminBackupBegin(const AdminBackupBeginRequest&) const
     * "GPUdb::adminBackupBegin".
     */
    struct AdminBackupBeginResponse
    {
        /**
         * Constructs an AdminBackupBeginResponse object with default
         * parameters.
         */
        AdminBackupBeginResponse() :
            info(std::map<std::string, std::string>())
        {
        }

        /**
         * Additional information.
         */
        std::map<std::string, std::string> info;
    };
} // end namespace gpudb

namespace avro
{
    template<> struct codec_traits<gpudb::AdminBackupBeginResponse>
    {
        static void encode(Encoder& e, const gpudb::AdminBackupBeginResponse& v)
        {
            ::avro::encode(e, v.info);
        }

        static void decode(Decoder& d, gpudb::AdminBackupBeginResponse& v)
        {
            if (::avro::ResolvingDecoder *rd = dynamic_cast< ::avro::ResolvingDecoder*>(&d))
            {
                const std::vector<size_t> fo = rd->fieldOrder();

                for (std::vector<size_t>::const_iterator it = fo.begin(); it != fo.end(); ++it)
                {
                    switch (*it)
                    {
                        case 0:
                            ::avro::decode(d, v.info);
                            break;

                        default:
                            break;
                    }
                }
            }
            else
            {
                ::avro::decode(d, v.info);
            }
        }
    };
} // end namespace avro

#endif // __ADMIN_BACKUP_BEGIN_H__

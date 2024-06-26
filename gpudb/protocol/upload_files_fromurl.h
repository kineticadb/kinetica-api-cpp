/*
 *  This file was autogenerated by the Kinetica schema processor.
 *
 *  DO NOT EDIT DIRECTLY.
 */
#ifndef __UPLOAD_FILES_FROMURL_H__
#define __UPLOAD_FILES_FROMURL_H__

namespace gpudb
{
    /**
     * A set of parameters for @ref
     * GPUdb::uploadFilesFromurl(const UploadFilesFromurlRequest&) const
     * "GPUdb::uploadFilesFromurl".
     *
     * Uploads one or more files to <a href="../../../tools/kifs/"
     * target="_top">KiFS</a>.
     *
     * Each file path must reside in a top-level KiFS directory, i.e. one of
     * the directories listed in @ref
     * GPUdb::showDirectories(const ShowDirectoriesRequest&) const
     * "GPUdb::showDirectories". The user must have write permission on the
     * directory. Nested directories are permitted in file name paths.
     * Directories are deliniated with the directory separator of '/'.  For
     * example, given the file path '/a/b/c/d.txt', 'a' must be a KiFS
     * directory.
     *
     * These characters are allowed in file name paths: letters, numbers,
     * spaces, the path delimiter of '/', and the characters: '.' '-' ':' '['
     * ']' '(' ')' '#' '='.
     */
    struct UploadFilesFromurlRequest
    {
        /**
         * Constructs an UploadFilesFromurlRequest object with default
         * parameters.
         */
        UploadFilesFromurlRequest() :
            fileNames(std::vector<std::string>()),
            urls(std::vector<std::string>()),
            options(std::map<std::string, std::string>())
        {
        }

        /**
         * Constructs an UploadFilesFromurlRequest object with the specified
         * parameters.
         *
         * @param[in] fileNames_  An array of full file name paths to be used
         *                        for the files uploaded to KiFS. File names
         *                        may have any number of nested directories in
         *                        their paths, but the top-level directory must
         *                        be an existing KiFS directory. Each file must
         *                        reside in or under a top-level directory. A
         *                        full file name path cannot be larger than
         *                        1024 characters.
         * @param[in] urls_  List of URLs to upload, for each respective file
         *                   in @a fileNames_.
         * @param[in] options_  Optional parameters. The default value is an
         *                      empty map.
         */
        UploadFilesFromurlRequest(const std::vector<std::string>& fileNames_, const std::vector<std::string>& urls_, const std::map<std::string, std::string>& options_):
            fileNames( fileNames_ ),
            urls( urls_ ),
            options( options_ )
        {
        }

        /**
         * An array of full file name paths to be used for the files uploaded
         * to KiFS. File names may have any number of nested directories in
         * their paths, but the top-level directory must be an existing KiFS
         * directory. Each file must reside in or under a top-level directory.
         * A full file name path cannot be larger than 1024 characters.
         */
        std::vector<std::string> fileNames;

        /**
         * List of URLs to upload, for each respective file in @ref fileNames.
         */
        std::vector<std::string> urls;

        /**
         * Optional parameters. The default value is an empty map.
         */
        std::map<std::string, std::string> options;
    };
} // end namespace gpudb

namespace avro
{
    template<> struct codec_traits<gpudb::UploadFilesFromurlRequest>
    {
        static void encode(Encoder& e, const gpudb::UploadFilesFromurlRequest& v)
        {
            ::avro::encode(e, v.fileNames);
            ::avro::encode(e, v.urls);
            ::avro::encode(e, v.options);
        }

        static void decode(Decoder& d, gpudb::UploadFilesFromurlRequest& v)
        {
            if (::avro::ResolvingDecoder *rd = dynamic_cast< ::avro::ResolvingDecoder*>(&d))
            {
                const std::vector<size_t> fo = rd->fieldOrder();

                for (std::vector<size_t>::const_iterator it = fo.begin(); it != fo.end(); ++it)
                {
                    switch (*it)
                    {
                        case 0:
                            ::avro::decode(d, v.fileNames);
                            break;

                        case 1:
                            ::avro::decode(d, v.urls);
                            break;

                        case 2:
                            ::avro::decode(d, v.options);
                            break;

                        default:
                            break;
                    }
                }
            }
            else
            {
                ::avro::decode(d, v.fileNames);
                ::avro::decode(d, v.urls);
                ::avro::decode(d, v.options);
            }
        }
    };
} // end namespace avro

namespace gpudb
{
    /**
     * A set of results returned by @ref
     * GPUdb::uploadFilesFromurl(const UploadFilesFromurlRequest&) const
     * "GPUdb::uploadFilesFromurl".
     */
    struct UploadFilesFromurlResponse
    {
        /**
         * Constructs an UploadFilesFromurlResponse object with default
         * parameters.
         */
        UploadFilesFromurlResponse() :
            successfulFileNames(std::vector<std::string>()),
            successfulUrls(std::vector<std::string>()),
            info(std::map<std::string, std::string>())
        {
        }

        /**
         * List of @ref gpudb::UploadFilesFromurlRequest::fileNames "fileNames"
         * that were successfully uploaded.
         */
        std::vector<std::string> successfulFileNames;

        /**
         * List of @ref gpudb::UploadFilesFromurlRequest::urls "urls" that were
         * successfully uploaded.
         */
        std::vector<std::string> successfulUrls;

        /**
         * Additional information.
         */
        std::map<std::string, std::string> info;
    };
} // end namespace gpudb

namespace avro
{
    template<> struct codec_traits<gpudb::UploadFilesFromurlResponse>
    {
        static void encode(Encoder& e, const gpudb::UploadFilesFromurlResponse& v)
        {
            ::avro::encode(e, v.successfulFileNames);
            ::avro::encode(e, v.successfulUrls);
            ::avro::encode(e, v.info);
        }

        static void decode(Decoder& d, gpudb::UploadFilesFromurlResponse& v)
        {
            if (::avro::ResolvingDecoder *rd = dynamic_cast< ::avro::ResolvingDecoder*>(&d))
            {
                const std::vector<size_t> fo = rd->fieldOrder();

                for (std::vector<size_t>::const_iterator it = fo.begin(); it != fo.end(); ++it)
                {
                    switch (*it)
                    {
                        case 0:
                            ::avro::decode(d, v.successfulFileNames);
                            break;

                        case 1:
                            ::avro::decode(d, v.successfulUrls);
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
                ::avro::decode(d, v.successfulFileNames);
                ::avro::decode(d, v.successfulUrls);
                ::avro::decode(d, v.info);
            }
        }
    };
} // end namespace avro

#endif // __UPLOAD_FILES_FROMURL_H__

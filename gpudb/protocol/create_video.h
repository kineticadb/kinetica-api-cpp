/*
 *  This file was autogenerated by the GPUdb schema processor.
 *
 *  DO NOT EDIT DIRECTLY.
 */
#ifndef __CREATE_VIDEO_H__
#define __CREATE_VIDEO_H__

namespace gpudb
{

    /**
     * A set of input parameters for {@link
     * #createVideo(const CreateVideoRequest&) const}.
     * <p>
     * Creates a job to generate a sequence of raster images that visualize
     * data over a specified time.
     */
    struct CreateVideoRequest
    {

        /**
         * Constructs a CreateVideoRequest object with default parameter
         * values.
         */
        CreateVideoRequest() :
            attribute(std::string()),
            begin(std::string()),
            durationSeconds(double()),
            end(std::string()),
            framesPerSecond(double()),
            style(std::string()),
            path(std::string()),
            styleParameters(std::string()),
            options(std::map<std::string, std::string>())
        {
        }

        /**
         * Constructs a CreateVideoRequest object with the specified
         * parameters.
         * 
         * @param[in] attribute_  The animated attribute to map to the video's
         *                        frames. Must be present in the LAYERS
         *                        specified for the visualization. This is
         *                        often a time-related field but may be any
         *                        numeric type.
         * @param[in] begin_  The start point for the video. Accepts an
         *                    expression evaluable over the @a attribute.
         * @param[in] durationSeconds_  Seconds of video to produce
         * @param[in] end_  The end point for the video. Accepts an expression
         *                  evaluable over the @a attribute.
         * @param[in] framesPerSecond_  The presentation frame rate of the
         *                              encoded video in frames per second.
         * @param[in] style_  The name of the visualize mode; should correspond
         *                    to the schema used for the @a styleParameters
         *                    field.
         *                    <ul>
         *                            <li> gpudb::create_video_chart
         *                            <li> gpudb::create_video_raster
         *                            <li> gpudb::create_video_classbreak
         *                            <li> gpudb::create_video_contour
         *                            <li> gpudb::create_video_heatmap
         *                            <li> gpudb::create_video_labels
         *                    </ul>
         * @param[in] path_  Fully-qualified <a href="../../../tools/kifs/"
         *                   target="_top">KiFS</a> path.  Write access is
         *                   required. A file must not exist at that path,
         *                   unless @a replace_if_exists is @a true.
         * @param[in] styleParameters_  A string containing the JSON-encoded
         *                              visualize request.  Must correspond to
         *                              the visualize mode specified in the @a
         *                              style field.
         * @param[in] options_  Optional parameters.
         *                      <ul>
         *                              <li> gpudb::create_video_ttl: Sets the
         *                      <a href="../../../concepts/ttl/"
         *                      target="_top">TTL</a> of the video.
         *                              <li> gpudb::create_video_window:
         *                      Specified using the data-type corresponding to
         *                      the @a attribute. For a window of size W, a
         *                      video frame rendered for time t will visualize
         *                      data in the interval [t-W,t]. The minimum
         *                      window size is the interval between successive
         *                      frames.  The minimum value is the default.  If
         *                      a value less than the minimum value is
         *                      specified, it is replaced with the minimum
         *                      window size.  Larger values will make changes
         *                      throughout the video appear more smooth while
         *                      smaller values will capture fast variations in
         *                      the data.
         *                              <li>
         *                      gpudb::create_video_no_error_if_exists: If @a
         *                      true, does not return an error if the video
         *                      already exists.  Ignored if @a
         *                      replace_if_exists is @a true.
         *                      <ul>
         *                              <li> gpudb::create_video_false
         *                              <li> gpudb::create_video_true
         *                      </ul>
         *                      The default value is gpudb::create_video_false.
         *                              <li>
         *                      gpudb::create_video_replace_if_exists: If @a
         *                      true, deletes any existing video with the same
         *                      path before creating a new video.
         *                      <ul>
         *                              <li> gpudb::create_video_false
         *                              <li> gpudb::create_video_true
         *                      </ul>
         *                      The default value is gpudb::create_video_false.
         *                      </ul>
         * 
         */
        CreateVideoRequest(const std::string& attribute_, const std::string& begin_, const double durationSeconds_, const std::string& end_, const double framesPerSecond_, const std::string& style_, const std::string& path_, const std::string& styleParameters_, const std::map<std::string, std::string>& options_):
            attribute( attribute_ ),
            begin( begin_ ),
            durationSeconds( durationSeconds_ ),
            end( end_ ),
            framesPerSecond( framesPerSecond_ ),
            style( style_ ),
            path( path_ ),
            styleParameters( styleParameters_ ),
            options( options_ )
        {
        }

        std::string attribute;
        std::string begin;
        double durationSeconds;
        std::string end;
        double framesPerSecond;
        std::string style;
        std::string path;
        std::string styleParameters;
        std::map<std::string, std::string> options;
    };
}

namespace avro
{
    template<> struct codec_traits<gpudb::CreateVideoRequest>
    {
        static void encode(Encoder& e, const gpudb::CreateVideoRequest& v)
        {
            ::avro::encode(e, v.attribute);
            ::avro::encode(e, v.begin);
            ::avro::encode(e, v.durationSeconds);
            ::avro::encode(e, v.end);
            ::avro::encode(e, v.framesPerSecond);
            ::avro::encode(e, v.style);
            ::avro::encode(e, v.path);
            ::avro::encode(e, v.styleParameters);
            ::avro::encode(e, v.options);
        }

        static void decode(Decoder& d, gpudb::CreateVideoRequest& v)
        {
            if (::avro::ResolvingDecoder *rd = dynamic_cast< ::avro::ResolvingDecoder*>(&d))
            {
                const std::vector<size_t> fo = rd->fieldOrder();

                for (std::vector<size_t>::const_iterator it = fo.begin(); it != fo.end(); ++it)
                {
                    switch (*it)
                    {
                        case 0:
                            ::avro::decode(d, v.attribute);
                            break;

                        case 1:
                            ::avro::decode(d, v.begin);
                            break;

                        case 2:
                            ::avro::decode(d, v.durationSeconds);
                            break;

                        case 3:
                            ::avro::decode(d, v.end);
                            break;

                        case 4:
                            ::avro::decode(d, v.framesPerSecond);
                            break;

                        case 5:
                            ::avro::decode(d, v.style);
                            break;

                        case 6:
                            ::avro::decode(d, v.path);
                            break;

                        case 7:
                            ::avro::decode(d, v.styleParameters);
                            break;

                        case 8:
                            ::avro::decode(d, v.options);
                            break;

                        default:
                            break;
                    }
                }
            }
            else
            {
                ::avro::decode(d, v.attribute);
                ::avro::decode(d, v.begin);
                ::avro::decode(d, v.durationSeconds);
                ::avro::decode(d, v.end);
                ::avro::decode(d, v.framesPerSecond);
                ::avro::decode(d, v.style);
                ::avro::decode(d, v.path);
                ::avro::decode(d, v.styleParameters);
                ::avro::decode(d, v.options);
            }
        }
    };
}

namespace gpudb
{

    /**
     * A set of output parameters for {@link
     * #createVideo(const CreateVideoRequest&) const}.
     * <p>
     * Creates a job to generate a sequence of raster images that visualize
     * data over a specified time.
     */
    struct CreateVideoResponse
    {

        /**
         * Constructs a CreateVideoResponse object with default parameter
         * values.
         */
        CreateVideoResponse() :
            jobId(int64_t()),
            path(std::string()),
            info(std::map<std::string, std::string>())
        {
        }

        int64_t jobId;
        std::string path;
        std::map<std::string, std::string> info;
    };
}

namespace avro
{
    template<> struct codec_traits<gpudb::CreateVideoResponse>
    {
        static void encode(Encoder& e, const gpudb::CreateVideoResponse& v)
        {
            ::avro::encode(e, v.jobId);
            ::avro::encode(e, v.path);
            ::avro::encode(e, v.info);
        }

        static void decode(Decoder& d, gpudb::CreateVideoResponse& v)
        {
            if (::avro::ResolvingDecoder *rd = dynamic_cast< ::avro::ResolvingDecoder*>(&d))
            {
                const std::vector<size_t> fo = rd->fieldOrder();

                for (std::vector<size_t>::const_iterator it = fo.begin(); it != fo.end(); ++it)
                {
                    switch (*it)
                    {
                        case 0:
                            ::avro::decode(d, v.jobId);
                            break;

                        case 1:
                            ::avro::decode(d, v.path);
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
                ::avro::decode(d, v.jobId);
                ::avro::decode(d, v.path);
                ::avro::decode(d, v.info);
            }
        }
    };
}

#endif
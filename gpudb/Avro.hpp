#ifndef __GPUDB__AVRO_HPP__
#define __GPUDB__AVRO_HPP__

#include "gpudb/GPUdbException.hpp"
#include "gpudb/Type.hpp"

#include <avro/Compiler.hh>
#include <avro/Generic.hh>
#include <avro/Schema.hh>
#include <avro/Specific.hh>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/make_shared.hpp>

namespace gpudb
{
    namespace avro
    {
        class Executor
        {
            public:
                virtual void run(const std::vector<boost::function0<void> >& tasks) const;
                virtual ~Executor();
        };

        typedef boost::shared_ptr<Executor> ExecutorPtr;

        template<typename T> void decode(T* objects, const std::vector<uint8_t>* encodedObjects, const size_t count)
        {
            if (count == 0)
            {
                return;
            }

            try
            {
                ::avro::DecoderPtr decoder = ::avro::binaryDecoder();

                for (size_t i = 0; i < count; ++i)
                {
                    std::auto_ptr< ::avro::InputStream> inputStream = ::avro::memoryInputStream(&encodedObjects[i][0], encodedObjects[i].size());
                    decoder->init(*inputStream);
                    ::avro::decode(*decoder, objects[i]);
                }
            }
            catch (const std::exception& ex)
            {
                throw GPUdbException("Could not decode object: " + std::string(ex.what()));
            }
        }

        template<typename T> void decode(T* objects, const std::vector<uint8_t>* encodedObjects, const size_t count, const size_t threadCount, const ExecutorPtr executor = ExecutorPtr())
        {
            if (threadCount == 1 || count <= 1)
            {
                decode(objects, encodedObjects, count);
                return;
            }

            if (threadCount < 1)
            {
                throw std::invalid_argument("Thread count must be greater than zero.");
            }

            size_t partitionSize = count / threadCount;
            size_t partitionExtras = count % threadCount;
            std::vector<boost::function0<void> > tasks;
            tasks.reserve(threadCount);
            void (&decodePtr)(T*, const std::vector<uint8_t>*, size_t) = decode;

            for (size_t i = 0; i < threadCount; ++i)
            {
                size_t partitionStart = i * partitionSize + std::min(i, partitionExtras);
                size_t partitionEnd = (i + 1) * partitionSize + std::min(i + 1, partitionExtras);
                tasks.push_back(boost::bind(decodePtr, &objects[partitionStart], &encodedObjects[partitionStart], partitionEnd - partitionStart));
            }

            if (executor)
            {
                executor->run(tasks);
            }
            else
            {
                Executor().run(tasks);
            }
        }

        template<typename T> void decode(T& object, const std::vector<uint8_t>& encodedObject)
        {
            decode(&object, &encodedObject, 1);
        }

        template<typename T> void decode(std::vector<T>& objects, const std::vector<std::vector<uint8_t> >& encodedObjects, const size_t threadCount = 1, const ExecutorPtr executor = ExecutorPtr())
        {
            objects.resize(encodedObjects.size(), T());
            decode<T>(&objects[0], &encodedObjects[0], encodedObjects.size(), threadCount, executor);
        }

        template<typename T> std::vector<T>& decode(const ::avro::ValidSchema& schema, std::vector<T>& objects, const std::vector<std::vector<uint8_t> >& encodedObjects, const size_t threadCount = 1, const ExecutorPtr executor = ExecutorPtr())
        {
            objects.resize(encodedObjects.size(), T(schema));
            decode(&objects[0], &encodedObjects[0], encodedObjects.size(), threadCount, executor);
        }

        template<typename T> std::vector<T>& decode(const Type& type, std::vector<T>& objects, const std::vector<std::vector<uint8_t> >& encodedObjects, const size_t threadCount = 1, const ExecutorPtr executor = ExecutorPtr())
        {
            objects.resize(encodedObjects.size(), T(type));
            decode(&objects[0], &encodedObjects[0], encodedObjects.size(), threadCount, executor);
        }

        template<typename T> void decodeAny(boost::any* objects, const std::vector<uint8_t>* encodedObjects, const size_t count)
        {
            if (count == 0)
            {
                return;
            }

            try
            {
                ::avro::DecoderPtr decoder = ::avro::binaryDecoder();

                for (size_t i = 0; i < count; ++i)
                {
                    std::auto_ptr< ::avro::InputStream> inputStream = ::avro::memoryInputStream(&encodedObjects[i][0], encodedObjects[i].size());
                    decoder->init(*inputStream);
                    ::avro::decode(*decoder, boost::any_cast<T&>(objects[i]));
                }
            }
            catch (const std::exception& ex)
            {
                throw GPUdbException("Could not decode object: " + std::string(ex.what()));
            }
        }

        template<typename T> void decodeAny(boost::any* objects, const std::vector<uint8_t>* encodedObjects, const size_t count, const size_t threadCount, const ExecutorPtr& executor = ExecutorPtr())
        {
            if (threadCount == 1 || count <= 1)
            {
                decodeAny<T>(objects, encodedObjects, count);
                return;
            }

            if (threadCount < 1)
            {
                throw std::invalid_argument("Thread count must be greater than zero.");
            }

            size_t partitionSize = count / threadCount;
            size_t partitionExtras = count % threadCount;
            std::vector<boost::function0<void> > tasks;
            tasks.reserve(threadCount);
            void (&decodePtr)(boost::any*, const std::vector<uint8_t>*, const size_t) = decodeAny<T>;

            for (size_t i = 0; i < threadCount; ++i)
            {
                size_t partitionStart = i * partitionSize + std::min(i, partitionExtras);
                size_t partitionEnd = (i + 1) * partitionSize + std::min(i + 1, partitionExtras);
                tasks.push_back(boost::bind(decodePtr, &objects[partitionStart], &encodedObjects[partitionStart], partitionEnd - partitionStart));
            }

            if (executor)
            {
                executor->run(tasks);
            }
            else
            {
                Executor().run(tasks);
            }
        }

        template<typename T> void encode(std::vector<uint8_t>* encodedObjects, const T* objects, const size_t count)
        {
            if (count == 0)
            {
                return;
            }

            try
            {
                ::avro::EncoderPtr encoder = ::avro::binaryEncoder();

                for (size_t i = 0; i < count; ++i)
                {
                    std::auto_ptr< ::avro::OutputStream> outputStream = ::avro::memoryOutputStream();
                    encoder->init(*outputStream);
                    ::avro::encode(*encoder, objects[i]);
                    encoder->flush();
                    int size = outputStream->byteCount();
                    std::auto_ptr< ::avro::InputStream> inputStream = ::avro::memoryInputStream(*outputStream);
                    ::avro::StreamReader streamReader(*inputStream);
                    encodedObjects[i].resize(size, 0);
                    streamReader.readBytes(&encodedObjects[i][0], size);
                }
            }
            catch (const std::exception& ex)
            {
                throw GPUdbException("Could not encode object: " + std::string(ex.what()));
            }
        }

        template<typename T> void encode(std::vector<uint8_t>* encodedObjects, const T* objects, const size_t count, const size_t threadCount, const ExecutorPtr executor = ExecutorPtr())
        {
            if (threadCount == 1 || count <= 1)
            {
                encode(encodedObjects, objects, count);
                return;
            }

            if (threadCount < 1)
            {
                throw std::invalid_argument("Thread count must be greater than zero.");
            }

            size_t partitionSize = count / threadCount;
            size_t partitionExtras = count % threadCount;
            std::vector<boost::function0<void> > tasks;
            tasks.reserve(threadCount);
            void (&encodePtr)(std::vector<uint8_t>*, const T*, const size_t) = encode;

            for (size_t i = 0; i < threadCount; ++i)
            {
                size_t partitionStart = i * partitionSize + std::min(i, partitionExtras);
                size_t partitionEnd = (i + 1) * partitionSize + std::min(i + 1, partitionExtras);
                tasks.push_back(boost::bind(encodePtr, &encodedObjects[partitionStart], &objects[partitionStart], partitionEnd - partitionStart));
            }

            if (executor)
            {
                executor->run(tasks);
            }
            else
            {
                Executor().run(tasks);
            }
        }

        template<typename T> void encode(std::vector<uint8_t>& encodedObject, const T& object)
        {
            encode(&encodedObject, &object, 1);
        }

        template<typename T> void encode(std::vector<std::vector<uint8_t> >& encodedObjects, const std::vector<T>& objects, const size_t threadCount = 1, const ExecutorPtr executor = ExecutorPtr())
        {
            encodedObjects.resize(objects.size(), std::vector<uint8_t>());
            encode(&encodedObjects[0], &objects[0], objects.size(), threadCount, executor);
        }

        class Decoder
        {
            public:
                virtual void decode(boost::any* objects, const std::vector<uint8_t>* encodedObjects, const size_t count, const size_t threadCount = 1, const ExecutorPtr executor = ExecutorPtr()) const = 0;
                virtual void initObject(boost::any& object) const = 0;
                virtual void resizeVector(std::vector<boost::any>& vector, const size_t size) const = 0;
                virtual ~Decoder();
        };

        typedef boost::shared_ptr<Decoder> DecoderPtr;

        template<typename T> class GenericSchemaDecoder : public Decoder
        {
            public:
                GenericSchemaDecoder(const ::avro::ValidSchema& schema) :
                    schema(schema)
                {
                }

                virtual void decode(boost::any* objects, const std::vector<uint8_t>* encodedObjects, const size_t count, const size_t threadCount = 1, const ExecutorPtr executor = ExecutorPtr()) const
                {
                    avro::decodeAny<T>(objects, encodedObjects, count, threadCount, executor);
                }

                virtual void initObject(boost::any& object) const
                {
                    object = T(schema);
                }

                virtual void resizeVector(std::vector<boost::any>& vector, const size_t size) const
                {
                    vector.resize(size, T(schema));
                }

            private:
                ::avro::ValidSchema schema;
        };
/*
        template<typename T> DecoderPtr createDecoder(const std::string& schemaString)
        {
            try
            {
                return boost::make_shared<GenericSchemaDecoder<T> >(::avro::compileJsonSchemaFromString(schemaString));
            }
            catch (const std::exception& ex)
            {
                throw std::invalid_argument("Invalid schema: " + std::string(ex.what()));
            }
        }

        template<typename T> DecoderPtr createDecoder(const ::avro::ValidSchema& schema)
        {
            return boost::make_shared<GenericSchemaDecoder<T> >(schema);
        }
*/
        template<typename T> class GenericTypeDecoder : public Decoder
        {
            public:
                GenericTypeDecoder(const Type& type) :
                    type(type)
                {
                }

                virtual void decode(boost::any* objects, const std::vector<uint8_t>* encodedObjects, const size_t count, const size_t threadCount = 1, const ExecutorPtr executor = ExecutorPtr()) const
                {
                    avro::decodeAny<T>(objects, encodedObjects, count, threadCount, executor);
                }

                virtual void initObject(boost::any& object) const
                {
                    object = T(type);
                }

                virtual void resizeVector(std::vector<boost::any>& vector, const size_t size) const
                {
                    vector.resize(size, T(type));
                }

            private:
                Type type;
        };

        template<typename T> DecoderPtr createDecoder(const Type& type)
        {
            return boost::make_shared<GenericTypeDecoder<T> >(type);
        }

        template<typename T> class SpecificDecoder : public Decoder
        {
            public:
                virtual void decode(boost::any* objects, const std::vector<uint8_t>* encodedObjects, const size_t count, const size_t threadCount = 1, const ExecutorPtr executor = ExecutorPtr()) const
                {
                    avro::decodeAny<T>(objects, encodedObjects, count, threadCount, executor);
                }

                virtual void initObject(boost::any& object) const
                {
                    object = T();
                }

                virtual void resizeVector(std::vector<boost::any>& vector, const size_t size) const
                {
                    vector.resize(size, T());
                }
        };

        template<typename T> DecoderPtr createDecoder()
        {
            return boost::make_shared<SpecificDecoder<T> >();
        }
    }
}

#endif
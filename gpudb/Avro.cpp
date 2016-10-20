#include "gpudb/Avro.hpp"

#include <boost/thread/thread.hpp>

namespace gpudb
{
    namespace avro
    {
        Decoder::~Decoder()
        {
        }

        void runTask(const boost::function0<void>* task, std::string* error)
        {
            try
            {
                (*task)();
            }
            catch (const std::exception& ex)
            {
                *error = ex.what();
            }
        }

        void Executor::run(const std::vector<boost::function0<void> >& tasks) const
        {
            std::vector<std::string> errors;
            errors.resize(tasks.size(), "");
            boost::thread_group threads;

            for (size_t i = 0; i < tasks.size(); ++i)
            {
                threads.create_thread(boost::bind(&runTask, &tasks[i], &errors[i]));
            }

            threads.join_all();

            for (size_t i = 0; i < tasks.size(); ++i)
            {
                if (!errors[i].empty())
                {
                    throw GPUdbException(errors[i]);
                }
            }
        }

        Executor::~Executor()
        {
        }
    }
}
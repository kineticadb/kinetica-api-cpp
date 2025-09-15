#ifndef __FAILBACKPOLLERSERVICE_HPP__
#define __FAILBACKPOLLERSERVICE_HPP__

#include "gpudb/GPUdb.hpp"

#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <memory>
#include <vector>
#include <chrono>


namespace gpudb {
    class FailbackPollerService {
        public:
            static constexpr int DEFAULT_START_DELAY = 0;
            static constexpr int DEFAULT_POLLING_INTERVAL = 5; // seconds
        
        private:
        
            const gpudb::GPUdb& db;
            std::string primaryUrl;
            long pollingInterval;
            std::atomic<bool> is_running;
            std::mutex service_mutex;
            std::unique_ptr<std::thread> scheduler;
        
            void polling_task();
        
            bool poll(); 
        
            void reset_cluster_pointers();

        public:
            FailbackPollerService(const gpudb::GPUdb& db, long pollingInterval) 
            :   db(db), 
                primaryUrl(db.getPrimaryURL()), 
                pollingInterval(pollingInterval), 
                is_running(false) 
            {}

            // Delete copy constructor and assignment operator
            FailbackPollerService(const FailbackPollerService&) = delete;
            FailbackPollerService& operator=(const FailbackPollerService&) = delete;

            virtual ~FailbackPollerService();

            /**
             * Checks whether the poller is running and returns true or false 
             */
            bool isRunning();
        
            /**
             * Starts the poller service
             */
            void start();
        
            /**
             * Stops the poller service
             */
            void stop();
        
            void restart();
        };
        
}

#endif

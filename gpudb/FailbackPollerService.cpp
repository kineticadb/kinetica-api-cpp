#include "gpudb/GPUdb.hpp"
#include "gpudb/FailbackPollerService.hpp"

#include <iostream>
#include <thread>
#include <mutex>
#include <memory>
#include <vector>
#include <chrono>

namespace gpudb {


    /// @brief The main method that is run in a background thread
    void FailbackPollerService::polling_task() {
        std::this_thread::sleep_for(std::chrono::seconds(0)); // DEFAULT_START_DELAY
        while (is_running.load()) {
            try {
                if (poll()) {
                    reset_cluster_pointers();
                    stop(); // Stop if polling succeeds
                    break; // break out of the infinite loop
                }
                std::this_thread::sleep_for(std::chrono::seconds(pollingInterval));
            } catch (const std::exception& e) {
                std::cout << e.what() << std::endl;
            }
        }
    }

    /// @brief The actual `poll` method which is called in
    /// a loop till a successful connection to the primary cluster
    /// can be made
    /// @return true or false
    bool FailbackPollerService::poll() {
        bool kineticaRunning = db.isKineticaRunning(primaryUrl);

        if (kineticaRunning) {
            gpudb::GPUdb::Options options;
            options.setDisableAutoDiscovery(true);
            options.setDisableFailover(true);
            options.setUsername(db.getUsername());
            options.setPassword(db.getPassword());

            try {
                gpudb::GPUdb conn(db.getPrimaryURL(), options);
                std::map<std::string, std::string> empty_params;
                auto statusMap = db.showSystemStatus(empty_params).statusMap;

                auto it = statusMap.find("ha_status");
                if (it != statusMap.end()) {
                    const std::string& haStatus = it->second;
                    if (haStatus == "drained") {
                        std::stringstream ss;
                        ss << "Failback to primary cluster at [" << primaryUrl << "] succeeded";
                        std::cout << ss.str() << std::endl;
                    } else {
                        kineticaRunning = false;
                    }
                }
            } catch (const gpudb::GPUdbException& e) {
                kineticaRunning = false;
            }
        } 

        return kineticaRunning;
    }

    /// @brief Sets the `m_currentUrl` member to 0 once the connection to
    /// the primary cluster is successful
    void FailbackPollerService::reset_cluster_pointers() {
        std::lock_guard<std::mutex> lock(service_mutex);
        this->db.m_currentUrl = 0;
    }


    bool FailbackPollerService::isRunning() {
        return is_running.load();
    }

    void FailbackPollerService::start() {
        std::lock_guard<std::mutex> lock(service_mutex);
        if (is_running.load()) {
            std::cout << "Poller is already running." << std::endl;
            return;
        }

        std::cout << "Starting the poller..." << std::endl;
        is_running.store(true);
        scheduler = std::make_unique<std::thread>(&FailbackPollerService::polling_task, this);
    }

    void FailbackPollerService::stop() {
        std::lock_guard<std::mutex> lock(service_mutex);
        if (!is_running.load()) {
            std::cout << "Poller is already stopped." << std::endl;
            return;
        }

        std::cout << "Stopping the poller..." << std::endl;
        is_running.store(false);
    }

    void FailbackPollerService::restart() {
        std::cout << "Restarting the poller..." << std::endl;
        stop();
        start();
    }

    FailbackPollerService::~FailbackPollerService() {
        stop();
        if (scheduler && scheduler->joinable()) {
            scheduler->join();
        }
    }

}

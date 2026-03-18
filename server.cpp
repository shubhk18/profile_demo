#include "httplib.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <sstream>
#include <queue>
#include <vector>
#include <mutex>
#include <atomic>
#include <cstdlib>

using namespace httplib;

// -------- Thread Pool Simulation --------
struct Metrics {
    double latency_us;
    int throughput;
};

Metrics run_simulation() {
    const int NUM_THREADS = 4;
    const int TOTAL_JOBS = 15000;

    std::queue<int> q;
    std::mutex m;
    std::atomic<int> processed{0};

    for (int i = 0; i < TOTAL_JOBS; i++) {
        q.push(i);
    }

    auto start = std::chrono::high_resolution_clock::now();

    auto worker = [&]() {
        while (true) {
            int job;
            {
                std::lock_guard<std::mutex> lock(m);
                if (q.empty()) break;
                job = q.front();
                q.pop();
            }

            // Simulated work
            for (volatile int i = 0; i < 100; i++);
            processed++;
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_THREADS; i++) {
        threads.emplace_back(worker);
    }

    for (auto &t : threads) t.join();

    auto end = std::chrono::high_resolution_clock::now();

    double total_time =
        std::chrono::duration<double>(end - start).count();

    Metrics mtx;
    mtx.throughput = processed / total_time;
    mtx.latency_us = (total_time * 1e6) / processed;

    return mtx;
}

// -------- MAIN --------
int main() {
    Server svr;

    svr.Get("/", [](const Request&, Response& res) {

        res.set_chunked_content_provider(
            "text/html",
            [](size_t, DataSink &sink) {

                auto send = [&](const std::string& data, int delay = 120) {
                    sink.write(data.c_str(), data.size());
                    std::this_thread::sleep_for(std::chrono::milliseconds(delay));
                };

                send(R"(<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>shubham@system</title>

<style>
body {
    margin: 0;
    background: #0d1117;
    color: #33ff99;
    font-family: "Courier New", monospace;
    padding: 20px;
    font-size: 14px;
}

.terminal {
    max-width: 900px;
    margin: auto;
    white-space: pre-wrap;
}

.prompt {
    color: #58a6ff;
}

.metric {
    color: #f2cc60;
}

.name {
    color: #ffffff;
    font-weight: bold;
}

.cursor {
    display: inline-block;
    width: 8px;
    height: 14px;
    background: #33ff99;
    margin-left: 4px;
    animation: blink 1s infinite;
}

@keyframes blink {
    50% { opacity: 0; }
}
</style>
</head>

<body>
<div class="terminal">
)");

                // -------- Boot --------
                send("<span class='prompt'>shubham@system:~$</span> boot\n", 200);
                send("Initializing system...\n", 120);
                send("Spawning worker threads...\n", 120);
                send("Allocating job queue...\n\n", 150);

                // Run simulation once
                Metrics m = run_simulation();

                send("<span class='prompt'>shubham@system:~$</span> run workload\n\n", 200);

                // -------- Processing animation --------
                send("Processing requests", 200);
                for (int i = 0; i < 3; i++) {
                    send(".", 300);
                }
                send("\n\n", 200);

                // -------- Metrics with realistic latency --------
                for (int i = 0; i < 4; i++) {

                    double jitter = (rand() % 40 - 20) / 100.0;

                    // occasional spike
                    if (rand() % 10 == 0) {
                        jitter += (rand() % 50) / 100.0;
                    }

                    double latency = m.latency_us + jitter;
                    int throughput = m.throughput - (rand() % 70000);

                    std::stringstream ss;
                    ss << "<span class='metric'>Latency:</span> "
                       << latency << " µs   |   "
                       << "<span class='metric'>Throughput:</span> "
                       << throughput << " ops/sec\n";

                    send(ss.str(), 180 + rand() % 120);
                }

                send("\n");

                // -------- Identity --------
                send("<span class='prompt'>shubham@system:~$</span> whoami\n", 150);
                send("<span class='name'>Shubham Kushwaha</span>\n", 150);
                send("C++ Lead Software Engineer\n\n", 150);

                send("<span class='prompt'>shubham@system:~$</span> skills\n", 150);
                send("- High-performance systems\n", 120);
                send("- Concurrency & multithreading\n", 120);
                send("- Low-latency architecture\n", 120);
                send("- Linux internals\n\n", 150);

                send("<span class='prompt'>shubham@system:~$</span> status\n", 150);
                send("System running at peak efficiency.\n", 150);

                send("<span class='cursor'></span>\n");

                send(R"(
</div>

<script>
setTimeout(() => location.reload(), 15000);
</script>

</body>
</html>
)");

                sink.done();
                return true;
            }
        );
    });

    // -------- Railway Port --------
    int port = 8080;
    if (const char* p = std::getenv("PORT")) {
        port = std::stoi(p);
    }

    std::cout << "🚀 Running on port " << port << std::endl;

    svr.listen("0.0.0.0", port);
}

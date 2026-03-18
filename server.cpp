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
    const int TOTAL_JOBS = 50000;

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

                auto send = [&](const std::string& data, int delay = 400) {
                    sink.write(data.c_str(), data.size());
                    std::this_thread::sleep_for(std::chrono::milliseconds(delay));
                };

                send(R"(<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Shubham Kushwaha | C++ Systems Engineer</title>

<style>
html, body {
    margin: 0;
    padding: 0;
    background: #0b0b0c;
    color: #d1d5db;
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", monospace;
    overflow-y: auto;
}

body {
    display: flex;
    justify-content: center;
    padding: 20px;
    font-size: 14px;
}

.container {
    max-width: 800px;
    width: 100%;
}

.terminal {
    background: #111113;
    border-radius: 16px;
    padding: 30px;
    box-shadow: 0 10px 40px rgba(0,0,0,0.6);
}

.line {
    opacity: 0;
    transform: translateY(10px);
    animation: fadeIn 0.8s ease forwards;
}

.highlight {
    color: #22c55e;
    font-weight: 500;
}

.metric {
    color: #60a5fa;
}

.cursor {
    display: inline-block;
    width: 6px;
    height: 14px;
    background: #22c55e;
    margin-left: 4px;
    animation: blink 1s infinite;
}

@keyframes fadeIn {
    to { opacity: 1; transform: translateY(0); }
}

@keyframes blink {
    50% { opacity: 0; }
}

/* Mobile polish */
@media (max-width: 600px) {
    .terminal {
        padding: 20px;
        border-radius: 12px;
    }
}
</style>
</head>
<body>
<div class="container">
<div class="terminal">
)");

                // -------- Boot --------
                send("<div class='line'>$ Initializing system...</div>", 700);
                send("<div class='line'>Spawning worker threads</div>", 500);
                send("<div class='line'>Allocating job queue</div>", 500);

                // -------- Metrics --------
                send("<div class='line'><br>Processing workload...</div>", 600);

                for (int i = 0; i < 3; i++) {
                    Metrics m = run_simulation();

                    std::stringstream ss;
                    ss << "<div class='line metric'>Latency: "
                       << m.latency_us << " µs &nbsp;&nbsp;|&nbsp;&nbsp; Throughput: "
                       << m.throughput << " ops/sec</div>";

                    send(ss.str(), 700);
                }

                // -------- Identity --------
                send("<div class='line'><br><br></div>", 200);
                send("<div class='line highlight'>Shubham Kushwaha</div>", 600);
                send("<div class='line'>C++ Lead Software Engineer</div>", 600);

                send("<div class='line'><br></div>", 200);
                send("<div class='line'>Designing systems handling millions of operations with low latency.</div>", 500);

                send("<div class='line'><br></div>", 200);
                send("<div class='line'>High-performance systems • Concurrency • Linux</div>", 500);

                // -------- Links --------
                send("<div class='line'><br></div>", 200);
                send("<div class='line'>GitHub: github.com/yourname</div>", 400);
                send("<div class='line'>LinkedIn: linkedin.com/in/yourname</div>", 400);

                // -------- Cursor --------
                send("<div class='line'><br>System running<span class='cursor'></span></div>", 500);

                send(R"(</div></div>

<script>
setTimeout(() => location.reload(), 25000);
</script>

</body>
</html>)");

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

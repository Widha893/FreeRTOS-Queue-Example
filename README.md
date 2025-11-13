# 🔬 FreeRTOS Queue Example: Sensor Data Averaging
This project is a practical, hands-on demonstration of FreeRTOS queues used for inter-task communication on an ESP32. It implements the classic Producer-Consumer design pattern to decouple a "sensor" task from a "handler" task.

The goal is to simulate reading a sensor, send that data to another task for processing, and then take action based on the processed results—all in a thread-safe and efficient way.

## ⚙️ How It Works

The project runs two main tasks concurrently:

### 1. dummySensor (The Producer)
- Role: Simulates a sensor generating data.
- Action: Every 500 milliseconds, it creates a random float value.
- It then sends this value into the sensor_data_queue using xQueueSend().
- This task is "dumb"—its only job is to produce data and send it.

### 2. dummySensorHandler (The Consumer)
- Role: Processes the data from the sensor.
- Action:
    - It waits patiently (blocks) until data arrives in the sensor_data_queue, using xQueueReceive().
    - It collects a batch of 10 data samples, summing them up as they arrive.
    - After 10 samples, it calculates the average.
    - It performs logic based on the result: if the average is greater than 6.0, it turns the built-in LED ON; otherwise, it turns it OFF.
    - It then repeats the process, waiting for the next batch of 10 samples.

## 🔑 Key Concepts Demonstrated

### 1. Inter-Task Communication: 
This is the core of the project. The queue (sensor_data_queue) is the only communication channel between the two tasks.

### 2. Task Decoupling: 
The dummySensor task has no idea what a LED is or what "averaging" means. The dummySensorHandler has no idea how the data is generated. This is a powerful design principle that makes code more modular, easier to debug, and reusable.

### 3. Buffering: 
The queue acts as a buffer. If the dummySensor task produces data slightly faster than the dummySensorHandler can process it (or vice-versa), the queue holds the data, preventing it from being lost.

### 4. Efficiency (Blocking): 
The dummySensorHandler task uses zero CPU time while it's waiting for data. Its call to xQueueReceive puts the task into a "sleeping" state until the FreeRTOS scheduler is notified that data has arrived, at which point the task is "woken up."

## 📝 Important notes & tips

- Platform: this example targets ESP32 (Arduino/PlatformIO). High-resolution timing uses `esp_timer_get_time()` (microseconds) on ESP32 — that API is not portable to other platforms. If you run on a different port, you may see different timing/behavior.

- Timing & measurement:
    - The project now logs end-to-end latency in microseconds (producer timestamp -> consumer timestamp). Be aware of what you timestamp:
        - If you timestamp before `xQueueSend()` the measured latency includes any time the producer blocked in `xQueueSend()` waiting for queue space.
        - To measure only post-enqueue → dequeue latency, capture the timestamp after `xQueueSend()` returns and place it into the queued object (use pointer-queue or pool).

- Queue behavior:
    - A FreeRTOS queue is full when `uxQueueMessagesWaiting(queue) == queue length` (or `uxQueueSpacesAvailable(queue) == 0`).
    - `xQueueSend(..., blockTime)` will block the calling task up to `blockTime` ticks if the queue is full; with `blockTime == 0` the call returns immediately with failure when full.

- Scheduling and priorities:
    - Scheduling is controlled by task priorities, blocking calls, and (on ESP32) core affinity. If you want the consumer to process immediately when data arrives, give the consumer a higher priority or use direct task notifications to wake it.
    - Use `vTaskDelayUntil()` for steady periodic sampling, not `vTaskDelay()` if you need a strict period.

- Debugging tips:
    - Print `uxQueueMessagesWaiting()` / `uxQueueSpacesAvailable()` to inspect queue occupancy.
    - Print send-blocking time (time before/after `xQueueSend`) to see if the producer is blocked by a full queue.
    - Minimize `Serial.println()` when measuring latency — serial I/O can affect scheduling and timing.

- Memory and allocation:
    - The example uses value-copy queue items by default. If you need to update queued contents after enqueue (e.g. set a post-send timestamp), either switch to a pointer-queue and manage a small pool, or record both pre/post timestamps locally in the producer and send them together.

- Production note:
    - For production use avoid `malloc()`/`free()` inside tight loops; use a fixed object pool instead.

- Please refer to the FreeRTOS API reference for function uses.
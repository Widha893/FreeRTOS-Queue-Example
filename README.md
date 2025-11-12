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
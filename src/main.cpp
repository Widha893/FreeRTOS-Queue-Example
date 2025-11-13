#include <Arduino.h>
#include <esp_timer.h>


void dummySensor(void *);
void dummySensorHandler(void *);
void dummyLED(void *);
float randomFloat(float min, float max);

QueueHandle_t sensor_data_queue;
constexpr int QUEUE_SIZE = 10;
int queue_size = QUEUE_SIZE;

// Structure to hold sensor value and the microsecond timestamp when it was sent
typedef struct {
  float value;
  int64_t send_time_us; // esp_timer_get_time() value (microseconds)
} SensorData_t;

// End-to-end timing stats (microseconds)
unsigned long long total_e2e_time_us = 0ULL;
uint32_t e2e_op_count = 0;


void setup() {
  Serial.begin(115200);
  Serial.println("==========FREE RTOS QUEUE EXAMPLE==========");
  delay(1000);

  sensor_data_queue = xQueueCreate(queue_size, sizeof(SensorData_t));

  if (sensor_data_queue == NULL) {
    Serial.println("Error creating the queue");

  } else {
    Serial.println("Queue created successfully");
  }

  // Initialize built-in LED pin for on/off control from the sensor handler
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // Create the dummy sensor task
  xTaskCreate(
    dummySensor,
    "DummySensor",
    2048,
    NULL,
    1,
    NULL);

  // Create the dummy sensor handler task
  xTaskCreate(
    dummySensorHandler,
    "DummySensorHandler",
    2048,
    NULL,
    1,
    NULL);
}

void loop() {
}


void dummySensor(void * parameter) {
  Serial.println("Dummy Sensor task started");

  SensorData_t data;
  const TickType_t delay_ticks = pdMS_TO_TICKS(200);
  TickType_t last_wake_time = xTaskGetTickCount();

  for (;;) {
    data.value = randomFloat(0.0, 10.0);

    int64_t t_before_us = esp_timer_get_time();
    data.send_time_us = t_before_us;
    BaseType_t send_res = xQueueSend(sensor_data_queue, &data, pdMS_TO_TICKS(100));
    int64_t t_after_us = esp_timer_get_time();

    if (send_res != pdPASS) {
      Serial.print("Failed to send data to the queue (res="); Serial.print((int)send_res); Serial.println(")");
      
    } else {
      Serial.print("Sensor value sent to queue: ");
      Serial.println(data.value);
      Serial.print("xQueueSend blocking time (us): ");
      Serial.println(t_after_us - t_before_us);
    }

    UBaseType_t waiting = uxQueueMessagesWaiting(sensor_data_queue);
    UBaseType_t spaces = uxQueueSpacesAvailable(sensor_data_queue);
    Serial.print("Queue - waiting: "); Serial.print((unsigned int)waiting);
    Serial.print(", spaces: "); Serial.println((unsigned int)spaces);

    vTaskDelayUntil(&last_wake_time, delay_ticks);
  }
}


void dummySensorHandler(void * parameter) {
  Serial.println("Dummy Sensor Handler task started");

  SensorData_t element;
  float sum = 0.0;
  float avg;
  int count = 0;
  constexpr int SAMPLE_COUNT = 10;
  constexpr float AVG_THRESHOLD = 6.0f;

  for (;;) {
    if (xQueueReceive(sensor_data_queue, &element, pdMS_TO_TICKS(500)) == pdPASS) {
      // Compute end-to-end latency using high-resolution timer (microseconds)
      int64_t recv_time_us = esp_timer_get_time();
      int64_t diff_us = recv_time_us - element.send_time_us;
      unsigned long long diff_us_u = (unsigned long long) diff_us;

      Serial.print("Received sensor value from queue: ");
      Serial.println(element.value);
      Serial.print("End-to-end latency (us): ");
      Serial.println((long long)diff_us);
      Serial.print("End-to-end latency (ms): ");
      Serial.println(((double)diff_us) / 1000.0);

      total_e2e_time_us += diff_us_u;
      e2e_op_count++;

      sum += element.value;
      count++;

      if (count >= SAMPLE_COUNT) {
        avg = sum / count;
        Serial.print("Average of 10 samples: ");
        Serial.println(avg);

        if (avg > AVG_THRESHOLD) {
          digitalWrite(LED_BUILTIN, HIGH);
        } else {
          digitalWrite(LED_BUILTIN, LOW);
        }

        if (e2e_op_count > 0) {
          unsigned long long avg_us = total_e2e_time_us / e2e_op_count;
          Serial.print("Average end-to-end latency for batch (us): ");
          Serial.println((unsigned long long)avg_us);
          Serial.print("Average end-to-end latency for batch (ms): ");
          Serial.println(((double)avg_us) / 1000.0);
        } else {
          Serial.println("No end-to-end samples recorded for this batch");
        }
        Serial.println("=========================================");
        Serial.println();

        sum = 0;
        count = 0;
        total_e2e_time_us = 0ULL;
        e2e_op_count = 0;
      }

    } else {
      Serial.println("No data received from the queue");
    }

  }
}


float randomFloat(float min, float max) {
  float random = ((float) rand()) / (float) RAND_MAX;
  float diff = max - min;
  float r = random * diff;
  return min + r;
}
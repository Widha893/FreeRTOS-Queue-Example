#include <Arduino.h>


void dummySensor(void *);
void dummySensorHandler(void *);
void dummyLED(void *);
float randomFloat(float min, float max);

QueueHandle_t sensor_data_queue;
constexpr int QUEUE_SIZE = 10;
int queue_size = QUEUE_SIZE;


void setup() {
  Serial.begin(115200);
  Serial.println("==========FREE RTOS QUEUE EXAMPLE==========");
  delay(1000);

  sensor_data_queue = xQueueCreate(queue_size, sizeof(float));

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

  float sensor_val;
  const TickType_t delay_ticks = pdMS_TO_TICKS(500);
  
  for (;;) {
    sensor_val = randomFloat(0.0, 10.0);

    if (xQueueSend(sensor_data_queue, &sensor_val, pdMS_TO_TICKS(250)) != pdPASS) {
      Serial.println("Failed to send data to the queue");
    }

    vTaskDelay(delay_ticks);

  }
}


void dummySensorHandler(void * parameter) {
  Serial.println("Dummy Sensor Handler task started");

  float element;
  float sum = 0.0;
  float avg;
  int count = 0;
  constexpr int SAMPLE_COUNT = 10;
  constexpr float AVG_THRESHOLD = 6.0f;

  for (;;) {
    if (xQueueReceive(sensor_data_queue, &element, pdMS_TO_TICKS(1000)) == pdPASS) {
      sum += element;
      count++;

      if (count >= SAMPLE_COUNT) {
        avg = sum / count;
        Serial.print("Average of 10 samples: ");
        Serial.println(avg);

        // Turn LED on if average > 6.0, otherwise turn it off
        if (avg > AVG_THRESHOLD) {
          digitalWrite(LED_BUILTIN, HIGH);
        } else {
          digitalWrite(LED_BUILTIN, LOW);
        }

        sum = 0;
        count = 0;
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
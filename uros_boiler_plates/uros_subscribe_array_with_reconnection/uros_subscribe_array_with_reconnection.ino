#include <micro_ros_arduino.h>

#include <stdio.h>
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <rmw_microros/rmw_microros.h>

#include <std_msgs/msg/int16_multi_array.h>
#include <std_msgs/msg/bool.h>

#define LED_PIN 2
#define RCCHECK(fn) \
  { \
    rcl_ret_t temp_rc = fn; \
    if ((temp_rc != RCL_RET_OK)) { return false; } \
  }
#define EXECUTE_EVERY_N_MS(MS, X) \
  do { \
    static volatile int64_t init = -1; \
    if (init == -1) { init = uxr_millis(); } \
    if (uxr_millis() - init > MS) { \
      X; \
      init = uxr_millis(); \
    } \
  } while (0)

rclc_support_t support;
rcl_node_t node;
rclc_executor_t executor;
rcl_allocator_t allocator;

rcl_subscription_t int16_array_subscriber;
rcl_subscription_t bool_subscriber;

std_msgs__msg__Int16MultiArray int16_array_msg;
std_msgs__msg__Bool bool_msg;

bool micro_ros_init_successful;
enum states {
  WAITING_AGENT,
  AGENT_AVAILABLE,
  AGENT_CONNECTED,
  AGENT_DISCONNECTED
} state;

// Variables to store received Int16 array data
int16_t int16_array[4] = {0, 0, 0, 0};

void int16_array_callback(const void* msgin) {
  const std_msgs__msg__Int16MultiArray* msg = (const std_msgs__msg__Int16MultiArray*)msgin;

  for (size_t i = 0; i < 4 && i < msg->data.size; i++) {
    int16_array[i] = msg->data.data[i];
  }
  if(msg->data.data[0] == 1 && msg->data.data[1] == 2 && msg->data.data[2] == 3 && msg->data.data[3] == 4){
  digitalWrite(LED_PIN, LOW);
  } else if(msg->data.data[0] == 255 && msg->data.data[1] == 255 && msg->data.data[2] == 255 && msg->data.data[3] == 255){
  digitalWrite(LED_PIN, HIGH);
  }
}

void bool_callback(const void* msgin) {
  const std_msgs__msg__Bool* msg = (const std_msgs__msg__Bool*)msgin;

  if (msg->data) {
    digitalWrite(LED_PIN, HIGH); // LED ON
  } else {
    digitalWrite(LED_PIN, LOW); // LED OFF
  }
}

bool create_entities() {
  allocator = rcl_get_default_allocator();

  // create init_options
  RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

  // create node
  RCCHECK(rclc_node_init_default(&node, "multi_subscriber_rclc", "", &support));

  // create subscriptions
  RCCHECK(rclc_subscription_init_default(
    &int16_array_subscriber,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int16MultiArray),
    "int16_array_topic"));

  RCCHECK(rclc_subscription_init_default(
    &bool_subscriber,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Bool),
    "bool_topic"));

  // create executor
  executor = rclc_executor_get_zero_initialized_executor();
  RCCHECK(rclc_executor_init(&executor, &support.context, 2, &allocator));
  RCCHECK(rclc_executor_add_subscription(&executor, &int16_array_subscriber, &int16_array_msg, &int16_array_callback, ON_NEW_DATA));
  RCCHECK(rclc_executor_add_subscription(&executor, &bool_subscriber, &bool_msg, &bool_callback, ON_NEW_DATA));

  return true;
}

void destroy_entities() {
  rmw_context_t* rmw_context = rcl_context_get_rmw_context(&support.context);
  (void)rmw_uros_set_context_entity_destroy_session_timeout(rmw_context, 0);

  rcl_subscription_fini(&int16_array_subscriber, &node);
  rcl_subscription_fini(&bool_subscriber, &node);
  rclc_executor_fini(&executor);
  rcl_node_fini(&node);
  rclc_support_fini(&support);
}

void executor_spin_and_reconnection() {
  switch (state) {
    case WAITING_AGENT:
      EXECUTE_EVERY_N_MS(500, state = (RMW_RET_OK == rmw_uros_ping_agent(100, 1)) ? AGENT_AVAILABLE : WAITING_AGENT;);
      break;
    case AGENT_AVAILABLE:
      state = (true == create_entities()) ? AGENT_CONNECTED : WAITING_AGENT;
      if (state == WAITING_AGENT) {
        destroy_entities();
      };
      break;
    case AGENT_CONNECTED:
      EXECUTE_EVERY_N_MS(200, state = (RMW_RET_OK == rmw_uros_ping_agent(100, 1)) ? AGENT_CONNECTED : AGENT_DISCONNECTED;);
      if (state == AGENT_CONNECTED) {
        rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100));
      }
      break;
    case AGENT_DISCONNECTED:
      destroy_entities();
      state = WAITING_AGENT;
      break;
    default:
      break;
  }
}

void setup() {
  set_microros_transports();
  pinMode(LED_PIN, OUTPUT);

  state = WAITING_AGENT;

  // Initialize Int16MultiArray message
  int16_array_msg.data.data = (int16_t*)malloc(4 * sizeof(int16_t));
  int16_array_msg.data.capacity = 4;
  int16_array_msg.data.size = 4;

  for (size_t i = 0; i < 4; i++) {
    int16_array_msg.data.data[i] = 0;
  }
}

void loop() {
  executor_spin_and_reconnection();
}

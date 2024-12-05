#include <micro_ros_arduino.h>

#include <stdio.h>
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <rmw_microros/rmw_microros.h>

#include <std_msgs/msg/int32_multi_array.h>
#include <std_msgs/msg/float32.h>
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
rcl_timer_t timer;
rclc_executor_t executor;
rcl_allocator_t allocator;

rcl_publisher_t int32_array_publisher;
rcl_publisher_t float32_publisher;
rcl_publisher_t bool_publisher;

std_msgs__msg__Int32MultiArray int32_array_msg;
std_msgs__msg__Float32 float32_msg;
std_msgs__msg__Bool bool_msg;

// Fixed array size for Int32MultiArray
int32_t int32_array_data[4] = {0, 0, 0, 0};

bool micro_ros_init_successful;

// Create timer
const unsigned int publisher_timer_timeout = 100;

enum states {
  WAITING_AGENT,
  AGENT_AVAILABLE,
  AGENT_CONNECTED,
  AGENT_DISCONNECTED
} state;

void timer_callback(rcl_timer_t* timer, int64_t last_call_time) {
  (void)last_call_time;
  if (timer != NULL) {
    // Publish Int32MultiArray message
    for (int i = 0; i < 4; i++) {
      int32_array_data[i] += 1; // Increment each element
    }
    rcl_publish(&int32_array_publisher, &int32_array_msg, NULL);

    // Publish Float32 message
    rcl_publish(&float32_publisher, &float32_msg, NULL);
    float32_msg.data += 0.1f;

    // Publish Bool message
    rcl_publish(&bool_publisher, &bool_msg, NULL);
    bool_msg.data = !bool_msg.data;
  }
}

// Functions create_entities and destroy_entities can take several seconds.
// To reduce this rebuild the library with:
// - RMW_UXRCE_ENTITY_CREATION_DESTROY_TIMEOUT=0
// - UCLIENT_MAX_SESSION_CONNECTION_ATTEMPTS=3

bool create_entities() {
  allocator = rcl_get_default_allocator();

  // Create init_options
  RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

  // Create node
  RCCHECK(rclc_node_init_default(&node, "multi_publisher_rclc", "", &support));

  // Create publishers for different message types
  RCCHECK(rclc_publisher_init_best_effort(
    &int32_array_publisher,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32MultiArray),
    "std_msgs_msg_Int32MultiArray"));

  RCCHECK(rclc_publisher_init_best_effort(
    &float32_publisher,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32),
    "std_msgs_msg_Float32"));

  RCCHECK(rclc_publisher_init_best_effort(
    &bool_publisher,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Bool),
    "std_msgs_msg_Bool"));

  // Initialize timer
  RCCHECK(rclc_timer_init_default(
    &timer,
    &support,
    RCL_MS_TO_NS(publisher_timer_timeout),
    timer_callback));

  // Create executor
  executor = rclc_executor_get_zero_initialized_executor();
  RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
  RCCHECK(rclc_executor_add_timer(&executor, &timer));

  return true;
}

void destroy_entities() {
  rmw_context_t* rmw_context = rcl_context_get_rmw_context(&support.context);
  (void)rmw_uros_set_context_entity_destroy_session_timeout(rmw_context, 0);

  rcl_publisher_fini(&int32_array_publisher, &node);
  rcl_publisher_fini(&float32_publisher, &node);
  rcl_publisher_fini(&bool_publisher, &node);
  rcl_timer_fini(&timer);
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
      }
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

  state = WAITING_AGENT;

  // Initialize Int32MultiArray message
  int32_array_msg.data.data = int32_array_data;
  int32_array_msg.data.size = 4;  // Array size
  int32_array_msg.data.capacity = 4;

  // Initialize Float32 and Bool messages
  float32_msg.data = 0.0f;
  bool_msg.data = false;
}

void loop() {
  executor_spin_and_reconnection();
}

# Micro-ROS Publisher and Subscriber Examples with Integer Arrays and Reconnection

This repository contains examples of Micro-ROS implementations on embedded devices using the Arduino framework. The examples include **publishing and subscribing to integer arrays**, along with other message types, all integrated with reconnection capabilities to ensure robust communication.

## Overview

This repository is divided into three main files, each designed to demonstrate specific functionalities: 

1. **uros_publish_array_with_reconnection.ino**  
   Demonstrates publishing multiple message types, including integer arrays, with reconnection functionality.  
   Topics published:  
   - **`std_msgs/msg/Int32MultiArray` (array of size 4)**  
   - `std_msgs/msg/Float32`  
   - `std_msgs/msg/Bool`

2. **uros_subscribe_array_with_reconnection.ino**  
   Demonstrates subscribing to multiple message types, including integer arrays, with reconnection functionality.  
   Topics subscribed:  
   - **`std_msgs/msg/Int16MultiArray` (array of size 4)**  
   - `std_msgs/msg/Bool`

3. **uros_pub_sub_array_with_reconnection.ino**  
   Combines publishing and subscribing into a single node with reconnection support.  
   - Topics published:  
     - **`std_msgs/msg/Int32MultiArray` (array of size 4)**  
     - `std_msgs/msg/Float32`  
     - `std_msgs/msg/Bool`  
   - Topics subscribed:  
     - **`std_msgs/msg/Int16MultiArray` (array of size 4)**  
     - `std_msgs/msg/Bool`

## Features

- **Integer Array Handling:**  
   - Publish and subscribe to multi-array message types (`Int32MultiArray`, `Int16MultiArray`) for structured data transmission.  
   - Example: Arrays with fixed size 4 are used for showcasing common use cases in embedded systems.

- **Reconnection Support:**  
   - Automatically detects and handles agent disconnections.  
   - Seamlessly reconnects without manual intervention.

- **Micro-ROS Integration:**  
   - Fully compatible with Micro-ROS over serial communication.  
   - Implements best practices for resource-constrained embedded systems.

## Getting Started

1. Clone this repository:
   ```bash
   git clone https://github.com/your-repo/micro-ros-pub-sub-array-reconnection.git
   cd micro-ros-pub-sub-array-reconnection
   ```
2. Open the desired file in your Arduino IDE:
   - `uros_publish_array_with_reconnection`
   - `uros_subscribe_array_with_reconnection`
   - `uros_pub_sub_array_with_reconnection`
3. Flash it to your microcontroller and test the functionality.

## Publishing Data via CLI

You can publish array data directly to the topics for testing:  
1. Publish to the `Int16MultiArray` topic:  
   ```bash
   ros2 topic pub /std_msgs_msg_Int16MultiArray std_msgs/Int16MultiArray "data: [1, 2, 3, 4]"
   ```  
2. Publish to the `Bool` topic:  
   ```bash
   ros2 topic pub /std_msgs_msg_Bool_sub std_msgs/Bool "data: true"
   ```

## Contributing

Feel free to fork this repository and make improvements. Contributions are always welcome!  

---

This README emphasizes the integration of integer array publishing and subscribing, providing clear instructions and highlighting the repository's features.

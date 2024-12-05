# microROS_boilerplate
microROS boilerplate's for publish and subscribe multiple topics at the same time also including multi array and auto re-connection setup

commands to publish data via terminal 
publish int32 data via terminal:
`ros2 topic pub /int32_topic std_msgs/msg/Int32 "data: 1" --once`

publish int16 multiArray data via terminal:
`ros2 topic pub /int16_array_topic std_msgs/msg/Int16MultiArray "{data: [1,2,3,4]}" --once`

rm -f /home/woody/work/compound_eye/mediapipe/bazel-bin/mediapipe/graphs/hand_tracking/multi_hand_tracking_mobile_gpu.binarypb
bazel build -c opt mediapipe/graphs/hand_tracking/subgraphs:multi_hand_renderer_gpu
bazel build -c opt mediapipe/graphs/hand_tracking:multi_hand_tracking_mobile_gpu_binary_graph
rm -f /home/woody/work/compound_eye/handtrackapp/app/src/main/assets/*.binarypb
cp /home/woody/work/compound_eye/mediapipe/bazel-bin/mediapipe/graphs/hand_tracking/multi_hand_tracking_mobile_gpu.binarypb /home/woody/work/compound_eye/handtrackapp/app/src/main/assets/
echo "graph is copied!"

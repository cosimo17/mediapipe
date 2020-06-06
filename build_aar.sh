bazel build -c opt --host_crosstool_top=@bazel_tools//tools/cpp:toolchain --fat_apk_cpu=arm64-v8a,armeabi-v7a \
    //mediapipe/examples/android/src/java/com/google/mediapipe/apps/aisdk_aar:hand_tracking_aar
#bazel build -c opt mediapipe/examples/android/src/java/com/google/mediapipe/apps/multihandtrackinggpu:mediapipe_binary_graph
rm -f /home/woody/work/compound_eye/handtrackapp/app/libs/hand_tracking_aar.aar
cp /home/woody/work/compound_eye/mediapipe/bazel-bin/mediapipe/examples/android/src/java/com/google/mediapipe/apps/aisdk_aar/hand_tracking_aar.aar /home/woody/work/compound_eye/handtrackapp/app/libs/
echo "file copied!"

#bazel build -c opt --config=android_arm64 mediapipe/examples/android/src/java/com/google/mediapipe/apps/multihandtrackinggpu:multihandtrackinggpu
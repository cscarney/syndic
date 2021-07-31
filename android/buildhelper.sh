APK_DIR=$(pwd)/feedkeeper-apk
mkdir $APK_DIR
/opt/helpers/build-cmake feedkeeper $HOME/src/feedkeeper -DQTANDROID_EXPORTED_TARGET=feedkeeper -DANDROID_APK_DIR=$HOME/src/feedkeeper/android -DANDROID_APK_OUTPUT_DIR=$APK_DIR
/opt/helpers/create-apk feedkeeper

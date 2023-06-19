# Prepare for Android

- download/compile the cppsdk for android
- install the artifact to local maven
  - if you only have the aar file, put it in some location

**prepare the environment from the installed library**
```
bash ./scripts/android/unzip.sh
```

**prepare the environment from the aar**

```
bash ./scripts/android/unzip.sh "path/to/the/file.aar"
```

# Build for Android

```
bash ./scripts/android/build.sh
```

# Install

- Add the Output content to the Plugins/DolbyIO
- It's recommended to upgrade the gradle/wrapper.properties bundled by UE:
  - open the file `[UE_5.2]/Engine/Build/Android/Java/gradle/gradle/wrapper/gradle-wrapper.properties`
  - for instance under */Users/Shared/Epic\ Games/UE_5.2/...*
  - change *gradle-x.y.z-all.zip* to `gradle-7.4.1-all.zip`

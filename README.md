# MicAudioRack

**MicAudioRack** is an open-source audio routing application written in C++ using the [JUCE](https://juce.com/) framework. It allows users to route microphone input through a customizable chain of audio effects, including both VST3 plugins and built-in processors, and output the processed audio to any available device.

## Getting Started

### Prerequisites

- JUCE 7.x (you can clone it from [https://github.com/juce-framework/JUCE](https://github.com/juce-framework/JUCE))
- CMake >= 3.15
- C++17-compatible compiler (MSVC, Clang, or GCC)

> Note: If you want to use this program to apply effects to your voice in real-time communication apps (such as Discord, Teams, Zoom, etc.), you will need to install a [virtual audio cable](https://vb-audio.com/Cable). 
> It allows you to route the processed audio from this program into other applications as if it were a microphone input.

### Build Instructions

```bash
# Clone project and JUCE (adjust paths if needed)
git clone https://github.com/usein-abilev/mic-audio-rack.git
cd ./mic-audio-rack

# Set JUCE_DIR in CMakeLists.txt if not already pointing to JUCE
cmake -Bbuild -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
```

The resulting executable will be located in `build/MicAudioRack_artefacts/Debug` or similar depending on your platform.

---

## Contributing

Contributions are welcome! 🎉 If you want to improve this project:

- Fork the repository
- Create a new branch for your feature or fix
- Submit a pull request with a clear description

💡 Note: This project is not written by a professional C++ developer. I’m happy to receive any suggestions, improvements, or code cleanups from experienced contributors. Your help is greatly appreciated!

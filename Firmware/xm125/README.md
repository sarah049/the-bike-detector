# Acconeer XM125 Firmware

## Quick How-To Compile (using make):

All example applications can be built from the command line using “make”. Make sure you're getting correct compiler versions!
1. Download the STM32Cube MCU Package for STM32L4 series (version 1.17.0) from [www.st.com](https://www.st.com/en/embedded-software/stm32cubel4.html).
2. Extract the archive into a folder, e.g. “/team5-thebikedetector/compilers/” (make sure this huge folder is in .gitignore so that you don't push it to git!!)
3. Download “GCC ARM Embedded 9-2020-q2-update” (.tar.bz2 on Mac) from [developer.arm.com](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads).
4. Extract the archive into a folder, e.g. “/team5-thebikedetector/compilers/”
5. Change directory into the folder where you saved the compilers (from team5-thebikedetector/) e.g.:
```
cd /compilers/
```

6. Set the environment variables:
```
export STM32CUBE_FW_L4_ROOT=$(pwd)/STM32Cube_FW_L4_V1.17.0/
export GNU_INSTALL_ROOT=$(pwd)/gcc-arm-none-eabi-9-2020-q2-update/bin/
```

7. (For Mac) Allow all scripts in gcc compiler to run:
```
sudo xattr -r -d com.apple.quarantine ./gcc-arm-none-eabi-9-2020-q2-update
```

7. Go into the firmware folder and make:
```
cd ../Firmware/xm125/
make -j10
```

More notes are in [xm125 firmware sdk]/doc/XM125 Software User Guide.pdf


## Quick How-To Setup Exploration Tool:

1. Install & open [Acconeer Exploration Tool](https://github.com/acconeer/acconeer-python-exploration)
```
python -m pip install --upgrade "acconeer-exptool[app]"
python -m acconeer.exptool.app
```

2. Install CP210x USB-Serial Drivers from [silabs.com](https://www.silabs.com/developer-tools/usb-to-uart-bridge-vcp-drivers?tab=downloads)

3. Install libusb if not yet installed:
```
brew install libusb
```

4. Flash Exploration Server firmware onto xm125 module from Exploration Tool. The binary is [xm125 firmware sdk]/out/acc_exploration_server_a121.bin
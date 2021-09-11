##Why switch to PlatformIO?
- You won't need to install the 20+ Arduino dependencies manually!
- Specify build flags like `ICM_20948_USE_DMP` in the platformio.ini instead of modifying library headers
- Use fixed version numbers for dependencies so your project won't break when a library updates
- It's optional to use; you can switch back to the Arduino IDE with no additional configuration
- Compiles the firmware much faster than the Arduino IDE


## Setup for Development with PlatformIO

- Install PlatformIO Core: [https://platformio.org/install](https://platformio.org/install)
- Follow these directions to install the apollo3blue platform: [https://github.com/nigelb/platform-apollo3blue#install](https://github.com/nigelb/platform-apollo3blue#install)
  - You might need to create the packages and platforms folders if they don't exist:
    - `mkdir ~/.platformio/packages`
    - `mkdir ~/.platformio/platforms`

## Build and Upload

Open the project in any [PlatformIO compatible IDE](https://platformio.org/install/integration) and upload the firmware to your OLA.

Or, you could upload to the OLA using the command line: (make sure to [install the pio shell commands first](https://docs.platformio.org/en/latest//core/installation.html#install-shell-commands))
- `cd <repo location on your computer>/OpenLog_Artemis`
- `pio run`
- Then, use the Arduino IDE serial monitor like usual, or run `pio device monitor -p 115200` to test the firmware!

See [https://docs.platformio.org/en/latest/what-is-platformio.html](https://docs.platformio.org/en/latest/what-is-platformio.html) for more information.